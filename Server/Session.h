#pragma once

#include <memory>
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
    void insertCompletedDeals(const DealData& deal);
    void UpdateUsersBalance(const BalanceChanges& user_id_income);
    void DeleteRequests(const std::vector<std::string>& delete_req);
    void UpdateRequests(const std::map<std::string, int>& req_id_count);
    void InsertRequest(const std::string& req);
    void UpdateUsdQuote(const std::string quote);

public:
    tcp::socket& Socket();

private:
    void ReadMessage();
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
    void handle_write(const boost::system::error_code& error);
    void handle_write_table(const std::vector<std::string>& data, const boost::system::error_code& error);
    void handle_read_table(const std::vector<std::string>& data, const boost::system::error_code& error, size_t bytes_transferred);
    void handle_write_empty(const boost::system::error_code& error);

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
