cmake_minimum_required(VERSION 3.10)
project(ChatRoom)

# Tìm các gói cần thiết
find_package(Threads REQUIRED)

# Server: Phải link với sqlite3 và pthread
add_executable(server src/server/server.c src/server/database.c)
target_link_libraries(server PRIVATE Threads::Threads sqlite3)

# Client: Phải link với raylib, math (m), dl, và pthread
set(CLIENT_SRC_DIR src/client)
set(CLIENT_SOURCES
    ${CLIENT_SRC_DIR}/main.c
    ${CLIENT_SRC_DIR}/views/screenManager/screenManager.c
    ${CLIENT_SRC_DIR}/utils/states/states.c
    ${CLIENT_SRC_DIR}/utils/constants/constants.c
    ${CLIENT_SRC_DIR}/utils/utils/utils.c
    ${CLIENT_SRC_DIR}/utils/colors/colors.c
    ${CLIENT_SRC_DIR}/utils/debugger/debugger.c
    ${CLIENT_SRC_DIR}/views/screens/loginScreen/loginScreen.c
    ${CLIENT_SRC_DIR}/views/screens/registerScreen/registerScreen.c
    ${CLIENT_SRC_DIR}/views/components/components.c
    ${CLIENT_SRC_DIR}/controllers/loginController/loginController.c
)

add_executable(client ${CLIENT_SOURCES})
target_link_libraries(client PRIVATE raylib GL m Threads::Threads dl rt X11)
