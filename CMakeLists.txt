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
add_executable(TradingSimulator main.cpp ${IMGUI_SOURCES} ${IMPLOT_SOURCES})

# Link libraries
target_link_libraries(TradingSimulator
        ${OPENGL_LIBRARIES}
        /usr/local/lib/libglfw.3.dylib
)

# macOS-specific frameworks
target_link_libraries(TradingSimulator
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
)