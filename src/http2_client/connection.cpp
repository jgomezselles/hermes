#include "connection.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <iostream>

using boost::asio::ip::tcp;
using nghttp2::asio_http2::client::session;
namespace http2_client
{
connection::connection(const std::string& h, const std::string& p)
    : io_service(),
      svc_work(boost::asio::io_service::work(io_service)),
      session(io_service, h, p),
      connection_status(status::NOT_OPEN)
{
    session.on_connect([this, h, p](tcp::resolver::iterator endpoint_it) {
        std::cerr << "Connected to " << h << ":" << p << std::endl;
        connection_status = status::OPEN;
        status_change_cond_var.notify_one();
    });

    session.on_error([this, h, p](const boost::system::error_code& ec) {
        std::cerr << "Error in connection to " << h << ":" << p
                  << " Message: " << ec.message().c_str() << std::endl;
        notify_close();
    });

    worker = std::thread([&] { io_service.run(); });
}

connection::~connection()
{
    io_service.stop();

    if (worker.joinable())
    {
        worker.join();
    }

    if (connection_status == status::OPEN)
    {
        session.shutdown();
    }
}

void connection::notify_close()
{
    connection_status = status::CLOSED;
    status_change_cond_var.notify_one();
}

void connection::close()
{
    session.shutdown();
    notify_close();
}

bool connection::wait_to_be_connected()
{
    std::unique_lock<std::mutex> lock(mtx);
    status_change_cond_var.wait_for(lock, std::chrono::duration<int, std::milli>(2000),
                                    [&] { return (connection_status != status::NOT_OPEN); });
    return (connection_status == status::OPEN);
}

bool connection::wait_for_status(const std::chrono::duration<int, std::milli>& max_time,
                                 const status& st)
{
    std::unique_lock<std::mutex> lock(mtx);
    return status_change_cond_var.wait_for(lock, max_time,
                                           [this, &st] { return connection_status == st; });
}

}  // namespace http2_client
