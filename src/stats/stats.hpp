#include "opentelemetry/sdk/metrics/sync_instruments.h"

#include <atomic>
#include <boost/asio.hpp>
#include <chrono>
#include <iostream>
#include <map>
#include <mutex>
#include <shared_mutex>

#include "stats_if.hpp"

#pragma once

namespace config
{
class params;
}

using namespace std::chrono;

namespace stats
{
using mutex_type = std::shared_timed_mutex;
using read_lock = std::shared_lock<mutex_type>;
using write_lock = std::unique_lock<mutex_type>;

struct snapshot
{
    friend inline bool operator==(const snapshot& lhs, const snapshot& rhs)
    {
        return lhs.sent == rhs.sent && lhs.responded_ok == rhs.responded_ok &&
               lhs.timed_out == rhs.timed_out && lhs.rate == rhs.rate && lhs.avg_rt == rhs.avg_rt &&
               lhs.max_rt == rhs.max_rt && lhs.min_rt == rhs.min_rt &&
               lhs.response_codes_ok == rhs.response_codes_ok &&
               lhs.response_codes_nok == rhs.response_codes_nok;
    }

    //Histo (id, code, timestamp)

    int64_t sent = 0;
    int64_t responded_ok = 0;
    int64_t timed_out = 0;
    float rate = 0;
    float avg_rt = 0;
    float max_rt = 0;
    float min_rt = 0;
    std::map<int, int64_t> response_codes_ok{};
    std::map<int, int64_t> response_codes_nok{};
    time_point<steady_clock> init_time{steady_clock::now()};
};

class stats : public stats_if
{
public:
    stats(boost::asio::io_context& io_ctx, const int print_period,
          const std::string& output_file_name, const std::vector<std::string>& msg_names);

    stats(const stats& s) = delete;

    ~stats() = default;

    void print();
    void end();

    void increase_sent(const std::string& id) override;
    void add_measurement(const std::string& id, const int64_t time, const int code) override;
    void add_timeout(const std::string& id) override;
    void add_error(const std::string& id, const int e) override;
    void add_client_error(const std::string& id, const int e) override;

protected:
    static std::string create_headers_str();
    void write_headers(std::fstream& fs);
    void write_errors() const;
    void print_headers() const;
    void print_snapshot(const snapshot& snap, const time_point<steady_clock>& init_time,
                        std::ostream& out = std::cout) const;
    void do_print();

    void update_rcs(snapshot& snap, const int code, const bool is_error);
    void update_rts(snapshot& snap, const int64_t elapsed_time);
    void add_measurement(snapshot& snap, const int64_t elapsed_time, const int code);

    boost::asio::steady_timer timer;
    int print_period;
    bool cancel;
    std::atomic<int64_t> counter;

    std::string file_prefix;
    std::string accum_filename;
    std::string partial_filename;
    std::string err_filename;

    snapshot total_snap;
    snapshot partial_snap;
    std::map<std::string, snapshot> msg_snaps;

    mutable mutex_type rw_mutex;

    const std::string stats_headers;

    opentelemetry::v1::nostd::unique_ptr<opentelemetry::v1::metrics::Counter<double>> double_counter;

};
}  // namespace stats
