#pragma once

#include <boost/asio.hpp>
#include <map>

#include "Common.hpp"

using boost::asio::ip::tcp;
class Session;

class Server
{
public:
    Server(boost::asio::io_service& io_service);
    ~Server();

public:
    void RegFeedbackSession(int user_id, Session* s);
    void CloseFeedbackSession(int user_id);

public:
    void insertCompletedDeals(const std::vector<DealData>& deals);
    void UpdateUsersBalance(const std::map<int, BalanceChanges>& user_id_income);
    void DeleteRequests(const std::vector<std::string>& delete_req);
    void UpdateRequests(const std::map<std::string, int>& req_id_count);
    void InsertRequest(const std::string& req);
    void UpdateUsdQuotes(const std::string& quote);

private:
    Session* FindFeedbackSession(int user_id);

private:
    void handle_accept(Session* new_session, const boost::system::error_code& error);

private:
    std::map<int, Session*> id_session_;
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
};
