#include "mainwindow.h"

#include <QApplication>
#include <boost/asio.hpp>

#include "Common.hpp"

int main(int argc, char *argv[])
{
    qRegisterMetaType<std::string>("std::string");
    qRegisterMetaType<std::vector<std::string>>("std::vector<std::string>");
    qRegisterMetaType<std::map<std::string,int>>("std::map<std::string,int>");

    QApplication a(argc, argv);

    boost::asio::io_service io_service;
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(tcp::v4(), "127.0.0.1", std::to_string(port));
    tcp::resolver::iterator iterator = resolver.resolve(query);

    MainWindow w(io_service, iterator);
    w.start();

    return a.exec();
}
