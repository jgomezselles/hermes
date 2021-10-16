#include <atomic>
#include <boost/asio.hpp>
#include <memory>
#include <mutex>
#include <shared_mutex>

#pragma once

#include "client.hpp"
#include "client_utils.hpp"
#include "connection.hpp"
#include "script_queue.hpp"

namespace stats
{
class stats_if;
}

namespace http2_client
{
class client_impl : public client
{
public:
    client_impl(std::shared_ptr<stats::stats_if> stats, boost::asio::io_context& io_ctx,
                std::unique_ptr<traffic::script_queue_if> q, const std::string& h,
                const std::string& p, const bool secure_session = false);

    virtual ~client_impl() {}

    void send() override;
    bool has_finished() const override { return !queue->has_pending_scripts(); };
    void close_window() override { queue->close_window(); };
    bool is_connected() const override
    {
        return conn != nullptr && conn->get_status() == connection::status::OPEN;
    };

private:
    void open_new_connection();
    void handle_timeout(const std::shared_ptr<race_control>& control, const std::string& msg_name);
    void handle_timeout_cancelled(const std::shared_ptr<race_control>& control,
                                  const std::string& msg_name);
    void on_timeout(const boost::system::error_code& e, std::shared_ptr<race_control> control,
                    std::string msg_name);

    std::shared_ptr<stats::stats_if> stats;
    boost::asio::io_context& io_ctx;
    std::unique_ptr<traffic::script_queue_if> queue;
    std::string host;
    std::string port;
    bool secure_session;
    std::unique_ptr<connection> conn;
    std::shared_timed_mutex mtx;
};

}  // namespace http2_client
