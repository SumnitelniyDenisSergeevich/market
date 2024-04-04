#include <cstdlib>
#include <iostream>

#include "Server.h"
#include "Session.h"

#include <boost/bind/bind.hpp>

Server::Server(boost::asio::io_service& io_service)
    : io_service_(io_service),
    acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
{
    std::cout << "Server started! Listen " << port << " port" << std::endl;

    session* new_session = new session(io_service_, this);

    acceptor_.async_accept(new_session->socket(),
        boost::bind(&Server::handle_accept, this, new_session,
            boost::asio::placeholders::error));
}

Server::~Server()
{}

void Server::handle_accept(session* new_session,
    const boost::system::error_code& error)
{
    if (!error)
    {
        new_session->start();
        new_session = new session(io_service_, this);

        acceptor_.async_accept(new_session->socket(),
            boost::bind(&Server::handle_accept, this, new_session,
                boost::asio::placeholders::error));
    }
    else
    {
        delete new_session;
    }
}

void Server::SendNotificationsAboutTransactions(std::vector<DealData> deals)
{
    for (const DealData& deal : deals)
    {
        double amount = deal.count * deal.price;
        session* buyer_session = FindFeedbackSession(deal.buyer_id);
        session* seller_session = FindFeedbackSession(deal.seller_id);

        if (buyer_session)
        {
            purchase_reply_ = "Notification: You bought " + std::to_string(deal.count) + " USD for " + std::to_string(amount) + " RUB;\n";
            boost::asio::async_write(buyer_session->socket(),
                boost::asio::buffer(purchase_reply_, purchase_reply_.size()),
                boost::bind(&Server::handle_write, this, boost::asio::placeholders::error));
        }
        if (seller_session)
        {
            sale_reply_ = "Notification: You sold " + std::to_string(deal.count) + " USD for " + std::to_string(amount) + " RUB;\n";
            boost::asio::async_write(seller_session->socket(),
                boost::asio::buffer(sale_reply_, sale_reply_.size()),
                boost::bind(&Server::handle_write, this, boost::asio::placeholders::error));
        }
    }
}

void Server::RegFeedbackSession(int user_id, session* s)
{
    id_session[user_id] = s;
}

session* Server::FindFeedbackSession(int user_id)
{
    auto iter = id_session.find(user_id);
    return iter != id_session.end() ? iter->second : nullptr;
}

void Server::handle_write(const boost::system::error_code& error)
{

}

