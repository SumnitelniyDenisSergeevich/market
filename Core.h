#pragma once

#include <libpq-fe.h>
#include <string>
#include <vector>
#include <map>
#include <shared_mutex>

#include "Common.hpp"
#include "json.hpp"

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

public:
    std::pair<std::string, std::string> GetUserbalance(const std::string& aUserId);
    std::vector<std::string> GetActiveRequests();//-
    std::vector<std::string> GetActiveUserRequests(const std::string& aUserId);//-
    std::vector<std::string>  GetCompletedDeals(const std::string& aUserId);//-
    std::string GetUSDQuotes();//-

public:
    std::string RegisterNewUser(const std::string& login, const std::string& password);
    std::string LogIn(const std::string& login, const std::string& password);
    std::string AddRequestSale(const std::string& aUserId, const std::string& count, const std::string& price);
    std::string AddRequestPurchase(const std::string& aUserId, const std::string& count, const std::string& price);    
    std::vector<DealData> ExecuteRequests();

    std::string CancelRequest(const std::string& aUserId, const std::string req_id);//-
    void LogOut(const std::string& aUserId);//-

private:
    bool AddRequest(const std::string& request);
    void ExecuteDBQuery(const std::string& query_str);
    PGresult* ExecuteDBQueryResponse(const std::string& query_str);
    std::vector<ReqData> DBExecuteRequests(bool for_sale);
    void DeleteCompletedRequests(const std::vector<std::string>& delete_req);
    void UpdateRequests(const std::map<std::string, int>& req_id_count);
    std::map<int, BalanceChanges> UpdateTransactionHistory(const std::vector<DealData>& deals);
    void UpdateBalance(const std::vector<DealData>& deals);

private:
    PGconn* db_conn_;
    std::shared_mutex db_mutex_;
};
