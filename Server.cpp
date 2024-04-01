#include <cstdlib>
#include <iostream>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <libpq-fe.h>

#include "json.hpp"
#include "Common.hpp"

namespace
{
    struct ReqData
    {
        int user_id;
        int count;
        double price;
        std::string req_id;
    };

    struct DealData
    {
        int buyer_id;
        int seller_id;
        int count;
        double price;
    };
}

using boost::asio::ip::tcp;

class Core
{
public:

    Core()
    {
        char* conninfo = "dbname=stock_market user=market_admin password=1 host=localhost port=5432";

        db_conn_ = PQconnectdb(conninfo);

        if (PQstatus(db_conn_) != CONNECTION_OK)
            throw PQerrorMessage(db_conn_);
        else
            std::cout << "db stock_market connect\n";
    }
    
    ~Core()
    {
        PQfinish(db_conn_);
    }

    // "Регистрирует" нового пользователя и возвращает его ID.
    std::string RegisterNewUser(const std::string& login, const std::string& password)
    {
        std::string result = "0";
        std::string query_str = "SELECT id FROM public.users_log_pus WHERE login = '" + login + "';";

        char* query = new char[query_str.length() + 1];
        strcpy(query, query_str.c_str());

        PGresult* res = PQexec(db_conn_, query);
        delete[] query;
        ExecStatusType resStatus = PQresultStatus(res);

        if (resStatus == PGRES_TUPLES_OK)
        {
            int rows = PQntuples(res);
            if (!rows)
            {
                query_str = "INSERT INTO public.users_log_pus(login, password) VALUES('" + login + "','" + password +"');";
                query = new char[query_str.length() + 1];
                strcpy(query, query_str.c_str());
                PQexec(db_conn_, query);
                delete[] query;

                std::cout << "User: " << login << " is Trying To Registered\n";
                result = LogIn(login, password);

                query_str = "INSERT INTO public.user_balance(user_id)	VALUES (" + result + ") ;";
                query = new char[query_str.length() + 1];
                strcpy(query, query_str.c_str());
                PQexec(db_conn_, query);
                delete[] query;
            }
        }
        PQclear(res);//mb need to clear in if

        return result;
    }

    // Возвращает ID зарегестрированного пользователя.
    std::string LogIn(const std::string& login, const std::string& password)
    {
        std::string result = "0";
        std::string query_str = "SELECT id FROM public.users_log_pus WHERE login = '" + login + "' AND password = '" + password + "'; ";

        char* query = new char[query_str.length() + 1];
        strcpy(query, query_str.c_str());

        PGresult* res = PQexec(db_conn_, query);
        delete[] query;
        ExecStatusType resStatus = PQresultStatus(res);

        if (resStatus == PGRES_TUPLES_OK)
        {
            int rows = PQntuples(res);
            if (rows)
            {
                std::string id{ PQgetvalue(res, 0, 0) };
                result = id;
                std::cout << "User: " << login << " is LogIn\n";
            }
        }
        PQclear(res);

        return result;
    }

    // Запрос имени клиента по ID
    std::string GetUserbalance(const std::string& aUserId)
    {
        std::string result;
        std::string query_str = "SELECT balance FROM public.user_balance WHERE user_id = " + aUserId + ";";
        char* query = new char[query_str.length() + 1];
        strcpy(query, query_str.c_str());

        PGresult* res = PQexec(db_conn_, query);
        delete[] query;
        ExecStatusType resStatus = PQresultStatus(res);

        if (resStatus == PGRES_TUPLES_OK)
        {
            int rows = PQntuples(res);
            if (rows)
            {
                std::string balance{ PQgetvalue(res, 0, 0) };
                result = balance;
                std::cout << "UserId: " << aUserId << " asked Balance\n";
            }
        }
        PQclear(res);

        return result;
    }

    std::string AddRequestSale(const std::string& aUserId, const std::string& count, const std::string& price)
    {
        std::string result = "Failed to create a request";
        std::string query_str = "INSERT INTO public.request_purchase_sale(user_id, dollar_price, dollars_count, sale) VALUES(" + aUserId + ", " + price + " , " + count + ", true ); ";

        if (AddRequest(query_str))
            result = "Request has been created";

        return result;
    }

    std::string AddRequestPurchase(const std::string& aUserId, const std::string& count, const std::string& price)
    {
        std::string result = "Failed to create a request";
        std::string query_str = "INSERT INTO public.request_purchase_sale(user_id, dollar_price, dollars_count) VALUES(" + aUserId + ", " + price + " , " + count + " ); ";

        if (AddRequest(query_str))
            result = "Request has been created";

        return result;
    }

    bool AddRequest(const std::string& request)
    {
        bool result = false;
        char* query = new char[request.length() + 1];
        strcpy(query, request.c_str());
        PGresult* res = PQexec(db_conn_, query);
        delete[] query;

        ExecStatusType resStatus = PQresultStatus(res);

        if (resStatus == PGRES_COMMAND_OK)
            result = true;

        PQclear(res);
        return result;
    }

    //При покупке, выставляю максимальную цену, могу ПОКУПАЮ СНАЧАЛА ДЕШЕВЫЕ, если есть 2 позиции, выбираю ту, что раньше датой
    //При продаже, выставляю минимальную цену, могу ПРОДАЮ СНАЧАЛА ДОРОГИЕ, если есть 2 позиции, выбираю ту, что раньше датой

    
    void ExecuteRequests()
    {
        std::string result;
        std::string query_str = "SELECT"            //Беру в БД только заявки на закупки, сортирую по убыванию доллара и по возрастанию даты
                                "   id,"
                                "   user_id,"
                                "   dollar_price,"
                                "   dollars_count"
                                "FROM public.request_purchase_sale"
                                "WHERE"
                                "   sale = false"
                                "ORDER BY"
                                "   dollar_price DESC,"
                                "   request_date;";

        char* query = new char[query_str.length() + 1];
        strcpy(query, query_str.c_str());

        PGresult* res = PQexec(db_conn_, query);
        delete[] query;

        ExecStatusType resStatus = PQresultStatus(res);

        std::vector<ReqData> purchase_req;//отсортированы в нужном порядке

        if (resStatus == PGRES_TUPLES_OK)
        {
            int rows = PQntuples(res);
            for (int i = 0; i < rows; i++)
            {
                std::string user_id{ PQgetvalue(res, i, 1) };
                std::string price  { PQgetvalue(res, i, 2) };
                std::string count  { PQgetvalue(res, i, 3) };
                purchase_req.push_back(ReqData{ std::stoi(user_id), std::stoi(price), std::stod(count), PQgetvalue(res, i, 0) });
            }
        }
        PQclear(res);

        query_str = "SELECT"            //Беру в БД только заявки на продажу, сортирую по возрастанию доллара и по возрастанию даты
                    "   id,"
                    "   user_id,"
                    "   dollar_price,"
                    "   dollars_count"
                    "FROM public.request_purchase_sale"
                    "WHERE"
                    "   sale = true"
                    "ORDER BY"
                    "   dollar_price ASC,"
                    "   request_date;";

        query = new char[query_str.length() + 1];
        strcpy(query, query_str.c_str());

        res = PQexec(db_conn_, query);
        delete[] query;

        resStatus = PQresultStatus(res);

        std::vector<ReqData> sale_req;//отсортированы в нужном порядке

        if (resStatus == PGRES_TUPLES_OK)
        {
            int rows = PQntuples(res);
            for (int i = 0; i < rows; i++)
            {
                std::string user_id{ PQgetvalue(res, i, 1) };
                std::string price  { PQgetvalue(res, i, 2) };
                std::string count  { PQgetvalue(res, i, 3) };
                sale_req.push_back(ReqData{ std::stoi(user_id), std::stoi(price), std::stod(count), PQgetvalue(res, i, 0) });
            }
        }
        PQclear(res);


        std::vector<DealData> deals;//для подсчета баланса и заполнения истории 
        std::vector<ReqData>::iterator pur_iter = purchase_req.begin();
        std::vector<ReqData>::iterator sale_iter = sale_req.begin();
        while (pur_iter != purchase_req.end() && sale_iter != sale_req.end())
        {
            if (pur_iter->price < sale_iter->price)
                break;

            if (pur_iter->user_id == sale_iter->user_id)//ОБОСРАЛО ВСЮ МАЛИНУ
            {

            }

            DealData deal;
            deal.buyer_id = pur_iter->user_id;
            deal.seller_id = sale_iter->user_id;
            deal.price = pur_iter->price;

            if (pur_iter->count == sale_iter->count)
            {
                deal.count = pur_iter->count;

                ++pur_iter;
                ++sale_iter;
            }
            else if (pur_iter->count > sale_iter->count)
            {
                deal.count = sale_iter->count;
                pur_iter->count -= sale_iter->count;

                ++sale_iter;
            }
            else
            {
                deal.count = pur_iter->count;
                sale_iter->count -= pur_iter->count;

                ++pur_iter;
            }
            deals.push_back(deal);
        }

        if (pur_iter != purchase_req.begin() || sale_iter != sale_req.begin())//Удаление пустых заявок
        {
            query_str = "DELETE FROM public.request_purchase_sale WHERE id = ";
            bool first = true;
            for (auto iter = purchase_req.begin(); iter != pur_iter; ++iter)
            {
                if (first)
                    query_str += iter->req_id;
                query_str += " OR id = " + iter->req_id;
                first = false;
            }
            for (auto iter = sale_req.begin(); iter != sale_iter; ++iter)
            {
                if (first)
                    query_str += iter->req_id;
                query_str += " OR id = " + iter->req_id;
                first = false;
            }

            query = new char[query_str.length() + 1];
            strcpy(query, query_str.c_str());
            res = PQexec(db_conn_, query);
            delete[] query;
            PQclear(res);
        }

        //Исправление заявки
        if (sale_iter != sale_req.end())
        {
            query_str = "UPDATE public.request_purchase_sale"
                "SET dollars_count = " + std::to_string(sale_iter->count) + " "
                "WHERE id = " + sale_iter->req_id + "; ";

            query = new char[query_str.length() + 1];
            strcpy(query, query_str.c_str());
            res = PQexec(db_conn_, query);
            delete[] query;
            PQclear(res);
        }
        if (pur_iter != purchase_req.end())
        {
            query_str = "UPDATE public.request_purchase_sale"
                "SET dollars_count = " + std::to_string(pur_iter->count) + " "
                "WHERE id = " + pur_iter->req_id + "; ";

            query = new char[query_str.length() + 1];
            strcpy(query, query_str.c_str());
            res = PQexec(db_conn_, query);
            delete[] query;
            PQclear(res);
        }
                
        for (auto iter = deals.begin(); iter != deals.end(); ++iter)
        {

        }

        //boost::asio::async_write(socket_,
        //    boost::asio::buffer(reply_, reply_.size()),
        //    boost::bind(&session::handle_write, this,
        //        boost::asio::placeholders::error));
    }

private:
    PGconn* db_conn_;
};

Core& GetCore()
{
    static Core core;
    return core;
}

class session
{
public:
    session(boost::asio::io_service& io_service)
        : socket_(io_service)
    {
    }

    tcp::socket& socket()
    {
        return socket_;
    }

    void start()
    {
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            boost::bind(&session::handle_read, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

    // Обработка полученного сообщения.
    void handle_read(const boost::system::error_code& error,
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
                reply_ = GetCore().RegisterNewUser(j["Login"], j["Password"]);
            else if (reqType == Requests::LogIn)
                reply_ = GetCore().LogIn(j["Login"], j["Password"]);
            else if (reqType == Requests::Balance)
                reply_ = GetCore().GetUserbalance(j["UserId"]);
            else if (reqType == Requests::AddRequestSale)
                reply_ = GetCore().AddRequestSale(j["UserId"], j["Count"], j["Price"]);
            else if (reqType == Requests::AddRequestPurchase)
                reply_ = GetCore().AddRequestPurchase(j["UserId"], j["Count"], j["Price"]);

            boost::asio::async_write(socket_,
                boost::asio::buffer(reply_, reply_.size()),
                boost::bind(&session::handle_write, this,
                    boost::asio::placeholders::error));
        }
        else
        {
            delete this;
        }
    }

    void handle_write(const boost::system::error_code& error)
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

private:
    std::string reply_;
    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
};

class server
{
public:
    server(boost::asio::io_service& io_service)
        : io_service_(io_service),
        acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
    {
        std::cout << "Server started! Listen " << port << " port" << std::endl;

        session* new_session = new session(io_service_);
        acceptor_.async_accept(new_session->socket(),
            boost::bind(&server::handle_accept, this, new_session,
                boost::asio::placeholders::error));
    }

    void handle_accept(session* new_session,
        const boost::system::error_code& error)
    {
        if (!error)
        {
            new_session->start();
            new_session = new session(io_service_);
            acceptor_.async_accept(new_session->socket(),
                boost::bind(&server::handle_accept, this, new_session,
                    boost::asio::placeholders::error));
        }
        else
        {
            delete new_session;
        }
    }

private:
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
};

int main()
{
    std::system("chcp 1251");

    try
    {
        boost::asio::io_service io_service;
        static Core core;

        server s(io_service);

        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}