//It is necessary to create a test database
//(creation script in the SQL folder of the test_base.sql file)

#include <gtest/gtest.h>
#include <libpq-fe.h>
#include <Core.h>
#include "json.hpp"

#include <iostream>

#define STRING(x) std::string{X}

Core core("dbname=test_stock_market user=market_admin password=1 host=localhost port=5432");

const char* conninfo = "dbname=test_stock_market user=market_admin password=1 host=localhost port=5432";
PGconn* db_conn = PQconnectdb(conninfo);// Необходимо для использования в тестах
std::string registered_user_id_1;
std::string registered_user_id_2;

PGresult* ExecuteDBQueryResponse(PGconn* db_conn, const std::string& query_str);

TEST(RegLogUser, RegLogRequest) {

    std::string user_id = core.RegisterNewUser("login", "password");
    registered_user_id_1 = user_id;

    EXPECT_GT(std::stoi(user_id), 0);

//user_login_pus
    std::string query = "SELECT id, login, password, online "
                        "FROM public.users_log_pus;";

    PGresult* res = ExecuteDBQueryResponse(db_conn, query);
    ExecStatusType resStatus = PQresultStatus(res);

    std::string user_id_str;

    if (resStatus == PGRES_TUPLES_OK)
    {
        int rows = PQntuples(res);
        EXPECT_EQ(rows, 1);

        std::string user_id_req{ PQgetvalue(res, 0, 0) };
        std::string login{ PQgetvalue(res, 0, 1) };
        std::string password{ PQgetvalue(res, 0, 2) };
        std::string online{ PQgetvalue(res, 0, 3) };

        EXPECT_EQ(user_id_req, user_id);
        EXPECT_EQ(login, "login");
        EXPECT_EQ(password, "password");
        EXPECT_EQ(online, "t");
    }
    PQclear(res);

//balance
    query = "SESELECT user_id, balance, usd_count "
            "FROM public.user_balance;";

    res = ExecuteDBQueryResponse(db_conn, query);
    resStatus = PQresultStatus(res);

    if (resStatus == PGRES_TUPLES_OK)
    {
        int rows = PQntuples(res);
        EXPECT_EQ(rows, 1);

        std::string user_id_req{ PQgetvalue(res, 0, 0) };
        std::string balance{ PQgetvalue(res, 0, 1) };
        std::string usd_count{ PQgetvalue(res, 0, 2) };

        EXPECT_EQ(user_id_req, user_id);
        EXPECT_EQ(balance, "0");
        EXPECT_EQ(usd_count, "0");
    }
    PQclear(res);
}

TEST(AddRequest, AddRequestSale) {
    std::string answer = core.AddRequestSale(registered_user_id_1, "5", "60");
    EXPECT_EQ(answer, "Request has been created");

    std::string query = "SELECT user_id, dollar_price, dollars_count, sale "
                        "FROM public.request_purchase_sale;";

    PGresult* res = ExecuteDBQueryResponse(db_conn, query);
    ExecStatusType resStatus = PQresultStatus(res);

    std::string user_id_str;

    if (resStatus == PGRES_TUPLES_OK)
    {
        int rows = PQntuples(res);
        EXPECT_EQ(rows, 1);

        std::string user_id_req{ PQgetvalue(res, 0, 0) };
        std::string dollar_price{ PQgetvalue(res, 0, 1) };
        std::string dollars_count{ PQgetvalue(res, 0, 2) };
        std::string sale{ PQgetvalue(res, 0, 3) };

        EXPECT_EQ(user_id_req, registered_user_id_1);
        EXPECT_DOUBLE_EQ(std::stod(dollar_price), 60.0);
        EXPECT_EQ(dollars_count, "5");
        EXPECT_EQ(sale, "t");
    }
    PQclear(res);
}

TEST(AddRequest, AddRequestPurchase) {

    std::string answer = core.AddRequestPurchase(registered_user_id_1, "5", "61");
    EXPECT_EQ(answer, "Request has been created");

    std::string query = "SELECT user_id, dollar_price, dollars_count, sale "
                        "FROM public.request_purchase_sale "
                        "WHERE sale = false;";

    PGresult* res = ExecuteDBQueryResponse(db_conn, query);
    ExecStatusType resStatus = PQresultStatus(res);

    std::string user_id_str;

    if (resStatus == PGRES_TUPLES_OK)
    {
        int rows = PQntuples(res);
        EXPECT_EQ(rows, 1);

        std::string user_id_req{ PQgetvalue(res, 0, 0) };
        std::string dollar_price{ PQgetvalue(res, 0, 1) };
        std::string dollars_count{ PQgetvalue(res, 0, 2) };
        std::string sale{ PQgetvalue(res, 0, 3) };

        EXPECT_EQ(user_id_req, registered_user_id_1);
        EXPECT_DOUBLE_EQ(std::stod(dollar_price), 61.0);
        EXPECT_EQ(dollars_count, "5");
        EXPECT_EQ(sale, "f");
    }
    PQclear(res);
}

TEST(GetUSDQuotes, GetUSDQuotes) {
    std::string answer = core.GetUSDQuotes();
    EXPECT_DOUBLE_EQ(std::stod(answer), 61.0);
}

TEST(Tables, GetActiveRequests) {
    std::vector<std::string> active_requests = core.GetActiveRequests();
    EXPECT_EQ(active_requests.size(), 2);

    auto req = nlohmann::json::parse(active_requests[0]);

    EXPECT_EQ(std::string{req["user_login"]}, "login");
    EXPECT_EQ(std::string{req["d_count"]   }, "5");
    EXPECT_EQ(std::string{req["side"]      }, "sells");
    EXPECT_DOUBLE_EQ(std::stod(std::string{req["d_price"]}) , 60.);


    req = nlohmann::json::parse(active_requests[1]);

    EXPECT_EQ(req["user_login"], "login");
    EXPECT_EQ(req["d_count"]   , "5");
    EXPECT_DOUBLE_EQ(std::stod(std::string{req["d_price"]}) , 61.);
    EXPECT_EQ(req["side"]      , "buys");
}

TEST(Tables, GetActiveUserRequests) {
    std::vector<std::string> active_requests = core.GetActiveUserRequests(registered_user_id_1);
    EXPECT_EQ(active_requests.size(), 2);

    auto req = nlohmann::json::parse(active_requests[0]);

    EXPECT_EQ(std::string{req["d_count"]   }, "5");
    EXPECT_EQ(std::string{req["side"]      }, "sell");
    EXPECT_DOUBLE_EQ(std::stod(std::string{req["d_price"]}) , 60.);


    req = nlohmann::json::parse(active_requests[1]);

    EXPECT_EQ(req["d_count"]   , "5");
    EXPECT_DOUBLE_EQ(std::stod(std::string{req["d_price"]}) , 61.);
    EXPECT_EQ(req["side"]      , "buy");
}

TEST(ExecuteRequests, ExecuteRequests) {
//second user
    std::string user_id = core.RegisterNewUser("login1", "password1");
    EXPECT_GT(std::stoi(user_id), 0);
    registered_user_id_2 = user_id;
//ADD PURCHASE
    std::string answer = core.AddRequestPurchase(registered_user_id_2, "5", "61");
    EXPECT_EQ(answer, "Request has been created");
//EXECUTE
    std::vector<DealData> deals = core.ExecuteRequests();
    EXPECT_EQ(deals.size(), 1);
    EXPECT_EQ(deals[0].seller_id, std::stoi(registered_user_id_1));
    EXPECT_EQ(deals[0].buyer_id, std::stoi(registered_user_id_2));
    EXPECT_EQ(deals[0].count, 5);
    EXPECT_DOUBLE_EQ(deals[0].price, 61.0);
//CHECK DB
    //requests
    std::string query = "SELECT user_id, dollar_price, dollars_count, sale "
                        "FROM public.request_purchase_sale";

    PGresult* res = ExecuteDBQueryResponse(db_conn, query);
    ExecStatusType resStatus = PQresultStatus(res);

    if (resStatus == PGRES_TUPLES_OK)
    {
        int rows = PQntuples(res);
        EXPECT_EQ(rows, 1);

        std::string user_id_req{ PQgetvalue(res, 0, 0) };
        std::string dollar_price{ PQgetvalue(res, 0, 1) };
        std::string dollars_count{ PQgetvalue(res, 0, 2) };
        std::string sale{ PQgetvalue(res, 0, 3) };

        EXPECT_EQ(user_id_req, registered_user_id_1);
        EXPECT_DOUBLE_EQ(std::stod(dollar_price), 61.0);
        EXPECT_EQ(dollars_count, "5");
        EXPECT_EQ(sale, "f");
    }
    PQclear(res);
    //history
    query = "SELECT buyer_id, seller_id, dollar_price, dollars_count "
            "FROM public.transaction_history;";

    res = ExecuteDBQueryResponse(db_conn, query);
    resStatus = PQresultStatus(res);

    if (resStatus == PGRES_TUPLES_OK)
    {
        int rows = PQntuples(res);
        EXPECT_EQ(rows, 1);

        std::string buyer_id{ PQgetvalue(res, 0, 0) };
        std::string seller_id{ PQgetvalue(res, 0, 1) };
        std::string dollar_price{ PQgetvalue(res, 0, 2) };
        std::string dollars_count{ PQgetvalue(res, 0, 3) };

        EXPECT_EQ(buyer_id, registered_user_id_2);
        EXPECT_EQ(seller_id, registered_user_id_1);
        EXPECT_DOUBLE_EQ(std::stod(dollar_price), 61.0);
        EXPECT_EQ(dollars_count, "5");
    }

    //balance
    query = "SELECT user_id, balance, usd_count "
            "FROM public.user_balance;";

    res = ExecuteDBQueryResponse(db_conn, query);
    resStatus = PQresultStatus(res);

    if (resStatus == PGRES_TUPLES_OK)
    {
        int rows = PQntuples(res);
        EXPECT_EQ(rows, 2);

        std::string user_id{ PQgetvalue(res, 0, 0) };
        std::string balance{ PQgetvalue(res, 0, 1) };
        std::string usd_count{ PQgetvalue(res, 0, 2) };

        EXPECT_EQ(user_id, registered_user_id_1);
        EXPECT_DOUBLE_EQ(std::stod(balance), 61.0 * 5);
        EXPECT_EQ(usd_count, "-5");

        std::string user_id_2{ PQgetvalue(res, 1, 0) };
        std::string balance_2{ PQgetvalue(res, 1, 1) };
        std::string usd_count_2{ PQgetvalue(res, 1, 2) };

        EXPECT_EQ(user_id_2, registered_user_id_2);
        EXPECT_DOUBLE_EQ(std::stod(balance_2), -61.0 * 5);
        EXPECT_EQ(usd_count_2, "5");
    }
    PQclear(res);
}

TEST(CancelRequest, NotCancelRequest) {
    std::string answer = core.CancelRequest(registered_user_id_2, "1");
    EXPECT_EQ(answer, "Request not canceled");
    //requests
    std::string query = "SELECT user_id, dollar_price, dollars_count, sale "
                        "FROM public.request_purchase_sale";

    PGresult* res = ExecuteDBQueryResponse(db_conn, query);
    ExecStatusType resStatus = PQresultStatus(res);

    if (resStatus == PGRES_TUPLES_OK)
    {
        int rows = PQntuples(res);
        EXPECT_EQ(rows, 1);

        std::string user_id_req{ PQgetvalue(res, 0, 0) };
        std::string dollar_price{ PQgetvalue(res, 0, 1) };
        std::string dollars_count{ PQgetvalue(res, 0, 2) };
        std::string sale{ PQgetvalue(res, 0, 3) };

        EXPECT_EQ(user_id_req, registered_user_id_1);
        EXPECT_DOUBLE_EQ(std::stod(dollar_price), 61.0);
        EXPECT_EQ(dollars_count, "5");
        EXPECT_EQ(sale, "f");
    }
    PQclear(res);
}

TEST(CancelRequest, CancelRequest) {
    //requests
    std::string query = "SELECT user_id, dollar_price, dollars_count, sale, id "
                        "FROM public.request_purchase_sale";

    PGresult* res = ExecuteDBQueryResponse(db_conn, query);
    ExecStatusType resStatus = PQresultStatus(res);

    std::string req_id;

    if (resStatus == PGRES_TUPLES_OK)
    {
        int rows = PQntuples(res);
        EXPECT_EQ(rows, 1);

        std::string user_id_req{ PQgetvalue(res, 0, 0) };
        std::string dollar_price{ PQgetvalue(res, 0, 1) };
        std::string dollars_count{ PQgetvalue(res, 0, 2) };
        std::string sale{ PQgetvalue(res, 0, 3) };
        req_id = std::string{PQgetvalue(res, 0, 4)};

        EXPECT_EQ(user_id_req, registered_user_id_1);
        EXPECT_DOUBLE_EQ(std::stod(dollar_price), 61.0);
        EXPECT_EQ(dollars_count, "5");
        EXPECT_EQ(sale, "f");
    }
    PQclear(res);

    std::string answer = core.CancelRequest(registered_user_id_1, req_id);
    EXPECT_EQ(answer, "Request canceled");
    //requests
    query = "SELECT user_id, dollar_price, dollars_count, sale "
                        "FROM public.request_purchase_sale";

    res = ExecuteDBQueryResponse(db_conn, query);
    resStatus = PQresultStatus(res);

    if (resStatus == PGRES_TUPLES_OK)
    {
        int rows = PQntuples(res);
        EXPECT_EQ(rows, 0);
    }
    PQclear(res);
}

TEST(LogOut, LogOut) {
    core.LogOut(registered_user_id_2);
//user_login_pus
    std::string query = "SELECT online "
                        "FROM public.users_log_pus "
                        "WHERE id = " + registered_user_id_2 + "; ";

    PGresult* res = ExecuteDBQueryResponse(db_conn, query);
    ExecStatusType resStatus = PQresultStatus(res);

    if (resStatus == PGRES_TUPLES_OK)
    {
        int rows = PQntuples(res);
        EXPECT_EQ(rows, 1);

        std::string online{ PQgetvalue(res, 0, 0) };

        EXPECT_EQ(online, "f");
    }
    PQclear(res);
}

TEST(TablesDeals, GetCompletedDeals) {
    std::vector<std::string> completed_deals = core.GetCompletedDeals(registered_user_id_1);
    EXPECT_EQ(completed_deals.size(), 1);

    auto req = nlohmann::json::parse(completed_deals[0]);

    EXPECT_EQ(std::string{req["Buyer"]   }, "login1");
    EXPECT_EQ(std::string{req["Seller"]  }, "login");
    EXPECT_EQ(std::string{req["d_count"] }, "5");
    EXPECT_DOUBLE_EQ(std::stod(std::string{req["d_price"]}) , 61.);
}

int main(int argc, char **argv) {

    if (PQstatus(db_conn) != CONNECTION_OK)
        throw PQerrorMessage(db_conn);
    else
    {
        std::cout << "db stock_market connect\n";
        std::string query_str = "TRUNCATE "
                                "  public.users_log_pus "
                                ", public.user_balance "
                                ", public.transaction_history "
                                ", public.request_purchase_sale;";
        PGresult* res = ExecuteDBQueryResponse(db_conn, query_str);
        ExecStatusType resStatus = PQresultStatus(res);

        if (resStatus == PGRES_COMMAND_OK)
            std::cout << "test base cleared\n";
        else
            std::cout << "test base NOT cleared\n";
        PQclear(res);
    }

    ::testing::InitGoogleTest( &argc, argv);
    int result = RUN_ALL_TESTS();

    PQfinish(db_conn);
    return result;
}

PGresult* ExecuteDBQueryResponse(PGconn* db_conn, const std::string& query_str)
{
    char* query = new char[query_str.length() + 1];
    strcpy(query, query_str.c_str());
    PGresult* res = PQexec(db_conn, query);
    delete[] query;
    return res;
}


