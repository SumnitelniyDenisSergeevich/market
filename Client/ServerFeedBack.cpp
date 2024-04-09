#include "ServerFeedBack.h"

#include <iostream>
#include <boost/bind/bind.hpp>

ServerFeedback::ServerFeedback(boost::asio::io_service& io)
    : s_feedback_(io)
{    }

void ServerFeedback::Start()
{
    s_feedback_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&ServerFeedback::handle_read, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

tcp::socket& ServerFeedback::Socket()
{
    return s_feedback_;
}

void ServerFeedback::handle_read(const boost::system::error_code& error,
                 size_t bytes_transferred)
{
    if (!error)
    {
        data_[bytes_transferred] = '\0';
        std::cout << data_;

        s_feedback_.async_read_some(boost::asio::buffer(data_, max_length),
            boost::bind(&ServerFeedback::handle_read, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
}
