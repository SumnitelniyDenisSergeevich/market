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

    struct BalanceChanges
    {
        double income;
        int count;
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
    std::pair<std::string, std::string> GetUserbalance(const std::string& aUserId)
    {
        std::pair<std::string, std::string> result;
        std::string query_str = "SELECT balance, usd_count FROM public.user_balance WHERE user_id = " + aUserId + ";";
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
                std::string usd_count{ PQgetvalue(res, 0, 1) };
                result = std::make_pair(balance, usd_count);
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

    
    std::vector<DealData> ExecuteRequests()
    {
        std::string result;
        std::string query_str = "SELECT "            //Беру в БД только заявки на закупки, сортирую по убыванию доллара и по возрастанию даты
                                "   id, "
                                "   user_id, "
                                "   dollar_price, "
                                "   dollars_count "
                                "FROM public.request_purchase_sale "
                                "WHERE "
                                "   sale = false "
                                "ORDER BY "
                                "   dollar_price DESC, "
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
                purchase_req.push_back(ReqData{ std::stoi(user_id), std::stoi(count), std::stod(price) , PQgetvalue(res, i, 0) });
            }
        }
        PQclear(res);

        query_str = "SELECT "            //Беру в БД только заявки на продажу, сортирую по возрастанию доллара и по возрастанию даты
                    "   id, "
                    "   user_id, "
                    "   dollar_price, "
                    "   dollars_count "
                    "FROM public.request_purchase_sale "
                    "WHERE "
                    "   sale = true "
                    "ORDER BY "
                    "   dollar_price ASC, "
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
                sale_req.push_back(ReqData{ std::stoi(user_id), std::stoi(count), std::stod(price) , PQgetvalue(res, i, 0) });
            }
        }
        PQclear(res);


        std::vector<DealData> deals;//для подсчета баланса и заполнения истории 
        std::vector<std::string> delete_req;
        std::map<std::string, int> req_id_count;

        for (auto pur_iter = purchase_req.begin(); pur_iter != purchase_req.end(); ++pur_iter)
            for (auto sale_iter = sale_req.begin(); sale_iter != sale_req.end(); ++sale_iter)
            {
                if (pur_iter->price < sale_iter->price || pur_iter->count == 0)
                    break;

                if (pur_iter->user_id == sale_iter->user_id || sale_iter->count == 0)
                    continue;

                DealData deal;
                deal.buyer_id = pur_iter->user_id;
                deal.seller_id = sale_iter->user_id;
                deal.price = pur_iter->price;

                if (pur_iter->count == sale_iter->count)
                {
                    deal.count = pur_iter->count;
                    pur_iter->count = 0;
                    sale_iter->count = 0;

                    delete_req.push_back(pur_iter->req_id);
                    delete_req.push_back(sale_iter->req_id);
                }
                else if (pur_iter->count > sale_iter->count)
                {
                    deal.count = sale_iter->count;
                    pur_iter->count -= sale_iter->count;
                    sale_iter->count = 0;
                    delete_req.push_back(sale_iter->req_id);
                    req_id_count[pur_iter->req_id] = pur_iter->count;
                }
                else
                {
                    deal.count = pur_iter->count;
                    sale_iter->count -= pur_iter->count;
                    pur_iter->count = 0;
                    delete_req.push_back(pur_iter->req_id);
                    req_id_count[sale_iter->req_id] = sale_iter->count;
                }
                deals.push_back(deal);
            }

        //Удаление заявок
        if (delete_req.size())
        {
            query_str = "DELETE FROM public.request_purchase_sale WHERE id = ";
            bool first = true;



            for (const std::string& id : delete_req)
            {
                auto iter = req_id_count.find(id);
                if (iter != req_id_count.end())// Удаляю лишние заявки, которые не надо будет обновлять
                    req_id_count.erase(iter);

                if (first)
                    query_str += id;
                else
                    query_str += " OR id = " + id;
                first = false;
            }

            query = new char[query_str.length() + 1];
            strcpy(query, query_str.c_str());
            res = PQexec(db_conn_, query);
            delete[] query;
            PQclear(res);
        }

        //Исправление заявки
        for (const auto [id, count] : req_id_count)
        {
            query_str = "UPDATE public.request_purchase_sale "
                "SET dollars_count = " + std::to_string(count) + " "
                "WHERE id = " + id + "; ";

            query = new char[query_str.length() + 1];
            strcpy(query, query_str.c_str());
            res = PQexec(db_conn_, query);
            delete[] query;
            PQclear(res);
        }
//============================= Изменение баланса пользователей =======================

        std::map<int, BalanceChanges> user_id_income; //Обрабатываю массив, что бы посылать на 1 юзера не более одного запроса

        if (!deals.empty())
        {
            query_str = "INSERT INTO public.transaction_history(buyer_id, seller_id, dollar_price, dollars_count) VALUES";
            bool first = true;
            for (const DealData& deal : deals) // впихнуть LOG
            {
                double income = deal.count * deal.price;
                user_id_income[deal.seller_id].income += income;
                user_id_income[deal.seller_id].count += -deal.count;
                user_id_income[deal.buyer_id].income += -income;
                user_id_income[deal.buyer_id].count += deal.count;

                if (!first)
                    query_str += ", ";

                query_str += " (" + std::to_string(deal.buyer_id) + ", " + std::to_string(deal.seller_id) + ", "
                    + std::to_string(deal.price) + ", " + std::to_string(deal.count) + ") ";

                first = false;
            }

            query = new char[query_str.length() + 1];
            strcpy(query, query_str.c_str());
            res = PQexec(db_conn_, query);
            delete[] query;
            PQclear(res);
        }

        for (const auto [id, balance_changes] : user_id_income)//Изменение баланса пользователей
        {
            std::string user_id = std::to_string(id);
            auto balance_count = GetUserbalance(user_id);

            query_str = "UPDATE public.user_balance "
                "SET usd_count = " + std::to_string(stoi(balance_count.second) + balance_changes.count) + ", "
                "    balance = " + std::to_string(stod(balance_count.first) + balance_changes.income) + " "
                "WHERE user_id = " + user_id + "; ";

            query = new char[query_str.length() + 1];
            strcpy(query, query_str.c_str());
            res = PQexec(db_conn_, query);
            delete[] query;
            PQclear(res);
        }

        return deals;//Для отправки писем пользователям, о совершении сделки
    }

private:
    PGconn* db_conn_;
};

Core& GetCore()
{
    static Core core;
    return core;
}

class server;

class session //можно сделать вложенным классом, тогда можно будет использовать методы server
{
public:
    session(boost::asio::io_service& io_service, server* serv)
        : user_id_(0)
        , socket_(io_service)
        , server_(serv)
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
            else if (reqType == Requests::AddRequestSale)
                reply_ = GetCore().AddRequestSale(j["UserId"], j["Count"], j["Price"]);
            else if (reqType == Requests::AddRequestPurchase)
                reply_ = GetCore().AddRequestPurchase(j["UserId"], j["Count"], j["Price"]);
            else if (reqType == Requests::Balance)
            {
                auto balance = GetCore().GetUserbalance(j["UserId"]);
                reply_ = balance.first + " RUB, " + balance.second + " USD";
            }

            if (reqType == Requests::Registration || reqType == Requests::LogIn)
                user_id_ = stoi(reply_);

            if (reqType == Requests::AddRequestSale || reqType == Requests::AddRequestPurchase)
            {
                std::vector<DealData> deals = GetCore().ExecuteRequests();

                for (const DealData& deal : deals)
                {
                    double amount = deal.count * deal.price;
                    purchase_reply_ = "You bought " + std::to_string(deal.count) + " USD for" + std::to_string(amount) + " RUB";
                    sale_reply_ = "You sold " + std::to_string(deal.count) + " USD for" + std::to_string(amount) + " RUB";

//                    boost::asio::async_write(socket_, //Надо посылать юзерам по Id, соответсвенно каждому нужен свой сокет
//                        boost::asio::buffer(reply_, reply_.size()),
//                        boost::bind(&session::handle_write, this,
//                            boost::asio::placeholders::error));
                }
            }


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
    std::string purchase_reply_;
    std::string sale_reply_;
    int user_id_;

    tcp::socket socket_; 
    server* server_;
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

        session* new_session = new session(io_service_, this);
        sessions_.push_back(new_session);

        acceptor_.async_accept(new_session->socket(),
            boost::bind(&server::handle_accept, this, new_session,
                boost::asio::placeholders::error));
    }

    ~server()
    {
        for (session* s : sessions_)
            delete s;
    }

    void handle_accept(session* new_session,
        const boost::system::error_code& error)
    {
        if (!error)
        {
            new_session->start();
            new_session = new session(io_service_, this);
            sessions_.push_back(new_session);

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
    std::vector<session*> sessions_;
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
};

int main()
{
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
