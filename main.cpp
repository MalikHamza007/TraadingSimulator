#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>
#include <GLFW/glfw3.h>
#include <nlohmann/json.hpp>
#include "src/integration/api.h"
#include <cmath>
#include <ctime>
#include <cstdlib>

using namespace std;
using json = nlohmann::json;

struct OHLC {
    double open;
    double high;
    double low;
    double close;
    double time;
    string datetime; // Store datetime for deduplication
};

void DrawSpinner(const char* label, float radius, float thickness, const ImU32& color) {
    ImGui::PushID(label);
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float time = static_cast<float>(glfwGetTime());
    int segments = 12;
    ImGui::GetWindowDrawList()->AddCircle(pos, radius, color, segments, thickness);
    ImGui::Dummy(ImVec2(radius * 2, radius * 2));
    ImGui::PopID();
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        cerr << "Failed to initialize GLFW" << endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Trading Simulator", nullptr, nullptr);
    if (!window) {
        cerr << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;

    glfwSwapInterval(1); // Enable vsync

    // Initialize ImGui and ImPlot
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Trading simulator state
    float cash_balance = 10000.0f;
    int shares_owned = 0;
    vector<string> transaction_log;
    vector<OHLC> price_history;
    string selected_stock = "AAPL";
    vector<string> stocks = {"AAPL", "MSFT", "GOOGL", "AMZN", "TSLA"};
    bool fetch_data = true; // Trigger initial fetch
    bool is_loading = false;
    int api_call_count = 0;
    double last_fetch_time = glfwGetTime();
    const double fetch_interval = 60.0; // Fetch every 60 seconds (1 minute)
    string last_datetime;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Periodic data fetch
        double current_time = glfwGetTime();
        if (current_time - last_fetch_time >= fetch_interval) {
            fetch_data = true;
        }

        // Fetch stock data when needed
        if (fetch_data && !is_loading) {
            is_loading = true;
            string response = fetchStockData(selected_stock);
            try {
                json j = json::parse(response);
                if (j.contains("values") && j["values"].is_array()) {
                    vector<OHLC> new_candles;
                    double time = price_history.empty() ? 0.0 : price_history.back().time + 1.0;

                    // Process candles in reverse order (oldest to newest)
                    for (auto it = j["values"].rbegin(); it != j["values"].rend(); ++it) {
                        const auto& value = *it;
                        string datetime = value["datetime"].get<string>();
                        cout << "Processing candle with datetime: " << datetime << ", last_datetime: " << last_datetime << endl;

                        if (!last_datetime.empty() && datetime <= last_datetime) {
                            cout << "Skipping duplicate or older candle: " << datetime << endl;
                            continue; // Skip duplicates or older data
                        }

                        OHLC candle;
                        candle.open = stod(value["open"].get<string>());
                        candle.high = stod(value["high"].get<string>());
                        candle.low = stod(value["low"].get<string>());
                        candle.close = stod(value["close"].get<string>());
                        candle.time = time++;
                        candle.datetime = datetime;
                        new_candles.push_back(candle);
                        last_datetime = datetime;
                        cout << "Added candle with datetime: " << datetime << ", new_candles size: " << new_candles.size() << endl;
                    }

                    if (!new_candles.empty()) {
                        // Append new candles to price_history
                        price_history.insert(price_history.end(), new_candles.begin(), new_candles.end());
                        api_call_count++;
                    }

                    // Limit history to 100 candles
                    if (price_history.size() > 100) {
                        price_history.erase(price_history.begin(), price_history.begin() + (price_history.size() - 100));
                        cout << "Trimmed price_history to 100 candles, new size: " << price_history.size() << endl;
                    }
                } else {
                    cerr << "Invalid API response format" << endl;
                }
            } catch (const exception& e) {
                cerr << "JSON parse error: " << e.what() << endl;
            }
            is_loading = false;
            fetch_data = false;
            last_fetch_time = current_time;
        }

        // Trading Simulator Window
        ImGui::Begin("Trading Simulator", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        // Stock selector
        ImGui::Text("Select Stock:");
        for (const auto& stock : stocks) {
            if (ImGui::Button(stock.c_str()) && stock != selected_stock) {
                selected_stock = stock;
                fetch_data = true;
                last_datetime.clear();
                cout << "Switched to stock: " << selected_stock << endl;
            }
            ImGui::SameLine();
        }
        ImGui::NewLine();

        // Loading indicator
        if (is_loading) {
            ImGui::Text("Wait new Stock Loading...");
            DrawSpinner("Spinner", 10.0f, 2.0f, ImGui::GetColorU32(ImGuiCol_Button));

        }

        // API call count
        ImGui::Text("API Calls: %d", api_call_count);

        float stock_price = price_history.empty() ? 100.0f : price_history.back().close;
        ImGui::Text("Stock Price: $%.2f", stock_price);
        ImGui::Separator();

        ImGui::Text("Trade");
        if (ImGui::Button("Buy Share")) {
            if (cash_balance >= stock_price) {
                cash_balance -= stock_price;
                shares_owned++;
                transaction_log.push_back("Bought 1 share of " + selected_stock + " at $" + to_string(stock_price));
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Sell Share")) {
            if (shares_owned > 0) {
                cash_balance += stock_price;
                shares_owned--;
                transaction_log.push_back("Sold 1 share of " + selected_stock + " at $" + to_string(stock_price));
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
        // if (is_loading) {
        //     ImGui::Text("Loading chart...");
        //     DrawSpinner("ChartSpinner", 15.0f, 3.0f, ImGui::GetColorU32(ImGuiCol_Button));
        // } else if (ImPlot::BeginPlot("Candlestick Chart", ImVec2(600, 400))) {
        //     ImPlot::SetupAxes("Time", "Price");
            if (ImPlot::BeginPlot("Candle Stick Chart", ImVec2(600, 400))) {
    ImPlot::SetupAxes("Time", "Price");

    // Ensure at least 30 units of x-axis for visibility
    double x_max = price_history.empty() ? 30.0 : price_history.back().time + 1;
    double x_min = price_history.empty() ? 0.0 : max(0.0, x_max - 50);
    ImPlot::SetupAxisLimits(ImAxis_X1, x_min, x_max, ImGuiCond_Always);

    double y_min = 99999, y_max = -99999;
    for (const auto& candle : price_history) {
        y_min = min(y_min, candle.low);
        y_max = max(y_max, candle.high);
    }
    if (!price_history.empty()) {
        double y_range = y_max - y_min;
        y_min -= y_range * 0.1;
        y_max += y_range * 0.1;
        ImPlot::SetupAxisLimits(ImAxis_Y1, y_min, y_max, ImGuiCond_Always);

        vector<double> times(price_history.size());
        vector<double> opens(price_history.size());
        vector<double> closes(price_history.size());
        vector<double> highs(price_history.size());
        vector<double> lows(price_history.size());

        for (size_t i = 0; i < price_history.size(); ++i) {
            times[i] = price_history[i].time;
            opens[i] = price_history[i].open;
            closes[i] = price_history[i].close;
            highs[i] = price_history[i].high;
            lows[i] = price_history[i].low;
        }

        for (size_t i = 0; i < price_history.size(); ++i) {
            double bottom = min(opens[i], closes[i]);
            double height = abs(closes[i] - opens[i]);
            double x = times[i];
            ImVec4 color = closes[i] >= opens[i] ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1);

            ImPlot::PushStyleColor(ImPlotCol_Fill, color);
            ImPlot::PushStyleColor(ImPlotCol_Line, color);
            ImPlot::PlotBars(("CandleBody" + to_string(i)).c_str(), &x, &height, 1, 0.6, ImPlotBarsFlags_Horizontal, 0, sizeof(double));
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Square, 0, color);
            ImPlot::PlotScatter(("CandleBottom" + to_string(i)).c_str(), &x, &bottom, 1);
            ImPlot::PopStyleColor(2);

            double x_wick[2] = {times[i], times[i]};
            double y_wick[2] = {lows[i], highs[i]};
            ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 2.0f);
            ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(1, 1, 1, 1));
            ImPlot::PlotLine(("CandleWick" + to_string(i)).c_str(), x_wick, y_wick, 2);
            ImPlot::PopStyleColor();
            ImPlot::PopStyleVar();
        }
    } else {
        ImGui::Text("No data available. Market may be closed or data fetch failed.");
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