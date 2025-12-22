cmake_minimum_required(VERSION 3.10)
project(Yahuu)

# Tìm các gói cần thiết
find_package(Threads REQUIRED)

set(SERVER_SRC_DIR src/server)
set(SERVER_SOURCES
    ${SERVER_SRC_DIR}/main.c
    ${SERVER_SRC_DIR}/services/peachdb/peachdb.c
    ${SERVER_SRC_DIR}/services/peachdb/functions/files/files.c
    ${SERVER_SRC_DIR}/services/peachdb/functions/debugger/debugger.c
    ${SERVER_SRC_DIR}/models/usermodel/userModel.c
    ${SERVER_SRC_DIR}/services/userService/userService.c
    ${SERVER_SRC_DIR}/services/socketService/socketService.c
    ${SERVER_SRC_DIR}/services/sessionManager/sessionManager.c
    ${SERVER_SRC_DIR}/services/messageService/messageService.c
    ${SERVER_SRC_DIR}/services/groupService/groupService.c
)

# Server
add_executable(server ${SERVER_SOURCES})
target_link_libraries(server PRIVATE Threads::Threads sqlite3)

# --- CLIENT ---
set(CLIENT_SRC_DIR src/client)
set(CLIENT_SOURCES
    ${CLIENT_SRC_DIR}/main.c
    ${CLIENT_SRC_DIR}/views/screenManager/screenManager.c
    ${CLIENT_SRC_DIR}/utils/states/states.c
    ${CLIENT_SRC_DIR}/utils/constants/constants.c
    ${CLIENT_SRC_DIR}/utils/utils/utils.c
    ${CLIENT_SRC_DIR}/utils/fonts/fonts.c
    ${CLIENT_SRC_DIR}/utils/colors/colors.c
    ${CLIENT_SRC_DIR}/utils/debugger/debugger.c
    ${CLIENT_SRC_DIR}/views/screens/loginScreen/loginScreen.c
    ${CLIENT_SRC_DIR}/views/screens/registerScreen/registerScreen.c
    ${CLIENT_SRC_DIR}/views/screens/chatScreen/chatScreen.c
    ${CLIENT_SRC_DIR}/views/components/components.c
    ${CLIENT_SRC_DIR}/controllers/loginController/loginController.c
    ${CLIENT_SRC_DIR}/controllers/registerController/registerController.c
    ${CLIENT_SRC_DIR}/services/networkService/networkService.c
    ${CLIENT_SRC_DIR}/services/authService/authService.c
    ${CLIENT_SRC_DIR}/services/messageService/messageService.c
    ${CLIENT_SRC_DIR}/services/groupService/groupService.c
)

# 1. Tạo executable TRƯỚC
add_executable(client ${CLIENT_SOURCES})

# 2. Sau đó mới thêm include directory
# Lưu ý: Sửa cú pháp biến thành ${CLIENT_SRC_DIR} (bỏ dấu $ thừa)
target_include_directories(client PRIVATE "${CLIENT_SRC_DIR}/views/raygui")

# 3. Link thư viện
target_link_libraries(client PRIVATE raylib GL m Threads::Threads dl rt X11)
