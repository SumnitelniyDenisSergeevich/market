#pragma once

#include <libpq-fe.h>
#include <string>
#include <vector>

#include "Common.hpp"

class Core
{
public:
    Core();
    ~Core();

    // "Регистрирует" нового пользователя и возвращает его ID.
    std::string RegisterNewUser(const std::string& login, const std::string& password);
    // Возвращает ID зарегестрированного пользователя.
    std::string LogIn(const std::string& login, const std::string& password);
    // Запрос имени клиента по ID
    std::pair<std::string, std::string> GetUserbalance(const std::string& aUserId);
    std::string AddRequestSale(const std::string& aUserId, const std::string& count, const std::string& price);
    std::string AddRequestPurchase(const std::string& aUserId, const std::string& count, const std::string& price);
    bool AddRequest(const std::string& request);
    std::vector<DealData> ExecuteRequests();

private:
    PGconn* db_conn_;
};
