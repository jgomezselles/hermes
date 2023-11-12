#include <libgen.h>
#include <nghttp2/asio_http2_client.h>
#include <syslog.h>

#include <boost/asio/steady_timer.hpp>
#include <boost/bind/bind.hpp>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <thread>

#include "client_impl.hpp"
#include "connection.hpp"
#include "metrics.hpp"
#include "params.hpp"
#include "script.hpp"
#include "script_queue.hpp"
#include "script_schema.hpp"
#include "sender.hpp"
#include "stats.hpp"
#include "timer_impl.hpp"
#include "tracer.hpp"

using nghttp2::asio_http2::header_map;
using nghttp2::asio_http2::header_value;
using namespace std::chrono;

using boost::asio::ip::tcp;
namespace ba = boost::asio;

using namespace nghttp2::asio_http2;
using namespace nghttp2::asio_http2::client;

const char* progname;
const unsigned int default_rate{10};
const int default_duration{60};
const int default_stats_print_period{10};

const std::string default_traffic_path{"/etc/scripts/traffic.json"};
const std::string default_output_file{"hermes.out"};

[[noreturn]] static void usage(int rc)
{
    syslog(LOG_INFO,
           "C++ Traffic Generator. Usage:  %s [options] \n"
           "options:\n\n"
           " \t-r <rate>\tRequests/second ( Default: %d )\n"
           " \t-t <time>\tTime to run traffic (s) ( Default: %d )\n"
           " \t-p <period>\tPrint and save statistics every <period> (s) ( Default: %d )\n"
           " \t-f <path>\tPath with the traffic json definition ( Default: %s )\n"
           " \t-s \t\tShow schema for json traffic definition.\n"
           " \t-o <file>\tOutput file for statistics( Default: %s )\n"
           " \t-h \t\tThis help.",
           progname, default_rate, default_duration, default_stats_print_period,
           default_traffic_path.c_str(), default_output_file.c_str());
    exit(rc);
}

int main(int argc, char* argv[])
{
    progname = basename(argv[0]);
    openlog(progname, LOG_CONS | LOG_PERROR, LOG_LOCAL1);

    unsigned int rate{default_rate};
    int duration{default_duration};
    int print_period{default_stats_print_period};
    std::string traffic_json_path{default_traffic_path};
    std::string output_file{default_output_file};

    int option{};
    while ((option = getopt(argc, argv, "hr:t:f:sp:o:")) != EOF)
    {
        switch (option)
        {
            case 'h':
                usage(0);
            case 'r':
                rate = atoi(optarg);
                break;
            case 't':
                duration = atoi(optarg);
                break;
            case 'p':
                print_period = atoi(optarg);
                break;
            case 'f':
                traffic_json_path = optarg;
                break;
            case 's':
                syslog(LOG_INFO, "Schema for traffic definition: \n%s", script::schema.c_str());
                exit(0);
            case 'o':
                output_file = optarg;
                break;
            default:
                std::cerr << "Invalid option" << std::endl;
                exit(1);
        }
    }

    std::optional<traffic::script> the_script;
    try
    {
        the_script = traffic::script(traffic_json_path);
    }
    catch (const std::logic_error& e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << "Error in script. Please, check your traffic script located in '"
                  << traffic_json_path << "' against the following schema:" << std::endl
                  << script::schema << std::endl;
        exit(1);
    }

    /******************************************************************
     * OBSERVABILITY
     ******************************************************************/
    if (const char* otlp_metrics_endpoint = std::getenv("OTLP_METRICS_ENDPOINT");
        otlp_metrics_endpoint && !std::string(otlp_metrics_endpoint).empty())
    {
        std::cerr << "Starting OTLP metrics exporter towards: " << otlp_metrics_endpoint
                  << std::endl;
        o11y::init_metrics_otlp_http(otlp_metrics_endpoint);
    }
    else
    {
        std::cerr << "OTLP_METRICS_ENDPOINT not found. Metrics won't be pushed." << std::endl;
    }

    if (const char* otlp_traces_endpoint = std::getenv("OTLP_TRACES_ENDPOINT");
        otlp_traces_endpoint && !std::string(otlp_traces_endpoint).empty())
    {
        std::cerr << "Starting OTLP tracing exporter towards: " << otlp_traces_endpoint
                  << std::endl;
        o11y::init_tracer(otlp_traces_endpoint);
    }
    else
    {
        std::cerr << "OTLP_TRACES_ENDPOINT not found. Traces won't be pushed." << std::endl;
    }

    /******************************************************************
     * IO_CTX
     ******************************************************************/
    ba::io_context io_ctx;
    ba::executor_work_guard<ba::io_context::executor_type> io_ctx_grd = ba::make_work_guard(io_ctx);
    std::vector<std::thread> workers;
    for (auto i = 0; i < 2; ++i)
    {
        workers.emplace_back([&io_ctx]() { io_ctx.run(); });
    }

    ba::io_context stats_io_ctx;
    ba::executor_work_guard<ba::io_context::executor_type> stats_io_ctx_guard =
        ba::make_work_guard(stats_io_ctx);

    std::vector<std::thread> stats_workers;
    for (auto i = 0; i < 2; ++i)
    {
        stats_workers.emplace_back([&stats_io_ctx]() { stats_io_ctx.run(); });
    }

    ba::io_context client_io_ctx;
    ba::executor_work_guard<ba::io_context::executor_type> client_io_ctx_guard =
        ba::make_work_guard(client_io_ctx);
    std::thread client_worker([&client_io_ctx]() { client_io_ctx.run(); });

    /******************************************************************
     * PARAMS
     ******************************************************************/
    std::cerr << "Rate is " << rate << "req/s" << std::endl;
    double wait_time = std::pow(10.0, 6) / double(rate);
    std::cerr << "Sending a request every " << wait_time << "us" << std::endl;
    auto params = std::make_shared<config::params>(int(wait_time), duration);

    auto stats = std::make_shared<stats::stats>(stats_io_ctx, print_period, output_file,
                                                the_script->get_message_names());

    /******************************************************************
     * CLIENT
     ******************************************************************/
    auto q = std::make_unique<traffic::script_queue>(*the_script);
    auto client = std::make_unique<http2_client::client_impl>(
        stats, client_io_ctx, std::move(q), the_script->get_server_dns(),
        the_script->get_server_port(), the_script->is_server_secure());
    if (!client->is_connected())
    {
        std::cerr << "Terminating application. Error connecting server." << std::endl;
        exit(1);
    }

    /******************************************************************
     * EXECUTION
     ******************************************************************/

    std::promise<void> prom;
    std::future<void> fut = prom.get_future();

    engine::sender sender(std::make_unique<engine::timer_impl>(io_ctx), std::move(client), params,
                          std::move(prom));

    fut.wait();

    stats->end();

    o11y::cleanup_metrics();
    o11y::cleanup_tracer();

    io_ctx_grd.reset();
    for (auto& thread : workers)
    {
        thread.join();
    }

    stats_io_ctx_guard.reset();
    for (auto& thread : stats_workers)
    {
        thread.join();
    }

    client_io_ctx_guard.reset();
    client_worker.join();
}
