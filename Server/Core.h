#pragma once

#include <libpq-fe.h>
#include <vector>
#include <map>
#include <shared_mutex>

#include "Common.hpp"

namespace  {
    struct ReqData
    {
        int user_id;
        int count;
        double price;
        std::string req_id;
    };
}

class Core
{
public:
    Core(const char* conninfo);
    ~Core();

public:
    std::pair<std::string, std::string> GetUserbalance(const std::string& aUserId);
    std::vector<std::string> GetActiveRequests();
    std::string LastAddedRequest(std::string user_id);
    std::vector<std::string>  GetCompletedDeals(const std::string& aUserId);
    std::string GetUSDQuotes();
    std::string GetNameById(std::string user_id);

public://mutable
    std::string RegisterNewUser(const std::string& login, const std::string& password);
    std::string LogIn(const std::string& login, const std::string& password);
    std::string AddRequestSale(const std::string& aUserId, const int count, const double price);
    std::string AddRequestPurchase(const std::string& aUserId, const int count, const double price);
    ChangesData ExecuteRequests();
    std::string CancelRequest(const std::string& aUserId, const std::string req_id);
    void LogOut(const std::string& aUserId);

private:
    bool AddRequest(const std::string& request);
    void ExecuteDBQuery(const std::string& query_str);
    PGresult* ExecuteDBQueryResponse(const std::string& query_str);
    std::vector<ReqData> DBExecuteRequests(bool for_sale);
    void DeleteCompletedRequests(const std::vector<std::string>& delete_req);
    void UpdateRequests(const std::map<std::string, int>& req_id_count);
    std::map<int, BalanceChanges> UpdateTransactionHistory(const std::vector<DealData>& deals);
    std::map<int, BalanceChanges> UpdateBalance(const std::vector<DealData>& deals);

private:
    PGconn* db_conn_;
    std::shared_mutex db_mutex_;
};
