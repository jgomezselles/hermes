#include <atomic>
#include <boost/asio.hpp>
#include <memory>
#include <mutex>
#include <shared_mutex>

#pragma once

#include "client.hpp"
#include "connection.hpp"
#include "script_queue.hpp"

namespace stats
{
class stats;
}

namespace http2_client
{
struct race_control
{
    race_control() : timed_out(false), answered(false) {}
    bool timed_out;
    bool answered;
    std::mutex mtx;
};

struct request
{
    std::string url;
    std::string method;
    std::string body;
    nghttp2::asio_http2::header_map headers;
    std::string name;
};

class client_impl : public client
{
public:
    enum class method
    {
        GET,
        PUT,
        POST,
        DELETE
    };

    client_impl(std::shared_ptr<stats::stats> stats, boost::asio::io_context& io_ctx,
                const traffic::script& s);

    virtual ~client_impl() {}

    void send() override;
    bool has_finished() const override { return !queue.has_pending_scripts(); };
    void close_window() override { queue.close_window(); };
    bool is_connected() const override
    {
        return conn != nullptr && conn->get_status() == connection::status::OPEN;
    };

private:
    void open_new_connection();
    request get_next_request(const traffic::script& s);
    std::string get_uri(const std::string& uri_path);
    void handle_timeout(const std::shared_ptr<race_control>& control, const std::string& msg_name);
    void handle_timeout_cancelled(const std::shared_ptr<race_control>& control,
                                  const std::string& msg_name);
    void on_timeout(const boost::system::error_code& e, std::shared_ptr<race_control> control,
                    std::string msg_name);

    std::shared_ptr<stats::stats> stats;
    boost::asio::io_context& io_ctx;
    traffic::script_queue queue;
    std::string host;
    std::string port;
    std::unique_ptr<connection> conn;
    std::shared_timed_mutex mtx;
};

}  // namespace http2_client
