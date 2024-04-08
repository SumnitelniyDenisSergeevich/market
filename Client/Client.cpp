#include "mainwindow.h"

#include <QApplication>
#include <boost/asio.hpp>

#include "Common.hpp"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    boost::asio::io_service io_service;

    tcp::resolver resolver(io_service);
    tcp::resolver::query query(tcp::v4(), "127.0.0.1", std::to_string(port));
    tcp::resolver::iterator iterator = resolver.resolve(query);
    boost::thread ClientThread(boost::bind(&boost::asio::io_service::run, &io_service));

    MainWindow w(io_service, iterator);

    w.start();

    return a.exec();
}










//#include <iostream>
//#include <boost/asio.hpp>
//#include <boost/thread.hpp>

//#include "Common.hpp"
//#include "json.hpp"
//#include "ServerFeedBack.h"

//using boost::asio::ip::tcp;

//// Отправка сообщения на сервер по шаблону.
//void SendMessage(
//    tcp::socket& aSocket,
//    const std::string& aId,
//    const std::string& aRequestType,
//    const std::string& aMessage)
//{
//    nlohmann::json req;
//    req["UserId"] = aId;
//    req["ReqType"] = aRequestType;
//    req["Message"] = aMessage;

//    std::string request = req.dump();
//    boost::asio::write(aSocket, boost::asio::buffer(request, request.size()));
//}

//void SendRequestMessage(
//    tcp::socket& aSocket,
//    const std::string& aRequestType)
//{
//    nlohmann::json req;
//    req["ReqType"] = aRequestType;

//    std::string request = req.dump();
//    boost::asio::write(aSocket, boost::asio::buffer(request, request.size()));
//}


//int main()
//{
//    try
//    {

//        ServerFeedback feedback(io_service);
//        feedback.Socket().connect(*iterator);
//        RegistrateFeedback(feedback.Socket(), my_id);
//        feedback.Start();
//        boost::thread ClientThread(boost::bind(&boost::asio::io_service::run, &io_service));//Для оповещений о совершонной сделке

//        while (true)
//        {
//            std::cout << "Menu:\n"
//                "1) Balance Request.\n"
//                "2) Add Request for sale.\n"
//                "3) Add Request for purchase.\n"
//                "4) View active requests.\n"
//                "5) My activer requests. \n"
//                "6) Cancel request.\n"
//                "7) My completed deals.\n"
//                "8) View the history of USD quotes.\n"
//                "9) Exit\n"
//                "> ";

//            std::cin >> menu_option_num;
//            switch (menu_option_num)
//            {

//                case 5://My active requests
//                {
//                    SendMessage(s, my_id, Requests::MyActiveRequests, "");
//                    std::vector<std::string> columns{ "req_id", "d_price", "d_count", "side" };
//                    PrintTable(s, ReadMessage(s), std::move(columns));
//                    break;
//                }
//                case 7://My completed deals
//                {
//                    SendMessage(s, my_id, Requests::CompletedTransactions, "");
//                    std::vector<std::string> columns{ "Buyer", "Seller", "d_price", "d_count" };
//                    PrintTable(s, ReadMessage(s), std::move(columns));
//                    break;
//                }

//            }
//        }
//    }
//    catch (std::exception& e)
//    {
//        std::cerr << "Exception: " << e.what() << "\n";
//    }

//    return 0;
//}
