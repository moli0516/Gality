cmake_minimum_required(VERSION 3.20)
project(gality CXX)

# C++17 標準
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 編譯器特定設定
if (MSVC)
    add_compile_options(/utf-8)
elseif (APPLE)
    # macOS Clang 設定
    add_compile_options(-Wall -Wextra)
    # 若需要 UTF-8 支援
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

# 尋找依賴
find_package(SFML 3 COMPONENTS Graphics Audio Window System REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# 收集所有原始碼
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 建立執行檔
add_executable(${PROJECT_NAME} ${SOURCES})

# 標頭檔搜尋路徑
target_include_directories(${PROJECT_NAME} PRIVATE src)

# 連結函式庫
target_link_libraries(${PROJECT_NAME} PRIVATE
    SFML::Graphics
    SFML::Audio
    SFML::Window
    SFML::System
    nlohmann_json::nlohmann_json
)

# macOS 特定：建立 .app bundle（可選）
if (APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE FALSE
        MACOSX_RPATH TRUE
    )
endif()

# 複製 assets 到執行檔目錄（開發用）
if (APPLE)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    )
endif()