#pragma once

#include <boost/asio.hpp>
#include <map>

#include "Common.hpp"

using boost::asio::ip::tcp;
class session;

class Server
{
public:
    Server(boost::asio::io_service& io_service);
    ~Server();

    void handle_accept(session* new_session, const boost::system::error_code& error);
    void handle_write(const boost::system::error_code& error);
    void SendNotificationsAboutTransactions(std::vector<DealData> deals);
    void RegFeedbackSession(int user_id, session* s);

private:
    session* FindFeedbackSession(int user_id);

private:
    std::string purchase_reply_;
    std::string sale_reply_;

    std::map<int, session*> id_session;
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
};