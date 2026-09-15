cmake_minimum_required(VERSION 3.20)
project(gality CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================================
# 建置模式：Develop 或 Product
# ============================================================================
option(GALITY_DEV_BUILD "Build with debug tools enabled" ON)

if (GALITY_DEV_BUILD)
    message(STATUS "=== Building DEVELOP version (debug tools enabled) ===")
    add_compile_definitions(GALITY_DEV_BUILD)
else()
    message(STATUS "=== Building PRODUCT version (debug tools disabled) ===")
endif()

# ============================================================================
# 平台特定設定
# ============================================================================
if (MSVC)
    add_compile_options(/utf-8)

    if (GALITY_DEV_BUILD)
        add_compile_options(/W4)
    else()
        add_compile_options(/W3 /O2)
    endif()
elseif (APPLE)
    add_compile_options(-Wall -Wextra)
    if (GALITY_DEV_BUILD)
        add_compile_options(-O1 -g)
    else()
        add_compile_options(-O3)
    endif()
elseif (EMSCRIPTEN)
    add_compile_options(-Wall -Wextra -O3)
endif()

# ============================================================================
# 尋找依賴
# ============================================================================
find_package(SFML 3 COMPONENTS Graphics Audio Window System REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# ============================================================================
# 收集原始碼
# ============================================================================
file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SFML::Graphics
    SFML::Audio
    SFML::Window
    SFML::System
    nlohmann_json::nlohmann_json
)

# ============================================================================
# Emscripten 設定
# ============================================================================
if (EMSCRIPTEN)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        SUFFIX ".html"
        LINK_FLAGS "-s USE_SDL=2 \
                    -s USE_SDL_IMAGE=2 \
                    -s USE_SDL_MIXER=2 \
                    -s USE_FREETYPE=1 \
                    -s USE_LIBPNG=1 \
                    -s USE_OGG=1 \
                    -s USE_VORBIS=1 \
                    -s USE_FLAC=1 \
                    -s ALLOW_MEMORY_GROWTH=1 \
                    -s INITIAL_MEMORY=256MB \
                    -s MAXIMUM_MEMORY=2GB \
                    -s ASYNCIFY=1 \
                    -s ASSERTIONS=0 \
                    -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap','FS'] \
                    -s EXPORTED_FUNCTIONS=['_main'] \
                    --preload-file assets@/assets \
                    --preload-file data.pak@/data.pak \
                    -O3"
    )
endif()

# ============================================================================
# macOS 建置後複製 assets
# ============================================================================
if (APPLE AND NOT EMSCRIPTEN)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    )
endif()