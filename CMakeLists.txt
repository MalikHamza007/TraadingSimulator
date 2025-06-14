cmake_minimum_required(VERSION 3.31)
project(TradingSimulator)

set(CMAKE_CXX_STANDARD 17)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Include GLFW from /usr/local/
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

# Include ImGui
set(IMGUI_PATH "${CMAKE_SOURCE_DIR}/libs/imgui")
include_directories(${IMGUI_PATH})
file(GLOB IMGUI_SOURCES
        ${IMGUI_PATH}/imgui.cpp
        ${IMGUI_PATH}/imgui_draw.cpp
        ${IMGUI_PATH}/imgui_widgets.cpp
        ${IMGUI_PATH}/imgui_tables.cpp
        ${IMGUI_PATH}/imgui_impl_glfw.cpp
        ${IMGUI_PATH}/imgui_impl_opengl3.cpp
        ${IMGUI_PATH}/imconfig.h
)

# === ImPlot Setup ===
set(IMPLOT_PATH "${CMAKE_SOURCE_DIR}/libs/implot")
include_directories(${IMPLOT_PATH})

file(GLOB IMPLOT_SOURCES
        ${IMPLOT_PATH}/implot.cpp
        ${IMPLOT_PATH}/implot_items.cpp
)

# Add executable
add_executable(TradingSimulator main.cpp ${IMGUI_SOURCES} ${IMPLOT_SOURCES}
        src/engine/trading_engine.cpp
        src/engine/trading_engine.h
        src/graph/graph_plotter.cpp
        src/graph/graph_plotter.h
        src/integration/api.cpp
        src/integration/api.h
        src/portfolio/portfolio.cpp
        src/portfolio/portfolio.h
        src/ui/ui_manager.cpp
        src/ui/ui+manager.h
        src/user/user_profile.cpp
        src/user/user_profile.h)

# Manually set curl paths (for Intel macOS with Homebrew)
set(CURL_INCLUDE_DIR "/usr/local/opt/curl/include")
set(CURL_LIBRARY "/usr/local/opt/curl/lib/libcurl.dylib")

include_directories(${CURL_INCLUDE_DIR})
# Include nlohmann/json
include_directories(${CMAKE_SOURCE_DIR}/libs/json/include)

# Link libraries
target_link_libraries(TradingSimulator
        ${OPENGL_LIBRARIES}
        /usr/local/lib/libglfw.3.dylib
        ${CURL_LIBRARY}

)

# macOS-specific frameworks
target_link_libraries(TradingSimulator
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
)