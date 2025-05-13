#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>
#include <GLFW/glfw3.h>

struct OHLC {
    double open;
    double high;
    double low;
    double close;
    double time;
};

int main() {
    // Seed random number generator
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Trading Simulator", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "OpenGL Version: " << version << std::endl;

    glfwSwapInterval(1); // Enable vsync

    // Initialize ImGui and ImPlot
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Trading simulator state
    float stock_price = 100.0f;
    float cash_balance = 10000.0f;
    int shares_owned = 0;
    std::vector<std::string> transaction_log;
    std::vector<OHLC> price_history;
    double current_time = 0.0;
    float open_price = stock_price;
    float high_price = stock_price;
    float low_price = stock_price;
    int tick_count = 0;
    const int ticks_per_candle = 60; // Number of ticks per candlestick

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Update stock price
        stock_price += (std::rand() % 100 - 50) / 100.0f;
        if (stock_price < 10.0f) stock_price = 10.0f;

        // Update high and low for the current candlestick
        high_price = std::max(high_price, stock_price);
        low_price = std::min(low_price, stock_price);

        // Increment tick count and time
        tick_count++;
        current_time += 1.0;

        // Create a new candlestick every ticks_per_candle ticks
        if (tick_count >= ticks_per_candle) {
            OHLC candle;
            candle.open = open_price;
            candle.high = high_price;
            candle.low = low_price;
            candle.close = stock_price;
            candle.time = current_time / ticks_per_candle;
            price_history.push_back(candle);

            // Reset for next candlestick
            open_price = stock_price;
            high_price = stock_price;
            low_price = stock_price;
            tick_count = 0;

            // Limit history to 100 candles to prevent memory issues
            if (price_history.size() > 100) {
                price_history.erase(price_history.begin());
            }
        }

        // Trading Simulator Window
        ImGui::Begin("Trading Simulator", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Stock Price: $%.2f", stock_price);
        ImGui::Separator();

        ImGui::Text("Trade");
        if (ImGui::Button("Buy Share")) {
            if (cash_balance >= stock_price) {
                cash_balance -= stock_price;
                shares_owned++;
                transaction_log.push_back("Bought 1 share at $" + std::to_string(stock_price));
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Sell Share")) {
            if (shares_owned > 0) {
                cash_balance += stock_price;
                shares_owned--;
                transaction_log.push_back("Sold 1 share at $" + std::to_string(stock_price));
            }
        }

        ImGui::Separator();
        ImGui::Text("Portfolio");
        ImGui::Text("Cash Balance: $%.2f", cash_balance);
        ImGui::Text("Shares Owned: %d", shares_owned);
        ImGui::Text("Portfolio Value: $%.2f", cash_balance + shares_owned * stock_price);

        ImGui::Separator();
        ImGui::Text("Transaction Log");
        ImGui::BeginChild("Log", ImVec2(0, 100), true);
        for (const auto& log : transaction_log) {
            ImGui::Text("%s", log.c_str());
        }
        ImGui::EndChild();

        ImGui::End();

        // Candlestick Chart Window
        ImGui::Begin("Stock Price Chart", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        if (ImPlot::BeginPlot("Candlestick Chart", ImVec2(600, 400))) {
            ImPlot::SetupAxes("Time", "Price");
            double x_max = (current_time / ticks_per_candle) + 1; // Extend x-axis to show current candle
            double x_min = x_max - 50; // Show last 50 candles
            ImPlot::SetupAxisLimits(ImAxis_X1, x_min, x_max, ImGuiCond_Always);

            // Dynamic y-axis limits based on price history
            double y_min = 99999, y_max = -99999;
            for (const auto& candle : price_history) {
                y_min = std::min(y_min, candle.low);
                y_max = std::max(y_max, candle.high);
            }
            // Include current candle's high/low
            y_min = std::min(y_min, static_cast<double>(low_price));
            y_max = std::max(y_max, static_cast<double>(high_price));
            // Add padding to y-axis
            double y_range = y_max - y_min;
            y_min -= y_range * 0.1;
            y_max += y_range * 0.1;
            ImPlot::SetupAxisLimits(ImAxis_Y1, y_min, y_max, ImGuiCond_Always);

            // Plot completed candlesticks
            if (!price_history.empty()) {
                // Prepare arrays for plotting
                std::vector<double> times(price_history.size());
                std::vector<double> opens(price_history.size());
                std::vector<double> closes(price_history.size());
                std::vector<double> highs(price_history.size());
                std::vector<double> lows(price_history.size());

                for (size_t i = 0; i < price_history.size(); ++i) {
                    times[i] = price_history[i].time;
                    opens[i] = price_history[i].open;
                    closes[i] = price_history[i].close;
                    highs[i] = price_history[i].high;
                    lows[i] = price_history[i].low;
                }

                // Plot candlestick bodies (open to close) using PlotBars
                for (size_t i = 0; i < price_history.size(); ++i) {
                    double bottom = std::min(opens[i], closes[i]);
                    double height = std::abs(closes[i] - opens[i]);
                    double x = times[i];
                    ImVec4 color = closes[i] >= opens[i] ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1); // Green for up, red for down

                    // Push color for this candle
                    ImPlot::PushStyleColor(ImPlotCol_Fill, color);
                    ImPlot::PushStyleColor(ImPlotCol_Line, color);

                    // Plot the body as a bar with increased width (from 0.4 to 0.6)
                    ImPlot::PlotBars(("CandleBody" + std::to_string(i)).c_str(), &x, &height, 1, 0.6, ImPlotBarsFlags_Horizontal, 0, sizeof(double));
                    ImPlot::SetNextMarkerStyle(ImPlotMarker_Square, 0, color);
                    ImPlot::PlotScatter(("CandleBottom" + std::to_string(i)).c_str(), &x, &bottom, 1);

                    // Pop colors
                    ImPlot::PopStyleColor(2);
                }

                // Plot wicks (high to low) using PlotLines with increased thickness
                for (size_t i = 0; i < price_history.size(); ++i) {
                    double x[2] = {times[i], times[i]};
                    double y[2] = {lows[i], highs[i]};
                    ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 2.0f); // Increase wick thickness
                    ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(1, 1, 1, 1)); // White wicks
                    ImPlot::PlotLine(("CandleWick" + std::to_string(i)).c_str(), x, y, 2);
                    ImPlot::PopStyleColor();
                    ImPlot::PopStyleVar();
                }
            }

            // Plot the current, incomplete candlestick
            if (tick_count > 0) {
                double current_candle_time = current_time / ticks_per_candle;
                double bottom = std::min(static_cast<double>(open_price), static_cast<double>(stock_price));
                double height = std::abs(static_cast<double>(stock_price) - static_cast<double>(open_price));
                double x = current_candle_time;
                ImVec4 color = stock_price >= open_price ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1);

                // Plot current candlestick body
                ImPlot::PushStyleColor(ImPlotCol_Fill, color);
                ImPlot::PushStyleColor(ImPlotCol_Line, color);
                ImPlot::PlotBars("CurrentCandleBody", &x, &height, 1, 0.6, ImPlotBarsFlags_Horizontal, 0, sizeof(double));
                ImPlot::SetNextMarkerStyle(ImPlotMarker_Square, 0, color);
                ImPlot::PlotScatter("CurrentCandleBottom", &x, &bottom, 1);
                ImPlot::PopStyleColor(2);

                // Plot current candlestick wick
                double x_wick[2] = {current_candle_time, current_candle_time};
                double y_wick[2] = {static_cast<double>(low_price), static_cast<double>(high_price)};
                ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 2.0f);
                ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(1, 1, 1, 1));
                ImPlot::PlotLine("CurrentCandleWick", x_wick, y_wick, 2);
                ImPlot::PopStyleColor();
                ImPlot::PopStyleVar();
            }

            ImPlot::EndPlot();
        }
        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}