#pragma once

namespace http2_client
{
class client
{
public:
    virtual ~client() = default;

    virtual void send() = 0;

    virtual bool has_finished() const = 0;

    virtual void close_window() = 0;

    virtual bool is_connected() const = 0;
};
}  // namespace http2_client
