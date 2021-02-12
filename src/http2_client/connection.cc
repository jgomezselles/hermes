#include "connection.hh"

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
      status(status::NOT_OPEN)
{
    session.on_connect([this, h, p](tcp::resolver::iterator endpoint_it) {
        std::cerr << "Connected to " << h << ":" << p << std::endl;
        status = status::OPEN;
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

    if (status == status::OPEN)
    {
        session.shutdown();
    }
}

void
connection::notify_close()
{
    status = status::CLOSED;
    status_change_cond_var.notify_one();
}

void
connection::close()
{
    session.shutdown();
    notify_close();
}

bool
connection::wait_to_be_connected()
{
    std::unique_lock<std::mutex> lock(mtx);
    status_change_cond_var.wait_for(lock, std::chrono::duration<int, std::milli>(2000), [&] {
        return (status != http2_client::connection::status::NOT_OPEN);
    });
    return (status == http2_client::connection::status::OPEN);
}

}  // namespace http2_client
