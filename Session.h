#pragma once

#include <boost/asio.hpp>
#include <boost/shared_array.hpp>
#include "Core.h"
using boost::asio::ip::tcp;

class Server;

struct shared_buffer
{
    boost::shared_array<char> buff;
    int size;
    shared_buffer(const std::string& str) : buff(new char[str.size() + 1]), size(str.size() + 1)
    {
        strcpy(buff.get(), str.c_str());
    }
    boost::asio::mutable_buffers_1 asio_buff() const
    {
        return boost::asio::buffer(buff.get(), size);
    }
};

class session //можно сделать вложенным классом, тогда можно будет использовать методы server
{
public:
    session(boost::asio::io_service& io_service, Server* serv);

    void start();
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
    void handle_write(const boost::system::error_code& error);
    void handle_write_table(const boost::system::error_code& error);
    //void handle_read_table(const boost::system::error_code& error, size_t bytes_transferred);

public:
    tcp::socket& socket();
    int UserId();

private:
    void SendTable(std::vector<nlohmann::json> requests);

private:
    //std::string reply_;
    int user_id_;

    tcp::socket socket_;
    Server* server_;
    enum { max_length = 1024 };
    char data_[max_length];

    static Core core_;
};
