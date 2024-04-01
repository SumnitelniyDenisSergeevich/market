#pragma once

#include <string>

static short port = 5555;

namespace Requests
{
    static std::string AddRequestPurchase = "AddRequestForPurchase";
    static std::string AddRequestSale     = "AddRequestForSale";
    static std::string Balance            = "Balance";
    static std::string LogIn              = "LogIn";
    static std::string Registration       = "Reg";
}
