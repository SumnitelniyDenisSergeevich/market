
#include <boost/bind/bind.hpp>

#include "Session.h"
#include "Core.h"
#include "Common.hpp"
#include "json.hpp"
#include <string>
#include "Server.h"

#include <iostream>

Core session::core_;

session::session(boost::asio::io_service& io_service, Server* serv)
    : user_id_(0)
    , socket_(io_service)
    , server_(serv)
{
}

tcp::socket& session::socket()
{
    return socket_;
}

int session::UserId()
{
    return user_id_;
}

void session::start()
{
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

void session::handle_read(const boost::system::error_code& error,
    size_t bytes_transferred)
{
    if (!error)
    {
        data_[bytes_transferred] = '\0';

        // Парсим json, который пришёл нам в сообщении.
        auto j = nlohmann::json::parse(data_);

        auto reqType = j["ReqType"];

        reply_ = "Error! Unknown request type";
        if (reqType == Requests::Registration)
            reply_ = core_.RegisterNewUser(j["Login"], j["Password"]);
        else if (reqType == Requests::LogIn)
            reply_ = core_.LogIn(j["Login"], j["Password"]);
        else if (reqType == Requests::AddRequestSale)
            reply_ = core_.AddRequestSale(j["UserId"], j["Count"], j["Price"]);
        else if (reqType == Requests::AddRequestPurchase)
            reply_ = core_.AddRequestPurchase(j["UserId"], j["Count"], j["Price"]);
        else if (reqType == Requests::SFeedBackReg)
        {
            server_->RegFeedbackSession(std::stoi(std::string{j["UserId"]}), this);
            std::cout << "REG FEEDBACK SESSION" << std::endl;
        }
        else if (reqType == Requests::Balance)
        {
            auto balance = core_.GetUserbalance(j["UserId"]);
            reply_ = balance.first + " RUB, " + balance.second + " USD";
        }
        else if (reqType == Requests::ActiveRequests)
        {

        }
        else if (reqType == Requests::CompletedTransactions)
        {

        }
        else if (reqType == Requests::USDQuotes)
        {

        }
        else if (reqType == Requests::LogOut)// Разрешить вход, удалить из notify списка
        {

        }
        else if (reqType == Requests::CancelReq)
        {

        }

        if (reqType == Requests::Registration || reqType == Requests::LogIn)
            user_id_ = stoi(reply_);

        if (reqType == Requests::AddRequestSale || reqType == Requests::AddRequestPurchase)
            server_->SendNotificationsAboutTransactions(core_.ExecuteRequests());

        if (reqType != Requests::SFeedBackReg)
            boost::asio::async_write(socket_,
                boost::asio::buffer(reply_, reply_.size()),//не успевал создаваться, закинул в private
                boost::bind(&session::handle_write, this,
                    boost::asio::placeholders::error));
    }
    else
    {
        delete this;
    }
}

void session::handle_write(const boost::system::error_code& error)
{
    if (!error)
    {
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            boost::bind(&session::handle_read, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        delete this;
    }
}
