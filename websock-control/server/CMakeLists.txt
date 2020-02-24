cmake_minimum_required(VERSION 3.4)

set(project_name websock-sbus-server) ## rename your project here

project(${project_name})

set(CMAKE_CXX_STANDARD 11)

## add executables
add_executable(${project_name}-exe
        src/AppComponent.hpp
        src/controller/MyController.hpp
        src/websocket/WSListener.cpp
        src/websocket/WSListener.hpp
        src/App.cpp
        src/sbus/sbus_service.cpp
        src/sbus/sbus_service.hpp
        src/sbus/sbus_coroutine.hpp
        src/sbus/sbus_serial.cpp
        src/sbus/sbus_serial.hpp
)

## link libs
## PiSBUS must be built separately.
find_package(oatpp              0.19.11 REQUIRED)
find_package(oatpp-websocket    0.19.11 REQUIRED)
target_link_libraries(${project_name}-exe
        PUBLIC oatpp::oatpp
        PUBLIC oatpp::oatpp-websocket
)

set_target_properties(${project_name}-exe PROPERTIES
        CXX_STANDARD 11
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
        LINKER_LANGUAGE CXX
)
