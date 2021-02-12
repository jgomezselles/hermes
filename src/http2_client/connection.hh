#ifndef SRC_HTTP2_CLIENT_CONNECTION_HH_
#define SRC_HTTP2_CLIENT_CONNECTION_HH_

/// External includes

#include <nghttp2/asio_http2_client.h>

#include <chrono>
#include <functional>
#include <string>
#include <thread>

namespace nghttp2
{
namespace asio_http2
{
namespace client
{
class session;
}
}  // namespace asio_http2
}  // namespace nghttp2

namespace http2_client
{
class connection;
using connection_callback = std::function<void(connection&)>;

class connection
{
public:
    /**
     * Enumeration for all possible connection status
     */
    enum class status
    {
        NOT_OPEN,
        OPEN,
        CLOSED
    };

public:
    connection(const std::string& host, const std::string& port);

    connection(const connection& o) = delete;
    connection(connection&& o) = delete;

    ~connection();

    connection& operator=(const connection& o) = delete;
    connection& operator=(connection&& o) = delete;

    nghttp2::asio_http2::client::session& get_session() { return session; };
    const status& get_status() const { return status; };
    bool wait_to_be_connected();
    void close();

private:
    void close_impl();
    void notify_close();

private:
    /// ASIO attributes
    boost::asio::io_service io_service;
    boost::asio::io_service::work svc_work;
    nghttp2::asio_http2::client::session session;

    /// Class attributes
    status status;

    /// Concurrency attributes
    std::mutex mtx;
    std::thread worker;
    std::condition_variable status_change_cond_var;
};

}  // namespace http2_client

#endif /* SRC_HTTP2_CLIENT_CONNECTION_HH_ */
