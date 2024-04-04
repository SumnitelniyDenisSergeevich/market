CMAKE_MINIMUM_REQUIRED(VERSION 3.16)
PROJECT(ClienServerEcn)

FIND_PACKAGE(Boost 1.40 COMPONENTS system REQUIRED)
FIND_PACKAGE(Boost 1.40 COMPONENTS thread REQUIRED)

FIND_PACKAGE(Threads REQUIRED)
FIND_PACKAGE(PostgreSQL REQUIRED)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${PostgreSQL_INCLUDE_DIRS})

SET(CMAKE_CXX_STANDARD 17)

ADD_EXECUTABLE(Server Server/ServerMain.cpp Server/Core.h Server/Core.cpp Server/Session.h Server/Session.cpp Server/Server.cpp Server/Server.h Common/Common.hpp Common/json.hpp)
TARGET_LINK_LIBRARIES(Server PRIVATE Threads::Threads ${Boost_LIBRARIES} PRIVATE ${PostgreSQL_LIBRARIES})
target_include_directories(Server PRIVATE Server/ Common/)

ADD_EXECUTABLE(Client Client/Client.cpp Common/Common.hpp Common/json.hpp)
TARGET_LINK_LIBRARIES(Client PRIVATE Threads::Threads ${Boost_LIBRARIES})
target_include_directories(Client PRIVATE Client/ Common/)
