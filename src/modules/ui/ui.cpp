// ui.cpp

module;

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>

#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <string>

#include "spdlog/spdlog.h"
#include "restClient.hpp"
#include "wsClient.hpp"

module ui;

import imguiSink;

void renderUI(ID3D11DeviceContext* g_pd3dDeviceContext,
    ID3D11RenderTargetView* g_mainRenderTargetView,
    IDXGISwapChain* g_pSwapChain)
{

    // 1) Set our render target & clear it
    const float clearColor[4] = { 0.10f, 0.10f, 0.10f, 1.00f };
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clearColor);

    // Start ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Create DockSpace and UI elements
    // Create a fullscreen window (hidden) to host the DockSpace
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    // Begin a new ImGui window
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("DockSpace Demo", nullptr, window_flags);
    ImGui::PopStyleVar(2);

    // DockSpace
    ImGuiIO& io = ImGui::GetIO();
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    ImGui::End();

    // Menu Bar
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New"))
			{
				// action for new project
                auto logger = spdlog::get("IMGUI_LOGGER");
                if (logger)
                {
                    logger->info("New project initialized.");
                }
                std::string newFilePath = openFileSaveDialog("Create New Config File");
                
			}
            if (ImGui::MenuItem("Open"))
            {
				// open file dialogue
                std::string loadPath = openFileOpenDialog("Open Project File");
            }
            if (ImGui::MenuItem("Save"))
            {
                // save file dialogue   
                std::string savePath = openFileSaveDialog("Save New Project File");
            }
            if (ImGui::MenuItem("Save As"))
            {
                std::string savePath = openFileSaveDialog("Save Project File");
            }
			if (ImGui::MenuItem("Exit"))
			{
				PostQuitMessage(0);
			}

            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View"))
        {
            static int zoomLevel = 100; // Default zoom level (%)

            // Dropdown for selecting zoom level
            if (ImGui::BeginMenu("Zoom"))
            {
                if (ImGui::MenuItem("75%", NULL, zoomLevel == 75)) zoomLevel = 75;
                if (ImGui::MenuItem("100%", NULL, zoomLevel == 100)) zoomLevel = 100;
                if (ImGui::MenuItem("125%", NULL, zoomLevel == 125)) zoomLevel = 125;
                if (ImGui::MenuItem("150%", NULL, zoomLevel == 150)) zoomLevel = 150;
                ImGui::EndMenu();
            }

            // Adjust global font size based on the selected zoom level
            float scale = static_cast<float>(zoomLevel) / 100.0f;
            ImGui::GetIO().FontGlobalScale = scale;

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
    // End Menu Bar
    /////////////////////////////////////////////////////////////////////////////////////////////

	// Application Main Window

    ///////////////////////////////////////////////////////////////////////////////////////////// 
    // REST CLIENT
	/////////////////////////////////////////////////////////////////////////////////////////////
	ImGui::Begin("RestClient");

        ImGui::Text("Get Request");

        static char urlBuffer[256] = "http://127.0.0.1:11707/status";
        ImGui::InputText("URL", urlBuffer, IM_ARRAYSIZE(urlBuffer));

	    if (ImGui::Button("Send Request"))
	    {
            auto logger = spdlog::get("IMGUI_LOGGER");
		    if (logger)
		    {

                std::string response = restClient::doGet(urlBuffer);
                logger->info("Response: {}", response);
		    }
	    }

        ImGui::Spacing();
        ImGui::Separator();    

	ImGui::End();

    ///////////////////////////////////////////////////////////////////////////////////////////// 
    // WS CLIENT
    /////////////////////////////////////////////////////////////////////////////////////////////
    ImGui::Begin("WebsocketClient");

    // URI input and connect button
    static char wsUriBuffer[256] = "ws://127.0.0.1:11808/";
    ImGui::InputText("WebSocket URI", wsUriBuffer, IM_ARRAYSIZE(wsUriBuffer));
    
    // Track connection status for button colors
    static bool isConnected = false;
    bool wasConnected = isConnected; // capture current state
    
    // Set button color based on connection status
    if (wasConnected)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.8f, 0.0f, 1.0f)); // Green
    
    if (ImGui::Button("Connect"))
    {
        auto logger = spdlog::get("IMGUI_LOGGER");
        if (strlen(wsUriBuffer) == 0)
        {
            if (logger) logger->error("Cannot connect: the WebSocket URI is empty.");
        }
        else
        {
            bool success = wsClient::connect(wsUriBuffer);
            if (logger)
            {
                if (success) 
                {
                    logger->info("Connected to WebSocket: {}", wsUriBuffer);
                    isConnected = true;
                }
                else 
                {
                    logger->error("Failed to connect. Check URL/port or logs for details: {}", wsUriBuffer);
                    isConnected = false;
                }
            }
        }
    }
    
    // Pop the color style if we pushed it
    if (wasConnected)
        ImGui::PopStyleColor();

    ImGui::SameLine();
    if (ImGui::Button("Close"))
    {
        wsClient::close();
        auto logger = spdlog::get("IMGUI_LOGGER");
        if (logger) logger->info("WebSocket closed");
        isConnected = false;
    }

    // A list of commands plus an 'Add' button
    static std::vector<std::string> commands;
    static char commandBuffer[256] = "{ \"feature\": \"winAudio\", \"action\": \"getVolume\" }";

    ImGui::Separator();
    ImGui::Text("Commands to Send:");
    
    // Style the command input with a different color
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.4f, 1.0f)); // Bluish tint
    ImGui::InputText("##NewCommand", commandBuffer, IM_ARRAYSIZE(commandBuffer));
    ImGui::PopStyleColor();
    
    ImGui::SameLine();
    if (ImGui::Button("Add"))
    {
        commands.emplace_back(commandBuffer);
    }

    // Display each command entry with a Send button and a Remove button
    for (size_t i = 0; i < commands.size(); ++i)
    {
        ImGui::PushID(static_cast<int>(i));
        
        // Display the command
        ImGui::InputText("##CommandItem", const_cast<char*>(commands[i].c_str()),
            commands[i].size() + 1, ImGuiInputTextFlags_ReadOnly);
        
        ImGui::SameLine();
        if (ImGui::Button("Send"))
        {
            bool result = wsClient::send(commands[i]);
            auto logger = spdlog::get("IMGUI_LOGGER");
            if (logger)
            {
                if (result)
                {
                    logger->info("Sent command: {}", commands[i]);

                    // Retrieve and print the response to the console
                    auto response = wsClient::getLastResponse();
                    logger->info("Response: {}", response);
                }
                else
                {
                    logger->error("Failed to send command: {}", commands[i]);
                }
            }
        }
        
        ImGui::SameLine();
        if (ImGui::Button("-")) // Remove button
        {
            commands.erase(commands.begin() + i);
            i--; // Adjust index after removal
        }
        
        ImGui::PopID();
    }

    ImGui::End();
	/////////////////////////////////////////////////////////////////////////////////////////////      

    // Render the console window
    renderConsoleWindow();

    /////////////////////////////////////////////////////////////////////////////////////////////

    ImGui::ShowDemoWindow();

    /////////////////////////////////////////////////////////////////////////////////////////////

    // Rendering
    ImGui::Render();

    // Update and Render additional Platform Windows (for multi-viewport support)
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
                ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault(nullptr);
    }

    // 4) Render ImGui draw data to your active render target
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // 5) Present to the screen
    //    vsync = 1 for 60fps, or 0 for uncapped
    g_pSwapChain->Present(1, 0);
}

void renderConsoleWindow()
{
    ImGui::Begin("Console");

    // Options for the console
    static bool autoScroll = true;

    // Dropdown for log level
    static int currentLogLevel = spdlog::level::trace; // Default to the most verbose level
    const char* logLevelItems[] = {
        "Trace",
        "Debug",
        "Info",
        "Warning",
        "Error",
        "Critical",
        "Off"
    };

    // Dropdown for log level
    ImGui::Text("Log Level: ");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (ImGui::BeginCombo("##LogLevel", logLevelItems[currentLogLevel]))
    {
        for (int i = 0; i < IM_ARRAYSIZE(logLevelItems); ++i)
        {
            const bool isSelected = (currentLogLevel == i);
            if (ImGui::Selectable(logLevelItems[i], isSelected))
            {
                currentLogLevel = i;

                // Update the logger level
                auto logger = spdlog::get("IMGUI_LOGGER");
                if (logger)
                {
                    logger->set_level(static_cast<spdlog::level::level_enum>(currentLogLevel));
                    logger->info("Log level changed to {}", logLevelItems[currentLogLevel]);
                }
            }
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    if (ImGui::Button("Clear"))
    {
        auto logger = spdlog::get("IMGUI_LOGGER");
        if (logger)
        {
            auto sinks = logger->sinks();
            for (const auto& sink : sinks)
            {
                auto imgui_sink = std::dynamic_pointer_cast<imguiSink::ImGuiSink>(sink);
                if (imgui_sink)
                {
                    imgui_sink->clear();
                    logger->info("Console cleared.");
                }
            }
        }
    }

    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &autoScroll);
    ImGui::Separator();

    // Begin a child region to enable scrolling
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    // Fetch logs from the sink
    auto logger = spdlog::get("IMGUI_LOGGER");
    if (logger)
    {
        auto sinks = logger->sinks();
        for (const auto& sink : sinks)
        {
            auto imgui_sink = std::dynamic_pointer_cast<imguiSink::ImGuiSink>(sink);
            if (imgui_sink)
            {
                const auto& logs = imgui_sink->get_logs();
                for (const auto& log : logs)
                {
                    ImGui::TextWrapped("%s", log.c_str());
                }

                if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                {
                    ImGui::SetScrollHereY(1.0f);
                }
            }
        }
    }

    ImGui::EndChild();
    ImGui::End();
}

std::string openFolderPicker(const char* dialogTitle)
{
    // Convert dialogTitle to wide string
    std::wstring wDialogTitle = stringToWString(dialogTitle);

    // Initialize the BROWSEINFO structure
    BROWSEINFOW bi = { 0 };
    bi.lpszTitle = wDialogTitle.c_str();
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    // Display the folder picker dialog
    LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
    if (pidl != nullptr)
    {
        wchar_t pathW[MAX_PATH];
        // Convert the item ID list to a file system path
        if (SHGetPathFromIDListW(pidl, pathW))
        {
            // Convert wide string back to std::string
            std::wstring wPath(pathW);
            std::string path = wStringToString(wPath);

            // Free the memory allocated for the item ID list
            IMalloc* imalloc = nullptr;
            if (SUCCEEDED(SHGetMalloc(&imalloc)))
            {
                imalloc->Free(pidl);
                imalloc->Release();
            }
            return path;
        }
        // Free the memory if path conversion fails
        IMalloc* imalloc = nullptr;
        if (SUCCEEDED(SHGetMalloc(&imalloc)))
        {
            imalloc->Free(pidl);
            imalloc->Release();
        }
    }
    return "";
}

// Helper function to convert std::string to std::wstring
std::wstring stringToWString(const std::string& s)
{
    if (s.empty()) return std::wstring();
    int len;
    int slength = static_cast<int>(s.length());
    len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), slength, nullptr, 0);
    std::wstring ws(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), slength, &ws[0], len);
    return ws;
}

// Helper function to convert std::wstring to std::string
std::string wStringToString(const std::wstring& ws)
{
    if (ws.empty()) return std::string();
    int len;
    int slength = static_cast<int>(ws.length());
    len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), slength, nullptr, 0, nullptr, nullptr);
    std::string s(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), slength, &s[0], len, nullptr, nullptr);
    return s;
}

std::string openFileOpenDialog(const char* title)
{
    char filename[MAX_PATH] = "";
    OPENFILENAMEA ofn = { sizeof(OPENFILENAMEA) };
    ofn.hwndOwner = nullptr; // If you have a window handle, set it here
    ofn.lpstrFilter = "INI Files\0*.ini\0All Files\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = title;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn))
    {
        return std::string(filename);
    }
    return "";
}

std::string openFileSaveDialog(const char* title)
{
    char filename[MAX_PATH] = "";
    OPENFILENAMEA ofn = { sizeof(OPENFILENAMEA) };
    ofn.hwndOwner = nullptr;
    ofn.lpstrFilter = "INI Files\0*.ini\0All Files\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = title;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

	// set defualt file type to .ini
	ofn.lpstrDefExt = "ini";


    if (GetSaveFileNameA(&ofn))
    {
        return std::string(filename);
    }
    return "";
}

// ui.cpp