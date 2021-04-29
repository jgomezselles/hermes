#include "stats.hpp"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <chrono>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>

using namespace std::chrono;

namespace stats
{
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
                << std::left << std::setw(10) << "Time (ms)" << std::right << std::setw(10)
                << "Code" << std::right << std::setw(10) << "Count" << std::endl;
    errors_file.close();

    print_headers();

    ++counter;
    timer.expires_after(milliseconds(print_period));
    timer.async_wait(boost::bind(&stats::print, this));
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
}

void stats::increase_sent(const std::string& id)
{
    write_lock wr_lock(rw_mutex);
    ++total_snap.sent;
    ++partial_snap.sent;
    ++msg_snaps.at(id).sent;
}

void stats::add_timeout(const std::string& id)
{
    write_lock wr_lock(rw_mutex);
    ++total_snap.timed_out;
    ++partial_snap.timed_out;
    ++msg_snaps.at(id).timed_out;
}

void stats::add_error(const std::string& id, const int e)
{
    write_lock wr_lock(rw_mutex);
    update_rcs(total_snap, e, true);
    update_rcs(partial_snap, e, true);
    update_rcs(msg_snaps.at(id), e, true);
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

void stats::print_snapshot(const snapshot& snap) const
{
    float partial_time = duration_cast<milliseconds>(steady_clock::now() - snap.init_time).count();
    if (partial_time == 0)
    {
        return;
    }

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

    std::cout << std::fixed << std::left << std::setw(10) << std::setprecision(1)
              << partial_time * 0.001 << std::right << std::setw(10)
              << float(snap.sent) / partial_time * 1000. << std::right << std::setw(10)
              << float(snap.responded_ok) / partial_time * 1000. <<

        std::right << std::setw(15) << std::setprecision(3) << snap.avg_rt / 1000. << std::right
              << std::setw(15) << snap.min_rt / 1000. << std::right << std::setw(15)
              << snap.max_rt / 1000. <<

        std::right << std::setw(15) << snap.sent << std::right << std::setw(15) << counter_ok
              << std::right << std::setw(15) << counter_nok << std::right << std::setw(15)
              << snap.timed_out << std::endl;
}

void stats::write_snapshot(const snapshot& snap, std::fstream& fs,
                           const time_point<steady_clock>& init_time) const
{
    float partial_time = duration_cast<milliseconds>(steady_clock::now() - snap.init_time).count();
    if (partial_time == 0)
    {
        return;
    }

    float total_time = duration_cast<milliseconds>(steady_clock::now() - init_time).count();

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

    fs << std::left << std::setw(10) << total_time * 0.001 << std::right << std::setw(10)
       << float(snap.sent) / partial_time * 1000. << std::right << std::setw(10)
       << float(snap.responded_ok) / partial_time * 1000. << std::right << std::setw(15)
       << snap.avg_rt / 1000. << std::right << std::setw(15) << snap.min_rt / 1000. << std::right
       << std::setw(15) << snap.max_rt / 1000. <<

        std::right << std::setw(15) << snap.sent << std::right << std::setw(15) << counter_ok
       << std::right << std::setw(15) << counter_nok << std::right << std::setw(15)
       << snap.timed_out << std::endl;
}

void stats::write_errors() const
{
    std::fstream err_file;
    err_file.open(err_filename, std::fstream::app);

    float time = duration_cast<milliseconds>(steady_clock::now() - total_snap.init_time).count();

    for (const auto& code : total_snap.response_codes_nok)
    {
        err_file << std::left << std::setw(10) << time << std::right << std::setw(10) << code.first
                 << std::right << std::setw(10) << code.second << std::endl;
    }
    err_file.close();
}

void stats::do_print()
{
    write_lock wr_lck(rw_mutex);
    std::fstream partials_file;
    partials_file.open(partial_filename, std::fstream::app);
    write_snapshot(partial_snap, partials_file, total_snap.init_time);
    partials_file.close();

    std::fstream accum_file;
    accum_file.open(accum_filename, std::fstream::app);
    write_snapshot(total_snap, accum_file, total_snap.init_time);
    accum_file.close();

    for (const auto& msg_snap : msg_snaps)
    {
        std::fstream msg_file;
        msg_file.open(file_prefix + "." + msg_snap.first, std::fstream::app);
        write_snapshot(msg_snap.second, msg_file, total_snap.init_time);
        msg_file.close();
    }

    write_errors();

    print_snapshot(total_snap);

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
            print_snapshot(msg_snap.second);
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
