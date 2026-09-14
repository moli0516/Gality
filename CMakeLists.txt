cmake_minimum_required(VERSION 3.20)
project(gality CXX)

if (MSVC)
    add_compile_options(/utf-8)
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3 COMPONENTS Graphics Audio Window System REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# 💡 關鍵修復：將 src 加入標頭檔搜尋路徑，徹底根絕 C1083 找不到檔案錯誤
target_include_directories(${PROJECT_NAME} PRIVATE src)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SFML::Graphics
    SFML::Audio
    SFML::Window
    SFML::System
    nlohmann_json::nlohmann_json
)

# Keep the build output lean for archive-first testing.
# The game mounts data.pak during startup instead of requiring the full assets directory copy.
# If you want the raw assets folder for debugging, uncomment the copy step below.
# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     "${CMAKE_CURRENT_SOURCE_DIR}/assets"
#     "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
# )