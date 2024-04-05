#include <boost/bind/bind.hpp>
#include <iostream>
#include <string>

#include "Server.h"
#include "Session.h"
#include "json.hpp"

Core Session::core_("dbname=stock_market user=market_admin password=1 host=localhost port=5432");

Session::Session(boost::asio::io_service& io_service, Server* serv)
    : user_id_(0)
    , socket_(io_service)
    , server_(serv)
{
}

tcp::socket& Session::Socket()
{
    return socket_;
}

void Session::start()
{
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&Session::handle_read, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

void Session::SendTable(const std::vector<std::string>& data)
{
    std::string data_size = std::to_string(data.size());

    boost::asio::async_write(socket_,
        boost::asio::buffer(data_size.c_str(), data_size.size()),
        boost::bind(&Session::handle_write_table, this, std::move(data),
            boost::asio::placeholders::error));
}

void Session::handle_write_table(const std::vector<std::string> &data, const boost::system::error_code& error)
{
    if (!error)
    {
        if (data.size())
            socket_.async_read_some(boost::asio::buffer(data_, max_length),
                boost::bind(&Session::handle_read_table, this, std::move(data),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
        else
            socket_.async_read_some(boost::asio::buffer(data_, max_length),
                boost::bind(&Session::handle_read, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        delete this;
    }
}

void Session::handle_read_table(const std::vector<std::string>& data, const boost::system::error_code& error, size_t bytes_transferred)
{
    if (!error)
    {
        for (std::string req : data)
        {
            boost::asio::async_write(socket_,
                boost::asio::buffer(req.c_str(), req.size()),
                boost::bind(&Session::handle_write_str, this,
                    boost::asio::placeholders::error));
        }
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            boost::bind(&Session::handle_read, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        delete this;
    }
}

void Session::handle_write_str(const boost::system::error_code& error)
{
    if (error)
        delete this;
}

void Session::handle_read(const boost::system::error_code& error,
    size_t bytes_transferred)
{
    if (!error)
    {
        data_[bytes_transferred] = '\0';
        auto j = nlohmann::json::parse(data_);
        auto reqType = j["ReqType"];

        std::string reply = "Error! Unknown request type";
        if (reqType == Requests::Registration)
            reply = core_.RegisterNewUser(j["Login"], j["Password"]);
        else if (reqType == Requests::LogIn)
            reply = core_.LogIn(j["Login"], j["Password"]);
        else if (reqType == Requests::AddRequestSale)
        {
            std::cout << "UserId: " << user_id_ << " add sale request" << std::endl;
            reply = core_.AddRequestSale(j["UserId"], j["Count"], j["Price"]);
        }
        else if (reqType == Requests::AddRequestPurchase)
        {
            std::cout << "UserId: " << user_id_ << " add purchase request" << std::endl;
            reply = core_.AddRequestPurchase(j["UserId"], j["Count"], j["Price"]);
        }
        else if (reqType == Requests::CancelReq)
            reply = core_.CancelRequest(j["UserId"], j["Message"]);
        else if (reqType == Requests::USDQuotes)
        {
            std::cout << "UserId: " << user_id_ << " asked husd quotes" << std::endl;
            reply = core_.GetUSDQuotes();
        }
        else if (reqType == Requests::SFeedBackReg)
            server_->RegFeedbackSession(user_id_, this);
        else if (reqType == Requests::ActiveRequests)
        {
            std::cout << "UserId: " << user_id_ << " asked active requests" << std::endl;
            SendTable(core_.GetActiveRequests());
        }
        else if (reqType == Requests::MyActiveRequests)
        {
            std::cout << "UserId: " << user_id_ << " asked his requests" << std::endl;
            SendTable(core_.GetActiveUserRequests(j["UserId"]));
        }
        else if (reqType == Requests::CompletedTransactions)
        {
            std::cout << "UserId: " << user_id_ << " asked his deals" << std::endl;
            SendTable(core_.GetCompletedDeals(j["UserId"]));
        }
        else if (reqType == Requests::Balance)
        {
            std::cout << "UserId: " << user_id_ << " asked balance" << std::endl;
            auto balance = core_.GetUserbalance(j["UserId"]);
            reply = "Your balance is: " + balance.first + " RUB, " + balance.second + " USD";
        }        
        else if (reqType == Requests::LogOut)
        {
            std::cout << "UserId: " << user_id_ << " log out" << std::endl;
            core_.LogOut(j["UserId"]);
            server_->CloseFeedbackSession(user_id_);
        }

        if (reqType == Requests::Registration || reqType == Requests::LogIn)
            user_id_ = stoi(reply);

        if (reqType == Requests::AddRequestSale || reqType == Requests::AddRequestPurchase)
            server_->SendNotificationsAboutTransactions(core_.ExecuteRequests());

        if (reqType != Requests::SFeedBackReg && reqType != Requests::ActiveRequests &&
                reqType != Requests::MyActiveRequests && reqType != Requests::CompletedTransactions)
        {
            boost::asio::async_write(socket_,
                boost::asio::buffer(reply.c_str(), reply.size()),
                boost::bind(&Session::handle_write, this,
                    boost::asio::placeholders::error));
        }
    }
    else
    {
        delete this;
    }
}

void Session::handle_write(const boost::system::error_code& error)
{
    if (!error)
    {
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            boost::bind(&Session::handle_read, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        delete this;
    }
}
