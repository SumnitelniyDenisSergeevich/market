CMAKE_MINIMUM_REQUIRED(VERSION 3.16)
PROJECT(ClienServerEcn)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

FIND_PACKAGE(Boost 1.40 COMPONENTS system REQUIRED)
FIND_PACKAGE(Boost 1.40 COMPONENTS thread REQUIRED)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui)

FIND_PACKAGE(Threads REQUIRED)
FIND_PACKAGE(PostgreSQL REQUIRED)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${PostgreSQL_INCLUDE_DIRS})

SET(CMAKE_CXX_STANDARD 17)

ADD_EXECUTABLE(Server Server/ServerMain.cpp Server/Core.h Server/Core.cpp Server/Session.h Server/Session.cpp Server/Server.cpp Server/Server.h Common/Common.hpp Common/json.hpp)
TARGET_LINK_LIBRARIES(Server PRIVATE Threads::Threads ${Boost_LIBRARIES} PRIVATE ${PostgreSQL_LIBRARIES})
target_include_directories(Server PRIVATE Server/ Common/)

ADD_EXECUTABLE(Client Client/Client.cpp Client/ServerFeedBack.h Client/ServerFeedBack.cpp Common/Common.hpp Common/json.hpp
    Client/mainwindow.h Client/mainwindow.cpp Client/mainwindow.ui
    Client/LogIn.h Client/LogIn.cpp Client/LogIn.ui
    Client/RequestDialog.h Client/RequestDialog.cpp Client/RequestDialog.ui
    Client/RequestsFilterModel.h Client/RequestsFilterModel.cpp
    Client/Notify.h Client/Notify.cpp)
TARGET_LINK_LIBRARIES(Client PRIVATE Threads::Threads ${Boost_LIBRARIES} PRIVATE Qt5::Core PRIVATE Qt5::Gui PRIVATE Qt5::Widgets)
target_include_directories(Client PRIVATE Client/ Common/)
