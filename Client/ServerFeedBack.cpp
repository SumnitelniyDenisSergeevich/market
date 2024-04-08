#include "ServerFeedBack.h"

#include "Common.hpp"
#include "json.hpp"
#include <QDebug>

#include <iostream>
#include <boost/bind/bind.hpp>
#include <boost/thread.hpp>

ServerFeedback::ServerFeedback(boost::asio::io_service& io)
    : m_feedbackSocket(io)
{    }

void ServerFeedback::Start()
{
//    m_feedbackSocket.async_read_some(boost::asio::buffer(data_, max_length),
//        boost::bind(&ServerFeedback::handle_read, this,
//            boost::asio::placeholders::error,
//            boost::asio::placeholders::bytes_transferred));


    m_feedbackSocket.async_read_some(boost::asio::buffer(data_, max_length),
                                boost::bind(&ServerFeedback::handle_read, this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));

}

tcp::socket& ServerFeedback::Socket()
{
    return m_feedbackSocket;
}

void ServerFeedback::handle_read(const boost::system::error_code& error,
                 size_t bytes_transferred)
{
    if (!error)
    {
        data_[bytes_transferred] = '\0';

        auto j = nlohmann::json::parse(data_);
        auto reqType = j["ReqType"];

        std::string reply = "ReadData";

        qDebug() << QString::fromStdString(std::string{reqType});

        if (reqType == Requests::UpdateActiveReq)
            emit updateReq(j["UpdatedReq"]);
        else if (reqType == Requests::DeleteActiveReq)
            emit deleteReq(j["DeletedReq"]);
        else if (reqType == Requests::InsertActiveReq)
            emit insertReq(j["Req"]);
        else if (reqType == Requests::UdpadeBalance)
            emit updateBalace(j["ChangedIncome"], j["ChangedCount"]);
        else if (reqType == Requests::InsertCompletedDeals)
            emit insertCompletedDeal(j["Price"], j["Count"], j["Saller"], j["Buyer"]);
        else if (reqType == Requests::UdpadeUsdQuote)
        {

        }
        else
            reply = "NotReadData";

        qDebug() << QString::fromStdString(std::string{reqType});

        boost::asio::async_write(m_feedbackSocket,
                                 boost::asio::buffer(reply.c_str(), reply.size()),
                                 boost::bind(&ServerFeedback::handle_write, this,
                                             boost::asio::placeholders::error));
    }
}

void ServerFeedback::handle_write(const boost::system::error_code& error)
{
    if (!error)
    {
        m_feedbackSocket.async_read_some(boost::asio::buffer(data_, max_length),
                                         boost::bind(&ServerFeedback::handle_read, this,
                                                     boost::asio::placeholders::error,
                                                     boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        delete this;
    }
}

void ServerFeedback::regFeedbackSocket(const QString& userId)
{
    nlohmann::json req;
    req["UserId"] = userId.toStdString();
    req["ReqType"] = Requests::SFeedBackReg;

    std::string request = req.dump();
    boost::asio::write(m_feedbackSocket, boost::asio::buffer(request, request.size()));
}
