// Author: Sreepathy Jayanand
#include "imgui.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include<bits/stdc++.h>
#define ASSERT(x) if(!(x)) __builtin_trap();
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))
namespace impl {
static void GLClearError() {
    while(glGetError() != GL_NO_ERROR);
}
static bool GLLogCall(const char* function, const char* file, int line) {
    while (GLenum error = glGetError()) {
        std::cout << "[OpenGL Error] (" << error << "):" << function << " " << file << ":" << line << std::endl;
        return false;
    }
    return true;
}

static void ParseShader(const std::string& filepath, std::string& vertexShader, std::string& fragmentShader) {
    std::ifstream stream(filepath);
    std::string line;
    std::stringstream ss[2];
    enum class ShaderType {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };
    ShaderType type = ShaderType::NONE;
    while (std::getline(stream, line)) {
        if (line.find("#shader") != std::string::npos) {
            if (line.find("vertex") != std::string::npos)
                type = ShaderType::VERTEX;  
            else if (line.find("fragment") != std::string::npos)
                type = ShaderType::FRAGMENT;  
    }
        else
            ss[(int)type] << line << "\n";
        
    }
    vertexShader = ss[0].str();
    fragmentShader = ss[1].str();
}

static unsigned int CompileShader(unsigned int type, const std::string& source) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if ( result == GL_FALSE ) {
      int length;
      glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
      char* message = (char*)alloca(length * sizeof(char));
      glGetShaderInfoLog(id, length, &length, message);
      std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader: " << message << std::endl;
      glDeleteShader(id);
      return 0;
    }
    return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}
void Render() {
        // READ THIS !!!
    // TL;DR; this demo is more complicated than what most users you would normally use.
    // If we remove all options we are showcasing, this demo would become:
    //     void ShowExampleAppDockSpace()
    //     {
    //         ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    //     }
    // In most cases you should be able to just call DockSpaceOverViewport() and ignore all the code below!
    // In this specific demo, we are not using DockSpaceOverViewport() because:
    // - (1) we allow the host window to be floating/moveable instead of filling the viewport (when opt_fullscreen == false)
    // - (2) we allow the host window to have padding (when opt_padding == true)
    // - (3) we expose many flags and need a way to have them visible.
    // - (4) we have a local menu bar in the host window (vs. you could use BeginMainMenuBar() + DockSpaceOverViewport()
    //      in your code, but we don't here because we allow the window to be floating)

    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }
    else
    {
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", nullptr, window_flags);
    if (!opt_padding)
        ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // Submit the DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Options"))
        {
            // Disabling fullscreen would allow the window to be moved to the front of other windows,
            // which we can't undo at the moment without finer window depth/z control.
            ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
            ImGui::MenuItem("Padding", NULL, &opt_padding);
            ImGui::Separator();

            if (ImGui::MenuItem("Flag: NoDockingOverCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingOverCentralNode) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingOverCentralNode; }
            if (ImGui::MenuItem("Flag: NoDockingSplit",         "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingSplit) != 0))             { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingSplit; }
            if (ImGui::MenuItem("Flag: NoUndocking",            "", (dockspace_flags & ImGuiDockNodeFlags_NoUndocking) != 0))                { dockspace_flags ^= ImGuiDockNodeFlags_NoUndocking; }
            if (ImGui::MenuItem("Flag: NoResize",               "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0))                   { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
            if (ImGui::MenuItem("Flag: AutoHideTabBar",         "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0))             { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
            if (ImGui::MenuItem("Flag: PassthruCentralNode",    "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
            ImGui::Separator();

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

        ImGui::Begin("Window");
        ImGui::End();
        ImGui::Begin("Viewport");
        ImDrawList* draw_list = ImGui::GetWindowDrawList(); 
 ImVec2 p = ImGui::GetCursorScreenPos(); 
 draw_list->AddCircleFilled(ImVec2(p.x + 50, p.y + 50), 30.0f, IM_COL32(255, 0, 0, 255), 16); 
 draw_list->AddLine(ImVec2(p.x, p.y), ImVec2(p.x + 100.0f, p.y + 100.0f), IM_COL32(255, 255, 0, 255), 3.0f); 
 ImGui::Dummy(ImVec2(200, 200));



    if (!glfwInit())
        return ;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        // Problem: glewInit failed
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        return;
    }
    unsigned int VAO; glGenVertexArrays(1, &VAO); glBindVertexArray(VAO);
    /* fprintf(stdout, "GLEW version: %s\n", glewGetString(GLEW_VERSION)); */
    /* fprintf(stdout, "GL version: %s\n", glGetString(GL_VERSION)); */

    float positions[] = {
       -0.5f, -0.5f,
        0.5f, -0.5f,
        0.5f,  0.5f,
        -0.5f, 0.5f
    };
    unsigned int indices[] = {
        0, 1, 2, 
        2, 3, 0
    };

    unsigned int buffer;
    GLCall(glGenBuffers(1, &buffer));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer));
    GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(float)* 4 * 2, positions, GL_STATIC_DRAW));

    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), 0));
    unsigned int ibo;
    GLCall(glGenBuffers(1, &ibo));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6 , indices, GL_STATIC_DRAW));

    std::string vertexShader, fragmentShader;
    GLCall(ParseShader("res/shaders/basic.shader", vertexShader, fragmentShader));
    unsigned int shader = CreateShader(vertexShader, fragmentShader);
    GLCall(glUseProgram(shader));
    int location = glGetUniformLocation(shader, "u_Color");
    ASSERT(location != -1);
    GLCall(glUniform4f(location, 0.2f, 0.3f, 0.8f, 1.0f));
    float r = 0.0f, increment = 0.01f;
    GLCall(glClear(GL_COLOR_BUFFER_BIT));
    GLCall(glBindVertexArray(VAO));
    GLCall(glUniform4f(location, r, 0.3f, 0.8f, 1.0f));
    if (r < 0.0f || r > 1.0f)
        increment = -increment;
    r += increment;
    GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));


    ImGui::End();
    ImGui::End();

}
}
