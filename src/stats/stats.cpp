#include "stats.hpp"

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <chrono>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>

#include "opentelemetry/exporters/ostream/metric_exporter_factory.h"
#include "opentelemetry/exporters/otlp/otlp_http_metric_exporter_factory.h"
#include "opentelemetry/metrics/provider.h"
#include "opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_factory.h"
#include "opentelemetry/sdk/metrics/meter_provider.h"
#include "opentelemetry/sdk/metrics/meter_provider_factory.h"
#include "opentelemetry/sdk/metrics/push_metric_exporter.h"
#include "opentelemetry/context/context.h"

using namespace std::chrono;

namespace stats
{

void InitMetricsOStream()
{
    auto exporter = opentelemetry::exporter::metrics::OStreamMetricExporterFactory::Create();

    // Initialize and set the global MeterProvider
    opentelemetry::sdk::metrics::PeriodicExportingMetricReaderOptions options;
    options.export_interval_millis = std::chrono::milliseconds(1000);
    options.export_timeout_millis = std::chrono::milliseconds(500);

    auto reader = opentelemetry::sdk::metrics::PeriodicExportingMetricReaderFactory::Create(
        std::move(exporter), options);

    auto u_provider = opentelemetry::sdk::metrics::MeterProviderFactory::Create();
    auto* p = static_cast<opentelemetry::sdk::metrics::MeterProvider*>(u_provider.get());
    p->AddMetricReader(std::move(reader));

    std::shared_ptr<opentelemetry::metrics::MeterProvider> provider(std::move(u_provider));
    opentelemetry::metrics::Provider::SetMeterProvider(provider);
}

void InitMetricsOtlpHttp()
{
    opentelemetry::exporter::otlp::OtlpHttpMetricExporterOptions otlpOptions;
    otlpOptions.url =
        "http://victoria-svc:8428/opentelemetry/api/v1/push";
    otlpOptions.content_type = opentelemetry::exporter::otlp::HttpRequestContentType::kBinary;
    otlpOptions.console_debug = true;
    auto exporter =
        opentelemetry::exporter::otlp::OtlpHttpMetricExporterFactory::Create(otlpOptions);

    // Initialize and set the periodic metrics reader
    opentelemetry::sdk::metrics::PeriodicExportingMetricReaderOptions options;
    options.export_interval_millis = std::chrono::milliseconds(1000);
    options.export_timeout_millis = std::chrono::milliseconds(500);

    auto reader = opentelemetry::sdk::metrics::PeriodicExportingMetricReaderFactory::Create(
        std::move(exporter), options);

    auto u_provider = opentelemetry::sdk::metrics::MeterProviderFactory::Create();
    auto* p = static_cast<opentelemetry::sdk::metrics::MeterProvider*>(u_provider.get());
    p->AddMetricReader(std::move(reader));

    std::shared_ptr<opentelemetry::metrics::MeterProvider> provider(std::move(u_provider));
    opentelemetry::metrics::Provider::SetMeterProvider(provider);
}

void CleanupMetrics()
{
    std::shared_ptr<opentelemetry::metrics::MeterProvider> none;
    opentelemetry::metrics::Provider::SetMeterProvider(none);
}

std::string stats::create_headers_str()
{
    std::stringstream h;
    h << std::left << std::setw(10) << "Time (s)" << std::right << std::setw(10) << "Sent/s"
      << std::right << std::setw(10) << "Recv/s" << std::right << std::setw(15) << "RT (ms)"
      << std::right << std::setw(15) << "minRT (ms)" << std::right << std::setw(15) << "maxRT (ms)"
      << std::right << std::setw(15) << "Sent" << std::right << std::setw(15) << "Success"
      << std::right << std::setw(15) << "Errors" << std::right << std::setw(15) << "Timeouts"
      << std::endl;

    return h.str();
}

stats::stats(boost::asio::io_context& io_ctx, const int p, const std::string& output_file_name,
             const std::vector<std::string>& msg_names)
    : timer(io_ctx),
      print_period(p * 1000),
      cancel(false),
      counter(0),
      file_prefix(output_file_name),
      accum_filename(output_file_name + ".accum"),
      partial_filename(output_file_name + ".partial"),
      err_filename(output_file_name + ".err"),
      total_snap(),
      partial_snap(),
      stats_headers(create_headers_str())
{
    for (const auto& name : msg_names)
    {
        snapshot msg_snap;
        msg_snaps.emplace(name, msg_snap);
        std::fstream msg_file;
        msg_file.open(output_file_name + "." + name, std::fstream::out);
        write_headers(msg_file);
        msg_file.close();
    }

    std::fstream accum_file;
    accum_file.open(accum_filename, std::fstream::out);
    write_headers(accum_file);
    accum_file.close();

    std::fstream partials_file;
    partials_file.open(partial_filename, std::fstream::out);
    write_headers(partials_file);
    partials_file.close();

    std::fstream errors_file;
    errors_file.open(err_filename, std::fstream::out);
    auto print_time = system_clock::to_time_t(system_clock::now());
    errors_file << "Traffic started at:  " << std::ctime(&print_time) << std::endl
                << std::left << std::setw(10) << "Time (s)" << std::right << std::setw(10) << "Code"
                << std::right << std::setw(10) << "Count" << std::endl;
    errors_file.close();

    print_headers();

    ++counter;
    timer.expires_after(milliseconds(print_period));
    timer.async_wait(boost::bind(&stats::print, this));

    InitMetricsOStream();
    InitMetricsOtlpHttp();

    auto provider = opentelemetry::metrics::Provider::GetMeterProvider();
    auto meter = provider->GetMeter("this_will_fail");

    auto sent = meter->CreateUInt64Counter("hermes_requests_sent", "Requests sent by hermes");
    requests_sent = std::move(sent);
    auto resp_ok = meter->CreateUInt64Counter("hermes_responses_rcv_ok", "Expected responses received by hermes");
    responses_ok = std::move(resp_ok);
    auto resp_nok = meter->CreateUInt64Counter("hermes_responses_rcv_err", "Unsuccessful responses received by hermes");
    responses_err = std::move(resp_nok);
    auto to =
        meter->CreateUInt64Counter("hermes_timeouts", "Timeouts in requests sent by hermes");
    timeouts = std::move(to);

    auto rtok = meter->CreateDoubleHistogram(
        "hermes_response_time_ok_ms",
        "Response Time of requests with response codes expected by hermes", "ms");
    histo_rtok_ms = std::move(rtok);
    /*auto rtnok = meter->CreateDoubleHistogram(
        "hermes_response_time_nok_ms",
        "Response Time of requests with response codes not expected by hermes", "ms");
    histo_rtnok_ms = std::move(rtnok);*/
}

stats::~stats()
{
    CleanupMetrics();
}

void stats::write_headers(std::fstream& fs)
{
    auto print_time = system_clock::to_time_t(system_clock::now());
    fs << "Traffic started at:  " << std::ctime(&print_time) << std::endl << stats_headers;
}

void stats::print_headers() const
{
    std::cout << stats_headers;
}

void stats::update_rcs(snapshot& snap, const int code, const bool is_error)
{
    auto& rcs = is_error ? snap.response_codes_nok : snap.response_codes_ok;
    auto previous_rc = rcs.find(code);
    if (previous_rc != rcs.end())
    {
        previous_rc->second += 1;
    }
    else
    {
        rcs.emplace(code, 1);
    }
}

void stats::update_rts(snapshot& snap, const int64_t elapsed_time)
{
    if (snap.responded_ok > 1)
    {
        // TODO: change avg_rt to double?
        snap.avg_rt *= std::pow((double(elapsed_time) / double(snap.avg_rt)),
                                (1.0 / double(snap.responded_ok)));
    }
    else
    {
        snap.avg_rt = elapsed_time;
    }

    if (snap.min_rt > elapsed_time || snap.min_rt == 0)
    {
        snap.min_rt = elapsed_time;
    }

    if (snap.max_rt < elapsed_time)
    {
        snap.max_rt = elapsed_time;
    }
}
void stats::add_measurement(snapshot& snap, const int64_t elapsed_time, const int code)
{
    ++snap.responded_ok;
    update_rts(snap, elapsed_time);
    update_rcs(snap, code, false);
}

void stats::add_measurement(const std::string& id, const int64_t elapsed_time, const int code)
{
    write_lock wr_lock(rw_mutex);
    add_measurement(total_snap, elapsed_time, code);
    add_measurement(partial_snap, elapsed_time, code);
    add_measurement(msg_snaps.at(id), elapsed_time, code);

    std::map<std::string, std::string> labels1{{"id", id}, {"response_code", std::to_string(code)}};
    auto labelkv1 = opentelemetry::common::KeyValueIterableView<decltype(labels1)>{labels1};

    auto context = opentelemetry::context::Context{};
    responses_ok->Add(1, labelkv1);
    histo_rtok_ms->Record(double(elapsed_time)/1000.0, labelkv1, context);
}

void stats::increase_sent(const std::string& id)
{
    write_lock wr_lock(rw_mutex);
    ++total_snap.sent;
    ++partial_snap.sent;
    ++msg_snaps.at(id).sent;

    // Create a label set which annotates metric values
    std::map<std::string, std::string> labels = {{"id", id}};
    auto labelkv = opentelemetry::common::KeyValueIterableView<decltype(labels)>{labels};
    requests_sent->Add(1, labelkv);
}

void stats::add_timeout(const std::string& id)
{
    write_lock wr_lock(rw_mutex);
    ++total_snap.timed_out;
    ++partial_snap.timed_out;
    ++msg_snaps.at(id).timed_out;

    std::map<std::string, std::string> labels = {{"id", id}};
    auto labelkv = opentelemetry::common::KeyValueIterableView<decltype(labels)>{labels};
    timeouts->Add(1, labelkv);
}

void stats::add_error(const std::string& id, const int e)
{
    write_lock wr_lock(rw_mutex);
    update_rcs(total_snap, e, true);
    update_rcs(partial_snap, e, true);
    update_rcs(msg_snaps.at(id), e, true);

    std::map<std::string, std::string> labels{{"id", id}, {"response_code", std::to_string(e)}};
    auto labelkv = opentelemetry::common::KeyValueIterableView<decltype(labels)>{labels};
    responses_err->Add(1, labelkv);
}

void stats::add_client_error(const std::string& id, const int e)
{
    write_lock wr_lock(rw_mutex);
    ++total_snap.sent;
    ++partial_snap.sent;
    ++msg_snaps.at(id).sent;
    update_rcs(total_snap, e, true);
    update_rcs(partial_snap, e, true);
    update_rcs(msg_snaps.at(id), e, true);
}

void stats::print_snapshot(const snapshot& snap, const time_point<steady_clock>& init_time,
                           std::ostream& out) const
{
    const auto now = steady_clock::now();
    float partial_time = duration_cast<milliseconds>(now - snap.init_time).count();
    if (partial_time == 0)
    {
        return;
    }

    float total_time = duration_cast<milliseconds>(now - init_time).count();

    int64_t counter_ok{0};
    for (const auto& code : snap.response_codes_ok)
    {
        counter_ok += code.second;
    }

    int64_t counter_nok{0};
    for (const auto& code : snap.response_codes_nok)
    {
        counter_nok += code.second;
    }

    out << std::fixed << std::left << std::setw(10) << std::setprecision(1) << total_time * 0.001
        << std::right << std::setw(10) << float(snap.sent) / partial_time * 1000. << std::right
        << std::setw(10) << float(snap.responded_ok) / partial_time * 1000. << std::right
        << std::setw(15) << std::setprecision(3) << snap.avg_rt / 1000. << std::right
        << std::setw(15) << snap.min_rt / 1000. << std::right << std::setw(15)
        << snap.max_rt / 1000. << std::right << std::setw(15) << snap.sent << std::right
        << std::setw(15) << counter_ok << std::right << std::setw(15) << counter_nok << std::right
        << std::setw(15) << snap.timed_out << std::endl;
}

void stats::write_errors() const
{
    std::fstream err_file;
    err_file.open(err_filename, std::fstream::app);

    float time = duration_cast<milliseconds>(steady_clock::now() - total_snap.init_time).count();

    for (const auto& code : total_snap.response_codes_nok)
    {
        err_file << std::left << std::setw(10) << time * 0.001 << std::right << std::setw(10)
                 << code.first << std::right << std::setw(10) << code.second << std::endl;
    }
    err_file.close();
}

void stats::do_print()
{
    write_lock wr_lck(rw_mutex);
    std::fstream partials_file;
    partials_file.open(partial_filename, std::fstream::app);
    print_snapshot(partial_snap, total_snap.init_time, partials_file);
    partials_file.close();

    std::fstream accum_file;
    accum_file.open(accum_filename, std::fstream::app);
    print_snapshot(total_snap, total_snap.init_time, accum_file);
    accum_file.close();

    for (const auto& msg_snap : msg_snaps)
    {
        std::fstream msg_file;
        msg_file.open(file_prefix + "." + msg_snap.first, std::fstream::app);
        print_snapshot(msg_snap.second, total_snap.init_time, msg_file);
        msg_file.close();
    }

    write_errors();

    print_snapshot(total_snap, total_snap.init_time);

    partial_snap = snapshot();
}

void stats::print()
{
    if (!cancel)
    {
        ++counter;
        steady_clock::time_point future_time =
            total_snap.init_time + std::chrono::milliseconds(print_period * counter.load());
        auto elapsed = duration_cast<milliseconds>(future_time - steady_clock::now()).count();

        timer.expires_after(milliseconds(elapsed));
        timer.async_wait(boost::bind(&stats::print, this));
    }
    else
    {
        print_headers();
        for (const auto& msg_snap : msg_snaps)
        {
            std::cout << ">>>" + msg_snap.first + "<<<" << std::endl;
            print_snapshot(msg_snap.second, total_snap.init_time);
        }
        std::cout << ">>>Total<<<" << std::endl;
    }

    do_print();
}

void stats::end()
{
    std::cerr << "Execution finished. Printing stats..." << std::endl;
    cancel = true;
    timer.cancel();
}
}  // namespace stats
