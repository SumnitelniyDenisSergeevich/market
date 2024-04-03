#pragma once

#include <string>

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
    static std::string MyActiveRequests      = "MyActiveRequests";
    static std::string CompletedTransactions = "CompletedTransactions";
    static std::string USDQuotes             = "USDQuotes";
    static std::string LogOut                = "LogOut";
    static std::string CancelReq             = "CancelReq";
}

struct DealData
{
    int buyer_id;
    int seller_id;
    int count;
    double price;
};
