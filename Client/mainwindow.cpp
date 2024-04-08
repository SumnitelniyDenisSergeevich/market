#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QDebug>

#include "Common.hpp"
#include "json.hpp"

//#include "ServerFeedBack.h"

#include <boost/asio.hpp>
//#include <boost/thread.hpp>

using boost::asio::ip::tcp;

MainWindow::MainWindow(boost::asio::io_service& io_service, boost::asio::ip::tcp::resolver::iterator iterator, QWidget *parent) :
    QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_socket(io_service)
    , m_myId("0")
    , m_feedback(io_service)
{
    ui->setupUi(this);

    m_socket.connect(*iterator);

    m_requestsModel.setHorizontalHeaderLabels({ "req_id", "user_login", "user_id", "d_price", "d_count", "side" });
    m_completedDeals.setHorizontalHeaderLabels({ "Buyer", "Seller", "d_price", "d_count" });

    m_userReqFilterModel.setSourceModel(&m_requestsModel);

    ui->AllRequestsTable->setModel(&m_requestsModel);
    ui->MyRequestsTable->setModel(&m_userReqFilterModel);
    ui->CompletedDealsTable->setModel(&m_completedDeals);

    ui->AllRequestsTable->hideColumn(2);
    ui->MyRequestsTable->hideColumn(1);
    ui->MyRequestsTable->hideColumn(2);

    ui->AllRequestsTable->setSortingEnabled(true);
    ui->MyRequestsTable->setSortingEnabled(true);
    ui->CompletedDealsTable->setSortingEnabled(true);

    ui->AllRequestsTable->verticalHeader()->setVisible(false);
    ui->MyRequestsTable->verticalHeader()->setVisible(false);
    ui->CompletedDealsTable->verticalHeader()->setVisible(false);

    ui->AllRequestsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->MyRequestsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->CompletedDealsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_feedback.Socket().connect(*iterator);    

    connect(&m_feedback, &ServerFeedback::updateBalace, this, [this](double income, int count)
    {
        ui->RubSpibBox->setValue(ui->RubSpibBox->value() + income);
        ui->UsdSpinBox->setValue(ui->UsdSpinBox->value() + count);
        qDebug() << "UPDATE BALANCE";
    });

    connect(&m_feedback, &ServerFeedback::deleteReq, this, [this](std::vector<std::string> delete_req)
    {
        for(const std::string& req : delete_req)
        {
            QList<QStandardItem*> removeItem = m_requestsModel.findItems(QString::fromStdString(req));
            m_requestsModel.removeRow(removeItem.first()->index().row());
        }
        qDebug() << "DELETE REQ";
    });

    connect(&m_feedback, &ServerFeedback::updateReq, this, [this](std::map<std::string, int> req_id_count)
    {
        for(const auto& [req_id, count] : req_id_count)
        {
            QList<QStandardItem*> updateItem = m_requestsModel.findItems(QString::fromStdString(req_id));
            m_requestsModel.item(updateItem.first()->index().row(), 4)->setData(count);
        }
        qDebug() << "UPDATE REQ";
    });

    connect(&m_feedback, &ServerFeedback::insertReq, this, [this](std::string dump_request)
    {
        auto j = nlohmann::json::parse(dump_request);
        QList<QStandardItem*> row;
        for (int i = 0; i < m_requestsModel.columnCount(); ++i)
        {
            QString column = m_requestsModel.horizontalHeaderItem(i)->text();
            QStandardItem* item = new QStandardItem(QString::fromStdString(j[column.toStdString()].template get<std::string>()));
            row.append(item);
        }
        m_requestsModel.appendRow(row);
        qDebug() << "INSERT REQ";
    });

    connect(&m_feedback, &ServerFeedback::insertCompletedDeal, this, [this](double price, int count, std::string seller, std::string buyer)
    {
        QList<QStandardItem*> row;
        //NOTIFICATION

        QStandardItem* item = new QStandardItem(QString::fromStdString(buyer));
        row.append(item);
        item = new QStandardItem(QString::fromStdString(seller));
        row.append(item);
        item = new QStandardItem(QString::fromStdString(std::to_string(price)));
        row.append(item);
        item = new QStandardItem(QString::fromStdString(std::to_string(count)));
        row.append(item);

        m_completedDeals.appendRow(row);

        qDebug() << "INSERT COMPLETED DEALS";
    });

    connect(&m_login, &LogIn::logButtonClicked, this, [this](const QString& login, const QString& password)
    {
        ProcessLogIn(login, password);
        if (m_myId != "0")
        {
            m_login.close();
            m_feedback.regFeedbackSocket(m_myId);
//            m_feedback.Start();

            SendMessage(Requests::Balance, "");
            auto j = nlohmann::json::parse(ReadMessage().toStdString());
            ui->RubSpibBox->setValue(QString::fromStdString(std::string{j["RUB"]}).toDouble());
            ui->UsdSpinBox->setValue(std::stoi(std::string{j["USD"]}));

            SendMessage(Requests::USDQuotes, "");
            ui->usdQuotSpinBOx->setValue(ReadMessage().toDouble());

            m_userReqFilterModel.setFilterStr(m_myId);

            SendMessage(Requests::ActiveRequests, "");
            PrintTable(&m_requestsModel, ReadMessage());

            SendMessage(Requests::CompletedTransactions, "");
            PrintTable(&m_completedDeals, ReadMessage());

            ui->AllRequestsTable->resizeColumnsToContents();
            ui->MyRequestsTable->resizeColumnsToContents();
            ui->CompletedDealsTable->resizeColumnsToContents();

            showMaximized();
        }
        else
            m_login.setErrorMessage("The user has already logged in, or the wrong login/password!");
    });

    connect(&m_login, &LogIn::RegButtonClicked, this, [this](const QString& login, const QString& password)
    {
        ProcessRegistration(login, password);
        if (m_myId != "0")
        {
            m_login.close();
            showMaximized();
        }
        else
            m_login.setErrorMessage("User with this Login is created, try another login!");
    });

    connect(ui->AddReqButton, &QPushButton::clicked, this, [this]
    {
        m_requestDialog.show();
    });

    connect(&m_requestDialog, &RequestDialog::addRequestButtonClicked, this, [this](int usdCount, double usdPrice, bool forSale)
    {
        SendRequestMessage(usdCount, usdPrice, forSale);
        QMessageBox msgBox;
        msgBox.setText(ReadMessage());
        msgBox.exec();
    });

    connect(ui->CancelReqButton, &QPushButton::clicked, this, [this]()
    {
        QString reqId = QString::number(ui->reqIdSpinBox->value());
        QMessageBox msgBox;
        msgBox.setText(ProcessCancelRequest(reqId));
        msgBox.exec();
    });

}

MainWindow::~MainWindow()
{
    SendMessage(Requests::LogOut, "");
    delete ui;
}

void MainWindow::start()
{
    m_login.show();
}

QString MainWindow::ReadMessage()
{
    boost::asio::streambuf b;
    boost::asio::read_until(m_socket, b, "\0");
    std::istream is(&b);
    std::string line(std::istreambuf_iterator<char>(is), {});
    return QString::fromStdString(line);
}

void MainWindow::SendMessage(
    const std::string& requestType,
    const QString& message)
{
    nlohmann::json req;
    req["UserId"] = m_myId.toStdString();
    req["ReqType"] = requestType;
    req["Message"] = message.toStdString();

    std::string request = req.dump();
    boost::asio::write(m_socket, boost::asio::buffer(request, request.size()));
}


void MainWindow::SendLogMessage(const QString& requestType,
    const QString& login,
    const QString& password)
{
    nlohmann::json req;
    req["ReqType"] = requestType.toStdString();
    req["Login"] = login.toStdString();
    req["Password"] = password.toStdString();

    std::string request = req.dump();
    boost::asio::write(m_socket, boost::asio::buffer(request, request.size()));
}

void MainWindow::SendRequestMessage(const bool forSale,
    const int dollarsCount,
    const double dollarPrice)
{
    nlohmann::json req;
    req["UserId"] = m_myId.toStdString();
    req["ReqType"] = forSale ? Requests::AddRequestSale : Requests::AddRequestPurchase;
    req["Count"] = std::to_string(dollarsCount);
    req["Price"] = std::to_string(dollarPrice);

    std::string request = req.dump();
    boost::asio::write(m_socket, boost::asio::buffer(request, request.size()));
}

void MainWindow::SendRequestMessage(const std::string& requestType)
{
    nlohmann::json req;
    req["ReqType"] = Requests::RecivedRowsCount;
    std::string request = req.dump();
    boost::asio::write(m_socket, boost::asio::buffer(request, request.size()));
}

void MainWindow::ProcessLogIn(const QString& login, const QString& password)
{
    SendLogMessage(QString::fromStdString(Requests::LogIn), login, password);
    m_myId = ReadMessage();
}

void MainWindow::ProcessRegistration(const QString& login, const QString& password)
{
    SendLogMessage(QString::fromStdString(Requests::Registration), login, password);
    m_myId = ReadMessage();
}

QString MainWindow::ProcessCancelRequest(const QString& reqId)
{
    SendMessage(Requests::CancelReq, reqId);
    return ReadMessage();
}

void MainWindow::PrintTable(QStandardItemModel* m,const QString& rows_count_str)
{
    int rows_count = rows_count_str.toInt();

    if (rows_count)
    {
        SendRequestMessage(Requests::RecivedRowsCount);
        int i = 0;
        QString str = "";

        while (i != rows_count)
        {
            str += ReadMessage();

            size_t s_pos = str.indexOf('{');
            size_t e_pos = str.indexOf('}');

            while (s_pos != -1 && e_pos != -1)
            {
                ++i;
                QStringView line = QStringView{str}.mid(s_pos, e_pos - s_pos + 1);

                nlohmann::json d = nlohmann::json::parse(line.toString().toStdString());

                QList<QStandardItem*> row;
                for (int i = 0; i < m->columnCount(); ++i)
                {
                    QString column = m->horizontalHeaderItem(i)->text();
                    QStandardItem* item = new QStandardItem(QString::fromStdString(d[column.toStdString()].template get<std::string>()));
                    row.append(item);
                }
                m->appendRow(row);

                s_pos = str.indexOf('{', e_pos);
                e_pos = str.indexOf('}', s_pos);
            }
            if (s_pos == -1)
                str = "";
            else if (i != rows_count)
                str = str.mid(s_pos);
        }        
    }
}

//void PrintReqTables(const QString& rows_count_str)
//{
//    int rows_count = rows_count_str.toInt();

//    if (rows_count)
//    {
//        SendRequestMessage(Requests::RecivedRowsCount);
//        int i = 0;
//        QString str = "";

//        while (i != rows_count)
//        {
//            str += ReadMessage();

//            size_t s_pos = str.indexOf('{');
//            size_t e_pos = str.indexOf('}');

//            while (s_pos != -1 && e_pos != -1)
//            {
//                ++i;
//                QStringView line = QStringView{str}.mid(s_pos, e_pos - s_pos + 1);

//                nlohmann::json d = nlohmann::json::parse(line.toString().toStdString());

//                QList<QStandardItem*> row;
//                for (int i = 0; i < m->columnCount(); ++i)
//                {
//                    QString column = m->horizontalHeaderItem(i)->text();
//                    QStandardItem* item = new QStandardItem(QString::fromStdString(d[column.toStdString()].template get<std::string>()));
//                    row.append(item);
//                }
//                m->appendRow(row);

//                s_pos = str.indexOf('{', e_pos);
//                e_pos = str.indexOf('}', s_pos);
//            }
//            if (s_pos == -1)
//                str = "";
//            else if (i != rows_count)
//                str = str.mid(s_pos);
//        }
//    }
//}
