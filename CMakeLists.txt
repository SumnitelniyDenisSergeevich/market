CMAKE_MINIMUM_REQUIRED(VERSION 3.21)
PROJECT(ClienServerEcn)


set(BOOST_ROOT "e:/boost/boost_build_vc143")
set(Boost_INCLUDE_DIR "e:/boost/boost_build_vc143/include/boost-1_84")
set(Boost_LIBRARIES "e:/boost/boost_build_vc143/lib")

set(Postgre_INCLUDE_DIR "e:/Postgre_sql/include")#нужно подключить только libpq-fe.h
set(Postgre_LIBRARIES "e:/Postgre_sql/lib/libpq.lib")

FIND_PACKAGE(Boost 1.40 COMPONENTS system REQUIRED)
FIND_PACKAGE(Threads REQUIRED)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${Postgre_INCLUDE_DIR})

SET(CMAKE_CXX_STANDARD 17)

ADD_EXECUTABLE(Server Server.cpp Common.hpp json.hpp)
TARGET_LINK_LIBRARIES(Server PRIVATE Threads::Threads ${Boost_LIBRARIES} ${Postgre_LIBRARIES})

ADD_EXECUTABLE(Client Client.cpp Common.hpp json.hpp)
TARGET_LINK_LIBRARIES(Client PRIVATE Threads::Threads ${Boost_LIBRARIES})