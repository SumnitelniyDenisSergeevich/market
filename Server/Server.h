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
    void SendNotificationsAboutTransactions(std::vector<DealData> deals);
    void RegFeedbackSession(int user_id, Session* s);
    void CloseFeedbackSession(int user_id);

private:
    Session* FindFeedbackSession(int user_id);

private:
    void handle_accept(Session* new_session, const boost::system::error_code& error);
    void handle_write(const boost::system::error_code& error);

private:
    std::map<int, Session*> id_session_;
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
};
