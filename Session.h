#pragma once

#include <boost/asio.hpp>
#include "Core.h"
using boost::asio::ip::tcp;

class Server;

class session //можно сделать вложенным классом, тогда можно будет использовать методы server
{
public:
    session(boost::asio::io_service& io_service, Server* serv);

    void start();
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
    void handle_write(const boost::system::error_code& error);

public:
    tcp::socket& socket();
    int UserId();

private:
    std::string reply_;
    int user_id_;

    tcp::socket socket_;
    Server* server_;
    enum { max_length = 1024 };
    char data_[max_length];

    static Core core_;
};
