#pragma once

#include <boost/asio.hpp>

#include "Core.h"

using boost::asio::ip::tcp;

class Server;

class Session
{
public:
    Session(boost::asio::io_service& io_service, Server* serv);

public:
    void start();

public:
    tcp::socket& Socket();

private:
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
    void handle_write(const boost::system::error_code& error);
    void handle_write_table(const std::vector<std::string>& data, const boost::system::error_code& error);
    void handle_read_table(const std::vector<std::string>& data, const boost::system::error_code& error, size_t bytes_transferred);
    void handle_write_str(const boost::system::error_code& error);

private:
    void SendTable(const std::vector<std::string>& data);

private:    
    tcp::socket socket_;
    Server* server_;
    static Core core_;

    enum { max_length = 1024 };
    char data_[max_length];

    int user_id_;
};
