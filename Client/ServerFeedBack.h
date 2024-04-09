#pragma once

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class ServerFeedback
{
public:
    ServerFeedback(boost::asio::io_service& io);

    void Start();

    tcp::socket& Socket();

    void handle_read(const boost::system::error_code& error, size_t bytes_transferred);

private:
    tcp::socket s_feedback_;
    enum { max_length = 1024 };
    char data_[max_length];
};
