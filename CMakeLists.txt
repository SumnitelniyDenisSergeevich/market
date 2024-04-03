CMAKE_MINIMUM_REQUIRED(VERSION 3.16)
PROJECT(ClienServerEcn)

#For WIN
#set(BOOST_ROOT "e:/boost/boost_build_vc143")
#set(Boost_INCLUDE_DIR "e:/boost/boost_build_vc143/include/boost-1_84")
#set(Boost_LIBRARIES "e:/boost/boost_build_vc143/lib")

#set(PostgreSQL_INCLUDE_DIRS "e:/Postgre_sql/include")
#set(PostgreSQL_LIBRARIES "e:/Postgre_sql/lib/libpq.lib")

FIND_PACKAGE(Boost 1.40 COMPONENTS system REQUIRED)
FIND_PACKAGE(Boost 1.40 COMPONENTS thread REQUIRED)

FIND_PACKAGE(Threads REQUIRED)
FIND_PACKAGE(PostgreSQL REQUIRED)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${PostgreSQL_INCLUDE_DIRS})

SET(CMAKE_CXX_STANDARD 17)

ADD_EXECUTABLE(Server ServerMain.cpp Core.h Core.cpp Session.h Session.cpp Server.cpp Server.h Common.hpp json.hpp)
TARGET_LINK_LIBRARIES(Server PRIVATE Threads::Threads ${Boost_LIBRARIES} PRIVATE ${PostgreSQL_LIBRARIES})

ADD_EXECUTABLE(Client Client.cpp Common.hpp json.hpp)
TARGET_LINK_LIBRARIES(Client PRIVATE Threads::Threads ${Boost_LIBRARIES})
