#include "gfx/Gui.hpp"

bool Gui::g_VSync = true;
bool Gui::g_Wireframe = false;
float Gui::g_Gamma = 2.2f;

void Gui::ImGuiInit(GLFWwindow* window)
{
    g_VSync = true;
    g_Wireframe = false;
    g_Gamma = 2.2f;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // opcjonalnie: ładny styl
    ImGui::StyleColorsDark();

    // Init backendów
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // VSync on
    glfwSwapInterval(1);
}

void Gui::ImGuiFrame(GLFWwindow* window)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ShowFrameInfo();
    ShowControlsInfo();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Gui::ImGuiShutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Gui::ShowFrameInfo()
{
    const float DISTANCE = 10.0f; // margines od rogu
    ImVec2 window_pos = ImVec2(DISTANCE, DISTANCE);
    ImVec2 window_pos_pivot = ImVec2(0.0f, 0.0f); // lewy górny róg
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    ImGui::SetNextWindowBgAlpha(0.35f); // przezroczyste tło

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoFocusOnAppearing
        | ImGuiWindowFlags_NoNav;

    ImGui::Begin("InfoOverlay", nullptr, flags);

    // dane do wyświetlenia
    ImGuiIO& io = ImGui::GetIO();
    float fps = io.Framerate;
    float ms  = (fps > 0.0f) ? (1000.0f / fps) : 0.0f;

    ImGui::Text("FPS: %.1f", fps);
    ImGui::Text("Frame: %.3f ms", ms);

    // przykłady własnych zmiennych:
    // ImGui::Text("isGrounded: %s", IsGrounded ? "true" : "false");
    // ImGui::Text("Velocity: (%.2f, %.2f, %.2f)", vel.x, vel.y, vel.z);

    ImGui::End();
}

void Gui::ShowControlsInfo()
{
    const float DISTANCE = 10.0f; // margines od rogu
    ImVec2 window_pos = ImVec2(ImGui::GetIO().DisplaySize.x - DISTANCE, DISTANCE);
    ImVec2 window_pos_pivot = ImVec2(1.0f, 0.0f); // prawy górny róg
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    ImGui::SetNextWindowBgAlpha(0.35f); // półprzezroczyste tło

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoFocusOnAppearing
        | ImGuiWindowFlags_NoNav;

    ImGui::Begin("ControlsOverlay", nullptr, flags);

    ImGui::Text("Controls:");
    ImGui::Separator();
    ImGui::Text("WSAD - Movement");
    ImGui::Text("Space - Up");
    ImGui::Text("Shift - Down");
    ImGui::Text("--------------");
    ImGui::Text("Postprocessing");
    ImGui::Text("Key 1 - Default");
    ImGui::Text("Key 2 - Blur");
    ImGui::Text("Key 3 - Highlight edges");
    ImGui::Text("Key 4 - Retro");
    ImGui::Text("Key 5 - Negative");

    ImGui::End();
}