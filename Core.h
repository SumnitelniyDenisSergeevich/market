#pragma once

#include <libpq-fe.h>
#include <string>
#include <vector>
#include <map>

#include "Common.hpp"

namespace  {
    struct ReqData
    {
        int user_id;
        int count;
        double price;
        std::string req_id;
    };

    struct BalanceChanges
    {
        double income;
        int count;
    };
}

class Core
{
public:
    Core();
    ~Core();

//    "4) View active requests.\n"
//    "5) Cancel request.\n"
//    "6) My completed deals.\n"
//    "7) View the history of USD quotes.\n"
//    "8) Exit\n"

public://возможно только чтение из БД
    std::pair<std::string, std::string> GetUserbalance(const std::string& aUserId);

    void GetActiveRequests();//-
    void GetActiveUserRequests(const std::string& aUserId);//-
    void GetCompletedDeals(const std::string& aUserId);//-
    void GetUSDQuotes();//-

public://возможно изменение БД
    std::string RegisterNewUser(const std::string& login, const std::string& password);
    std::string LogIn(const std::string& login, const std::string& password);
    std::string AddRequestSale(const std::string& aUserId, const std::string& count, const std::string& price);
    std::string AddRequestPurchase(const std::string& aUserId, const std::string& count, const std::string& price);
    bool AddRequest(const std::string& request);
    std::vector<DealData> ExecuteRequests();

    void CancelRequest(const std::string& aUserId, const std::string req_id);//-
    void LogOut(const std::string& aUserId);//-

private://DB
    void ExecuteDBQuery(const std::string& query_str);
    PGresult* ExecuteDBQueryResponse(const std::string& query_str);
    std::vector<ReqData> DBExecuteRequests(bool for_sale);
    void DeleteCompletedRequests(const std::vector<std::string>& delete_req);
    void UpdateRequests(const std::map<std::string, int>& req_id_count);
    std::map<int, BalanceChanges> UpdateTransactionHistory(const std::vector<DealData>& deals);
    void UpdateBalance(const std::vector<DealData>& deals);

private:
    PGconn* db_conn_;
};
