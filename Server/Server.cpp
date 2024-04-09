#include <cstdlib>
#include <iostream>
#include <boost/bind/bind.hpp>

#include "Server.h"
#include "Session.h"
#include "json.hpp"

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

void Server::insertCompletedDeals(const std::vector<DealData>& deals)
{
    for (const DealData& deal : deals)
    {
        Session* buyer_session = FindFeedbackSession(deal.buyer_id);
        Session* seller_session = FindFeedbackSession(deal.seller_id);

        if (buyer_session)
            buyer_session->insertCompletedDeals(deal);
        if (seller_session)
            seller_session->insertCompletedDeals(deal);
    }
}

void Server::UpdateUsersBalance(const std::map<int, BalanceChanges>& user_id_income)\
{
    for (const auto [user_id, balance] : user_id_income)
    {
        Session* user_session = FindFeedbackSession(user_id);

        if (user_session)
            user_session->UpdateUsersBalance(balance);
    }
}

void Server::UpdateUsdQuotes(const std::string& quote)
{
    for(const auto [user_id, session] : id_session_)
        session->UpdateUsdQuote(quote);
}

void Server::DeleteRequests(const std::vector<std::string>& delete_req)
{
    for(const auto [user_id, session] : id_session_)
        session->DeleteRequests(delete_req);
}

void Server::UpdateRequests(const std::map<std::string, int>& req_id_count)
{
    for(const auto [user_id, session] : id_session_)
        session->UpdateRequests(req_id_count);
}

void Server::InsertRequest(const std::string& req)
{
    for(const auto [user_id, session] : id_session_)
        session->InsertRequest(req);
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
