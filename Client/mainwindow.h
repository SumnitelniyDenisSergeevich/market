#pragma once

#include <QMainWindow>
#include <QStandardItemModel>

#include <boost/asio.hpp>

#include "RequestsFilterModel.h"
#include "LogIn.h"
#include "RequestDialog.h"
#include "ServerFeedBack.h"

#include <string>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(boost::asio::io_service &io_service, boost::asio::ip::tcp::resolver::iterator iterator, QWidget *parent = nullptr);
    ~MainWindow();

public:
    void start();

private:
    void ProcessLogIn(const QString& login, const QString& password);
    void ProcessRegistration(const QString& login, const QString& password);
    QString ProcessCancelRequest(const QString& reqId);

    void PrintTable(QStandardItemModel *m, const QString& rows_count_str);

private:
    QString ReadMessage();
    void SendMessage(
        const std::string& requestType,
        const QString& message);
    void SendLogMessage(const QString& requestType,
                        const QString& login,
                        const QString& password);
    void SendRequestMessage(const bool forSale,
                            const int dollarsCount,
                            const double dollarPrice);
    void SendRequestMessage(const std::string& requestType);

private:
    Ui::MainWindow *ui;
    LogIn m_login;
    RequestDialog m_requestDialog;

    boost::asio::io_service &m_ioService;
    boost::asio::ip::tcp::socket m_socket;
    ServerFeedback m_feedback;

    QStandardItemModel m_requestsModel;
    QStandardItemModel m_completedDeals;

    RequestsFilterModel m_userReqFilterModel;

    QString m_myId;
};
