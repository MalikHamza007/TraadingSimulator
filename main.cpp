#include <iostream>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set OpenGL version to 3.2 core profile for macOS compatibility
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

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330"); // Fallback to GLSL 120 for older Macs

    // Trading simulator state
    float stock_price = 100.0f;
    float cash_balance = 10000.0f;
    int shares_owned = 0;
    std::vector<std::string> transaction_log;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        stock_price += (std::rand() % 100 - 50) / 100.0f;
        if (stock_price < 10.0f) stock_price = 10.0f;

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
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}