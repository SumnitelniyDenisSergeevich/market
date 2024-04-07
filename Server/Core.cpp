#include <iostream>
#include <map>

#include "Core.h"
#include "json.hpp"

Core::Core(const char *conninfo)
{
    db_conn_ = PQconnectdb(conninfo);

    if (PQstatus(db_conn_) != CONNECTION_OK)
        throw PQerrorMessage(db_conn_);
    else
    {
        std::cout << "db stock_market connect\n";

        std::string query_str = "UPDATE public.users_log_pus SET online=false";

        db_mutex_.lock();
        ExecuteDBQuery(query_str);
        db_mutex_.unlock();
    }
}

Core::~Core()
{
    PQfinish(db_conn_);
}

std::string Core::RegisterNewUser(const std::string& login, const std::string& password)
{
    std::string result = "0";
    std::string query_str = "SELECT id FROM public.users_log_pus WHERE login = '" + login + "';";

    db_mutex_.lock_shared();
    PGresult* res = ExecuteDBQueryResponse(query_str);
    db_mutex_.unlock_shared();
    ExecStatusType resStatus = PQresultStatus(res);

    if (resStatus == PGRES_TUPLES_OK)
    {
        int rows = PQntuples(res);
        if (!rows)
        {
            std::cout << "User " << login << " registered" << std::endl;
            query_str = "INSERT INTO public.users_log_pus(login, password) VALUES('" + login + "','" + password +"');";
            db_mutex_.lock();
            ExecuteDBQuery(query_str);
            db_mutex_.unlock();

            result = LogIn(login, password);

            query_str = "INSERT INTO public.user_balance(user_id)	VALUES (" + result + ") ;";
            db_mutex_.lock();
            ExecuteDBQuery(query_str);
            db_mutex_.unlock();
        }
    }
    PQclear(res);

    return result;
}

std::string Core::LogIn(const std::string& login, const std::string& password)
{
    std::string result = "0";
    std::string query_str = "SELECT id, online FROM public.users_log_pus WHERE login = '" + login + "' AND password = '" + password + "' AND online = false; ";
    db_mutex_.lock_shared();
    PGresult* res = ExecuteDBQueryResponse(query_str);
    db_mutex_.unlock_shared();
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

    if (result != "0")
    {
        query_str = "UPDATE public.users_log_pus "
                            "SET online=true "
                            "WHERE id = " + result + ";";
        db_mutex_.lock();
        ExecuteDBQuery(query_str);
        db_mutex_.unlock();
    }

    return result;
}

std::pair<std::string, std::string> Core::GetUserbalance(const std::string& aUserId)
{
    std::pair<std::string, std::string> result;
    std::string query_str = "SELECT balance, usd_count FROM public.user_balance WHERE user_id = " + aUserId + ";";
    db_mutex_.lock_shared();
    PGresult* res = ExecuteDBQueryResponse(query_str);
    db_mutex_.unlock_shared();
    ExecStatusType resStatus = PQresultStatus(res);

    if (resStatus == PGRES_TUPLES_OK)
    {
        int rows = PQntuples(res);
        if (rows)
        {
            std::string balance{ PQgetvalue(res, 0, 0) };
            std::string usd_count{ PQgetvalue(res, 0, 1) };
            result = std::make_pair(balance, usd_count);            
        }
    }
    PQclear(res);

    return result;
}

std::string Core::AddRequestSale(const std::string& aUserId, const std::string& count, const std::string& price)
{
    std::string result = "Failed to create a request";
    std::string query_str = "INSERT INTO public.request_purchase_sale(user_id, dollar_price, dollars_count, sale) VALUES(" + aUserId + ", " + price + " , " + count + ", true ); ";

    if (AddRequest(query_str))
        result = "Request has been created";

    return result;
}

std::string Core::AddRequestPurchase(const std::string& aUserId, const std::string& count, const std::string& price)
{
    std::string result = "Failed to create a request";
    std::string query_str = "INSERT INTO public.request_purchase_sale(user_id, dollar_price, dollars_count) VALUES(" + aUserId + ", " + price + " , " + count + " ); ";
    
    if (AddRequest(query_str))
        result = "Request has been created";
    
    return result;
}

bool Core::AddRequest(const std::string& request)
{
    bool result = false;
    db_mutex_.lock();
    PGresult* res = ExecuteDBQueryResponse(request);    
    db_mutex_.unlock();
    char* inserted_rows_count = PQcmdTuples(res);// память отчистится в PQclear
    ExecStatusType resStatus = PQresultStatus(res);

    if (resStatus == PGRES_COMMAND_OK && std::stoi(inserted_rows_count) == 1)
        result = true;

    PQclear(res);
    return result;
}

ChangesData Core::ExecuteRequests()
{
    std::vector<ReqData> purchase_req = DBExecuteRequests(false);
    std::vector<ReqData> sale_req = DBExecuteRequests(true);

    //Выполнение запросов
    ChangesData data;

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

                data.delete_req.push_back(pur_iter->req_id);
                data.delete_req.push_back(sale_iter->req_id);
            }
            else if (pur_iter->count > sale_iter->count)
            {
                deal.count = sale_iter->count;
                pur_iter->count -= sale_iter->count;
                sale_iter->count = 0;
                data.delete_req.push_back(sale_iter->req_id);
                data.req_id_count[pur_iter->req_id] = pur_iter->count;
            }
            else
            {
                deal.count = pur_iter->count;
                sale_iter->count -= pur_iter->count;
                pur_iter->count = 0;
                data.delete_req.push_back(pur_iter->req_id);
                data.req_id_count[sale_iter->req_id] = sale_iter->count;
            }
            data.deals.push_back(deal);
        }

    for (const std::string& id : data.delete_req)// Удаляю лишние заявки, которые не надо будет обновлять
    {
        auto iter = data.req_id_count.find(id);
        if (iter != data.req_id_count.end())
            data.req_id_count.erase(iter);
    }

    DeleteCompletedRequests(data.delete_req);
    UpdateRequests(data.req_id_count);
    data.user_id_income = UpdateBalance(data.deals);

    return data;//Для отправки писем пользователям, о совершении сделки
}

void Core::ExecuteDBQuery(const std::string& query_str)
{
    char* query = new char[query_str.length() + 1];
    strcpy(query, query_str.c_str());
    PGresult* res = PQexec(db_conn_, query);
    delete[] query;
    PQclear(res);
}

PGresult* Core::ExecuteDBQueryResponse(const std::string& query_str)
{
    char* query = new char[query_str.length() + 1];
    strcpy(query, query_str.c_str());
    PGresult* res = PQexec(db_conn_, query);
    delete[] query;
    return res;
}

std::vector<ReqData> Core::DBExecuteRequests(bool for_sale)
{
    std::string query_str = "SELECT "
                            "   id, "
                            "   user_id, "
                            "   dollar_price, "
                            "   dollars_count "
                            "FROM public.request_purchase_sale "
                            "WHERE "
                            "   sale = ";
    query_str += (for_sale ? "true" : "false" );
    query_str += " "
                 "ORDER BY "
                 "   dollar_price DESC, "
                 "   request_date;";

    db_mutex_.lock_shared();
    PGresult* res = ExecuteDBQueryResponse(query_str);
    db_mutex_.unlock_shared();
    ExecStatusType resStatus = PQresultStatus(res);

    std::vector<ReqData> request;

    if (resStatus == PGRES_TUPLES_OK)
    {
        int rows = PQntuples(res);
        for (int i = 0; i < rows; i++)
        {
            std::string user_id{ PQgetvalue(res, i, 1) };
            std::string price  { PQgetvalue(res, i, 2) };
            std::string count  { PQgetvalue(res, i, 3) };
            request.push_back(ReqData{ std::stoi(user_id), std::stoi(count), std::stod(price) , PQgetvalue(res, i, 0) });
        }
    }
    PQclear(res);

    return request;
}

void Core::DeleteCompletedRequests(const std::vector<std::string>& delete_req)
{
    if (delete_req.size())
    {
        std::string query_str = "DELETE FROM public.request_purchase_sale WHERE id = ";
        bool first = true;

        for (const std::string& id : delete_req)
        {
            if (first)
                query_str += id;
            else
                query_str += " OR id = " + id;
            first = false;
        }
        db_mutex_.lock();
        ExecuteDBQuery(query_str);
        db_mutex_.unlock();
    }
}

void Core::UpdateRequests(const std::map<std::string, int>& req_id_count)
{
    for (const auto [id, count] : req_id_count)
    {
        std::string query_str = "UPDATE public.request_purchase_sale "
            "SET dollars_count = " + std::to_string(count) + " "
            "WHERE id = " + id + "; ";
        db_mutex_.lock();
        ExecuteDBQuery(query_str);
        db_mutex_.unlock();
    }
}

std::map<int, BalanceChanges> Core::UpdateTransactionHistory(const std::vector<DealData>& deals)
{
    std::map<int, BalanceChanges> user_id_income; //Создаю дату, что бы посылать на 1 юзера не более одного запроса

    if (!deals.empty())
    {
        std::string query_str = "INSERT INTO public.transaction_history(buyer_id, seller_id, dollar_price, dollars_count) VALUES";
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
        db_mutex_.lock();
        ExecuteDBQuery(query_str);
        db_mutex_.unlock();
    }
    return user_id_income;
}

std::map<int, BalanceChanges> Core::UpdateBalance(const std::vector<DealData>& deals)
{
    std::map<int, BalanceChanges> user_id_income = UpdateTransactionHistory(deals);
    for (const auto [id, balance_changes] : user_id_income)//Изменение баланса пользователей
    {
        std::string user_id = std::to_string(id);
        auto balance_count = GetUserbalance(user_id);

        std::string query_str = "UPDATE public.user_balance "
            "SET usd_count = " + std::to_string(stoi(balance_count.second) + balance_changes.count) + ", "
            "    balance = " + std::to_string(stod(balance_count.first) + balance_changes.income) + " "
            "WHERE user_id = " + user_id + "; ";
        db_mutex_.lock();
        ExecuteDBQuery(query_str);
        db_mutex_.unlock();
    }
    return user_id_income;
}

std::vector<std::string> Core::GetActiveRequests()
{
    std::string query_str = "SELECT "
                            "    REQ.id, "
                            "    USR.login, "
                            "    USR.id, "
                            "    REQ.dollar_price, "
                            "    REQ.dollars_count, "
                            "    CASE WHEN REQ.sale = true THEN 'sells' "
                            "         ELSE 'buys' "
                            "    END "
                            "FROM public.request_purchase_sale AS REQ "
                            "    JOIN public.users_log_pus AS USR ON REQ.user_id = USR.id "
                            "ORDER BY "
                            "    REQ.request_date;";
    db_mutex_.lock_shared();
    PGresult* res = ExecuteDBQueryResponse(query_str);
    db_mutex_.unlock_shared();
    ExecStatusType resStatus = PQresultStatus(res);
    std::vector<std::string> requests;

    if (resStatus == PGRES_TUPLES_OK)
    {
        int rows = PQntuples(res);
        for (int i = 0; i < rows; i++)
        {
            nlohmann::json req;
            req["req_id"]     = std::string{ PQgetvalue(res, i, 0) };
            req["user_login"] = std::string{ PQgetvalue(res, i, 1) };
            req["user_id"]    = std::string{ PQgetvalue(res, i, 2) };
            req["d_count"]    = std::string{ PQgetvalue(res, i, 4) };
            req["d_price"]    = std::string{ PQgetvalue(res, i, 3) };
            req["side"]       = std::string{ PQgetvalue(res, i, 5) };

            requests.push_back(req.dump());
        }
    }
    PQclear(res);

    return requests;
}

std::string Core::LastAddedRequest()
{
    std::string query_str = "SELECT "
                            "    REQ.id, "
                            "    USR.login, "
                            "    USR.id, "
                            "    REQ.dollar_price, "
                            "    REQ.dollars_count, "
                            "    CASE WHEN REQ.sale = true THEN 'sells' "
                            "         ELSE 'buys' "
                            "    END "
                            "FROM public.request_purchase_sale AS REQ "
                            "    JOIN public.users_log_pus AS USR ON REQ.user_id = USR.id "
                            "ORDER BY "
                            "    REQ.request_date "
                            "LIMIT 1;";
    db_mutex_.lock_shared();
    PGresult* res = ExecuteDBQueryResponse(query_str);
    db_mutex_.unlock_shared();
    ExecStatusType resStatus = PQresultStatus(res);
    std::string result;

    if (resStatus == PGRES_TUPLES_OK)
    {
        int rows = PQntuples(res);
        if (rows == 1)
        {
            nlohmann::json req;
            req["req_id"]     = std::string{ PQgetvalue(res, 0, 0) };
            req["user_login"] = std::string{ PQgetvalue(res, 0, 1) };
            req["user_id"]    = std::string{ PQgetvalue(res, 0, 2) };
            req["d_count"]    = std::string{ PQgetvalue(res, 0, 4) };
            req["d_price"]    = std::string{ PQgetvalue(res, 0, 3) };
            req["side"]       = std::string{ PQgetvalue(res, 0, 5) };

            result = req.dump();
        }

    }
    PQclear(res);

    return result;
}

std::vector<std::string> Core::GetCompletedDeals(const std::string& aUserId)
{
    std::string query_str = "SELECT "
                            "    BUYER.login, "
                            "    SELLER.login, "
                            "    HIST.dollar_price, "
                            "    HIST.dollars_count "
                            "FROM public.transaction_history AS HIST "
                            "    JOIN public.users_log_pus AS BUYER "
                            "        ON HIST.buyer_id = BUYER.id "
                            "    JOIN public.users_log_pus AS SELLER "
                            "        ON HIST.seller_id = SELLER.id "
                            "WHERE "
                            "    HIST.buyer_id = " + aUserId + " OR HIST.seller_id = " + aUserId + "; ";

    db_mutex_.lock_shared();
    PGresult* res = ExecuteDBQueryResponse(query_str);
    db_mutex_.unlock_shared();
    ExecStatusType resStatus = PQresultStatus(res);
    std::vector<std::string> requests;

    if (resStatus == PGRES_TUPLES_OK)
    {
        int rows = PQntuples(res);
        for (int i = 0; i < rows; i++)
        {
            nlohmann::json req;
            req["Buyer"]     = std::string{ PQgetvalue(res, i, 0) };
            req["Seller"]    = std::string{ PQgetvalue(res, i, 1) };
            req["d_price"]    = std::string{ PQgetvalue(res, i, 2) };
            req["d_count"]    = std::string{ PQgetvalue(res, i, 3) };

            requests.push_back(req.dump());
        }
    }
    PQclear(res);

    return requests;
}

std::string Core::GetUSDQuotes()
{
    std::string query_str = "SELECT AVG(dollar_price) " //Среднее значение цены доллара для покупки( за сколько готовы купить в среднем) переработать
                            "    FROM public.request_purchase_sale "
                            "WHERE "
                            "    sale = false;";

    db_mutex_.lock_shared();
    PGresult* res = ExecuteDBQueryResponse(query_str);
    db_mutex_.unlock_shared();
    ExecStatusType resStatus = PQresultStatus(res);

    std::string result;
    if (resStatus == PGRES_TUPLES_OK)
    {
        int rows = PQntuples(res);
        if (rows)
            result = PQgetvalue(res, 0, 0);
    }

    if (result.empty())
        result = "0";

    PQclear(res);
    return result;
}

std::string Core::GetNameById(std::string user_id)
{
    std::string query_str = "SELECT login "
                            "    FROM public.users_log_pus "
                            "WHERE "
                            "    id = " + user_id + ";";

    db_mutex_.lock();
    PGresult* res = ExecuteDBQueryResponse(query_str);
    db_mutex_.unlock();
    ExecStatusType resStatus = PQresultStatus(res);

    std::string result;
    if (resStatus == PGRES_TUPLES_OK)
        result = PQgetvalue(res, 0, 0);

    PQclear(res);
    return result;
}

std::string Core::CancelRequest(const std::string& aUserId, const std::string req_id)
{
    std::string query_str = "DELETE FROM public.request_purchase_sale "
                            "WHERE id = (SELECT id FROM public.request_purchase_sale "
                            "               WHERE id = " + req_id + "  AND user_id = " + aUserId + " );";

    db_mutex_.lock();
    PGresult* res = ExecuteDBQueryResponse(query_str);    
    db_mutex_.unlock();
    char* deleted_rows_count = PQcmdTuples(res);// память отчистится в PQclear
    ExecStatusType resStatus = PQresultStatus(res);

    std::string result;

    if (resStatus == PGRES_COMMAND_OK && std::stoi(deleted_rows_count) == 1)
    {
        std::cout << "UserId: " << aUserId << " canceled request" << req_id << std::endl;
        result = "Request canceled";
    }
    else
    {
        std::cout << "UserId: " << aUserId << " not canceled request" << req_id << std::endl;
        result = "Request not canceled";
    }

    PQclear(res);

    return result;
}

void Core::LogOut(const std::string& aUserId)//В Qt Gui можно поставить на закрытие приложения
{
    std::string query_str = "UPDATE public.users_log_pus "
                            "SET online=false "
                            "WHERE id = " + aUserId + ";";
    db_mutex_.lock();
    ExecuteDBQuery(query_str);
    db_mutex_.unlock();
}
