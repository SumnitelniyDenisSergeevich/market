#pragma once

#include <QObject>
#include <boost/asio.hpp>
#include <map>

using boost::asio::ip::tcp;

class ServerFeedback : public QObject
{
    Q_OBJECT
public:
    ServerFeedback(boost::asio::io_service& io);

    void Start();
    tcp::socket& Socket();
    void regFeedbackSocket(const QString &userId);

public:
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
    void handle_write(const boost::system::error_code& error);

signals:
    void updateBalace(double income, int count);
    void deleteReq(std::vector<std::string> delete_req);
    void updateReq(std::map<std::string, int> req_id_count);
    void insertReq(const std::string& dump_request);
    void insertCompletedDeal(double price, int count, std::string seller, std::string buyer);
    void updateUsdQuote(double quote);

private:
    tcp::socket m_feedbackSocket;
    enum { max_length = 1024 };
    char data_[max_length];
};
