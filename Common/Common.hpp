#pragma once

#include <string>
#include <vector>
#include <map>

static short port = 5555;

namespace Requests
{
    static std::string AddRequestPurchase    = "AddRequestForPurchase";
    static std::string AddRequestSale        = "AddRequestForSale";
    static std::string Balance               = "Balance";
    static std::string LogIn                 = "LogIn";
    static std::string Registration          = "Reg";
    static std::string SFeedBackReg          = "SFeedBackReg";
    static std::string ActiveRequests        = "ActiveRequests";
    static std::string CompletedTransactions = "CompletedTransactions";
    static std::string USDQuotes             = "USDQuotes";
    static std::string LogOut                = "LogOut";
    static std::string CancelReq             = "CancelReq";
    static std::string RecivedRowsCount      = "RecivedRowsCount";
    static std::string UpdateActiveReq       = "UpdateActiveReq";
    static std::string DeleteActiveReq       = "DeleteActiveReq";
    static std::string InsertActiveReq       = "InsertActiveReq";
    static std::string UdpadeBalance         = "UdpadeBalance";
    static std::string UpdateUsdQuote        = "UpdateUsdQuote";
    static std::string InsertCompletedDeals  = "InsertCompletedDeals";
}

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

struct ChangesData
{
    std::vector<DealData> deals;//Совершенные сделки
    std::vector<std::string> delete_req;//Закрытые запросы
    std::map<std::string, int> req_id_count;//запросы на обновление
    std::map<int, BalanceChanges> user_id_income;
};
