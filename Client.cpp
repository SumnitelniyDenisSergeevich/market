#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread.hpp>

#include "Common.hpp"
#include "json.hpp"

using boost::asio::ip::tcp;

// Отправка сообщения на сервер по шаблону.
void SendMessage(
    tcp::socket& aSocket,
    const std::string& aId,
    const std::string& aRequestType,
    const std::string& aMessage)
{
    nlohmann::json req;
    req["UserId"] = aId;
    req["ReqType"] = aRequestType;
    req["Message"] = aMessage;

    std::string request = req.dump();
    boost::asio::write(aSocket, boost::asio::buffer(request, request.size()));
}

void SendLogMessage(
    tcp::socket& aSocket,
    const std::string& aId,
    const std::string& aRequestType,
    const std::string& aLogin,
    const std::string& aPassword)
{
    nlohmann::json req;
    req["UserId"] = aId;
    req["ReqType"] = aRequestType;
    req["Login"] = aLogin;
    req["Password"] = aPassword;

    std::string request = req.dump();
    boost::asio::write(aSocket, boost::asio::buffer(request, request.size()));
}

void SendRequestMessage(
    tcp::socket& aSocket,
    const std::string& aId,
    const std::string& aRequestType,
    const std::string& dollars_count,
    const std::string& dollar_price)
{
    nlohmann::json req;
    req["UserId"] = aId;
    req["ReqType"] = aRequestType;
    req["Count"] = dollars_count;
    req["Price"] = dollar_price;

    std::string request = req.dump();
    boost::asio::write(aSocket, boost::asio::buffer(request, request.size()));
}

// Возвращает строку с ответом сервера на последний запрос.
std::string ReadMessage(tcp::socket& aSocket)
{
    boost::asio::streambuf b;
    boost::asio::read_until(aSocket, b, "\0");
    std::istream is(&b);
    std::string line(std::istreambuf_iterator<char>(is), {});
    return line;
}

// "Создаём" пользователя, получаем его ID.
std::string ProcessRegistration(tcp::socket& aSocket)
{
    std::string name;
    std::string password;

    std::cout << "Enter login: ";
    std::cin >> name;

    std::cout << "Enter password: ";
    std::cin >> password;

    // Для регистрации Id не нужен, заполним его нулём

    return ReadMessage(aSocket);
}

void RegistrateFeedback(tcp::socket& s, std::string my_id)
{
    SendMessage(s, my_id, Requests::SFeedBackReg, "");
}

// "Входим" в аккаунт пользвателя.
std::string ProcessLogIn(tcp::socket& aSocket)
{
    std::string name;
    std::string password;

    std::cout << "Enter login: ";
    std::cin >> name;

    std::cout << "Enter password: ";
    std::cin >> password;

    // Для регистрации Id не нужен, заполним его нулём
    SendLogMessage(aSocket, "0", Requests::LogIn, name, password);
    return ReadMessage(aSocket);
}

// "Входим" в аккаунт пользвателя.
std::string ProcessAddRequest(tcp::socket& aSocket, const std::string& id, const std::string& aRequestType)
{
    std::string dollars_count;
    std::string dollar_price;

    std::cout << "Enter the number of dollars : ";
    std::cin >> dollars_count;

    std::cout << "Specify the price for one dollar: ";
    std::cin >> dollar_price;


    SendRequestMessage(aSocket, id, aRequestType, dollars_count, dollar_price);
    return ReadMessage(aSocket);
}

class ServerFeedback
{
public:
    ServerFeedback(boost::asio::io_service& io)
        : s_feedback_(io)
    {
    }

    void Start()
    {
        s_feedback_.async_read_some(boost::asio::buffer(data_, max_length),
            boost::bind(&ServerFeedback::handle_read, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

    tcp::socket& Socket()
    {
        return s_feedback_;
    }

    void handle_read(const boost::system::error_code& error,
                     size_t bytes_transferred)
    {
        if (!error)
        {
            data_[bytes_transferred] = '\0';

            std::cout << "data come!!!!!!!" << std::endl;
            std::cout << data_ << std::endl;

            s_feedback_.async_read_some(boost::asio::buffer(data_, max_length),
                boost::bind(&ServerFeedback::handle_read, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
        }
        else
        {
            std::cout << "EROR" << std::endl;
        }
    }

private:
    tcp::socket s_feedback_;
    enum { max_length = 1024 };
    char data_[max_length];
};

int main()
{
    try
    {
        boost::asio::io_service io_service;

        tcp::resolver resolver(io_service);
        tcp::resolver::query query(tcp::v4(), "127.0.0.1", std::to_string(port));
        tcp::resolver::iterator iterator = resolver.resolve(query);

        tcp::socket s(io_service);
        s.connect(*iterator);

        std::string my_id = "0";
        short menu_option_num;

        while (my_id == "0")
        {
            std::cout << "Login:\n"
                "1) Log In\n"
                "2) Registration\n"
                "3) Exit\n"
                << "> ";

            std::cin >> menu_option_num;

            switch (menu_option_num)
            {
            case 1:
                my_id = ProcessLogIn(s);
                if (my_id != "0")
                    std::cout << "Complete!\n";
                else
                    std::cout << "Try again!\n";
                break;
            case 2:
                my_id = ProcessRegistration(s);
                if (my_id != "0")
                    std::cout << "Regisration is complete!\n";
                else
                    std::cout << "User with this Login is created, try another login!\n";
                break;
            case 3:
                exit(0);
                break;
            default:
                break;
            }
        }

        ServerFeedback feedback(io_service);

        feedback.Socket().connect(*iterator);
        RegistrateFeedback(feedback.Socket(), my_id);
        feedback.Start();

        boost::thread ClientThread(boost::bind(&boost::asio::io_service::run, &io_service));

        while (true)
        {
            // Тут реализовано "бесконечное" меню.
            std::cout << "Menu:\n"
                "1) Balance Request\n"
                "2) Add Request for sale\n"
                "3) Add Request for purchase\n"
                "4) Exit\n"
                "> ";

            std::cin >> menu_option_num;
            switch (menu_option_num)
            {
            case 1:
            {
                SendMessage(s, my_id, Requests::Balance, "");
                std::cout << "Your balance is: " << ReadMessage(s) << "!\n";
                break;
            }
            case 2:
            {
                std::cout << ProcessAddRequest(s, my_id, Requests::AddRequestSale) << "\n";
                break;
            }
            case 3:
            {
                std::cout << ProcessAddRequest(s, my_id, Requests::AddRequestPurchase) << "\n";
                break;
            }
            case 4:
            {
                exit(0);
                break;
            }
            default:
            {
                std::cout << "Unknown menu option\n" << std::endl;
            }
            }

        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
