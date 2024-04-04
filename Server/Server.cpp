#include <cstdlib>
#include <iostream>
#include <boost/bind/bind.hpp>

#include "Server.h"
#include "Session.h"

Server::Server(boost::asio::io_service& io_service)
    : io_service_(io_service),
    acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
{
    std::cout << "Server started! Listen " << port << " port" << std::endl;

    Session* new_session = new Session(io_service_, this);

    acceptor_.async_accept(new_session->Socket(),
        boost::bind(&Server::handle_accept, this, new_session,
            boost::asio::placeholders::error));
}

Server::~Server()
{
    for(auto [_,s] : id_session_)
        delete s;
}

void Server::RegFeedbackSession(int user_id, Session* s)
{
    id_session_[user_id] = s;
}

void Server::CloseFeedbackSession(int user_id)
{
    auto iter = id_session_.find(user_id);
    if (iter != id_session_.end())
    {
        Session* s = iter->second;
        id_session_.erase(iter);
        delete s;
    }
}

Session* Server::FindFeedbackSession(int user_id)
{
    auto iter = id_session_.find(user_id);
    return iter != id_session_.end() ? iter->second : nullptr;
}

void Server::SendNotificationsAboutTransactions(std::vector<DealData> deals)
{
    for (const DealData& deal : deals)
    {
        double amount = deal.count * deal.price;
        Session* buyer_session = FindFeedbackSession(deal.buyer_id);
        Session* seller_session = FindFeedbackSession(deal.seller_id);

        if (buyer_session)
        {
            std::string purchase_reply = "\nNotification: You bought " + std::to_string(deal.count) + " USD for " + std::to_string(amount) + " RUB;\n> ";
            boost::asio::async_write(buyer_session->Socket(),
                boost::asio::buffer(purchase_reply.c_str(), purchase_reply.size()),
                boost::bind(&Server::handle_write, this, boost::asio::placeholders::error));
        }
        if (seller_session)
        {
            std::string sale_reply = "\nNotification: You sold " + std::to_string(deal.count) + " USD for " + std::to_string(amount) + " RUB;\n> ";
            boost::asio::async_write(seller_session->Socket(),
                boost::asio::buffer(sale_reply.c_str(), sale_reply.size()),
                boost::bind(&Server::handle_write, this, boost::asio::placeholders::error));
        }
    }
}

void Server::handle_accept(Session* new_session,
    const boost::system::error_code& error)
{
    if (!error)
    {
        new_session->start();
        new_session = new Session(io_service_, this);

        acceptor_.async_accept(new_session->Socket(),
            boost::bind(&Server::handle_accept, this, new_session,
                boost::asio::placeholders::error));
    }
    else
    {
        delete new_session;
    }
}

void Server::handle_write(const boost::system::error_code& error) {}
