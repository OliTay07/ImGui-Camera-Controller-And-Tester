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
#include "presets.hpp"

#include <nlohmann/json.hpp>
#include <iostream>



module ui;



import imguiSink;

void renderUI(ID3D11DeviceContext* g_pd3dDeviceContext,
    ID3D11RenderTargetView* g_mainRenderTargetView,
    IDXGISwapChain* g_pSwapChain,
    presets::commandValues& commandValues)
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
    static char wsUriBuffer[256] = "ws://192.168.0.179:11808/";
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





    /*   ImGui::Text("Name:");
       ImGui::SameLine(300); ImGui::SetNextItemWidth(200);
       static char bufferName[256] = "";
       ImGui::InputText("##NodeName", bufferName, IM_ARRAYSIZE(bufferName), ImGuiInputTextFlags_CharsNoBlank);*/

    ImGui::Spacing();

    //Add new command
    ImGui::Text("Add new cmd");
    ImGui::Spacing();

    //Feature
    ImGui::Text("Feature:");
    ImGui::SameLine(300);
    ImGui::SetNextItemWidth(200);

    // Static variables for the input and combo box
    static int selectedFeature = -1;
    // Buffer for the input text
    static char featureInput[256] = "";
    const char* featureList[] = { "camsHandler" };
    int featureCount = IM_ARRAYSIZE(featureList);



    // Open the custom combo box for Feature
    const char* comboLabel = (selectedFeature == -1 && featureInput[0] == '\0') ? "Select or type" : featureInput;
    if (ImGui::BeginCombo("##FeatureDropdown", comboLabel)) {

        // Input text at the top of the dropdown
        bool inputChanged = ImGui::InputText("##FeatureInput", featureInput, sizeof(featureInput));
        ImGui::Spacing(); // Add spacing between input and the options

        // If the input text changes, clear the selected index
        if (inputChanged) {
            // Resets the selection if the user starts typing
            selectedFeature = -1;
        }

        // Loop through the list of features and display them
        for (int i = 0; i < featureCount; ++i) {
            bool isSelected = (selectedFeature == i);
            if (ImGui::Selectable(featureList[i], isSelected)) {
                selectedFeature = i;
                // Update the input with the selected value
                strcpy_s(featureInput, featureList[i]);

                commandValues.feature = featureInput;

            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        /*
          strncpy_s(bufferName, configManager.project.projectConfig.projectName.c_str(), IM_ARRAYSIZE(bufferName));
          if (ImGui::InputText("##ProjectName", bufferName, IM_ARRAYSIZE(bufferName)))
              configManager.project.projectConfig.projectName = bufferName;*/

        ImGui::EndCombo();
    }

    ImGui::Spacing();
    
    // ID dropdown and input
    ImGui::Text("ID:");
    ImGui::SameLine(300);
    ImGui::SetNextItemWidth(200);

    // Static variables for the input and combo box for ID
    static int selectedID = -1;
    static char idInput[256] = "";
    const char* idList[] = { "1" };
    int idCount = IM_ARRAYSIZE(idList);

    // Open the custom combo box for ID

    const char* comboLabel1 = (selectedID == -1 && idInput[0] == '\0') ? "Select or type" : idInput;
    if (ImGui::BeginCombo("##idDropdown", comboLabel1)) {
        // Input text at the top of the dropdown
        bool inputChanged = ImGui::InputText("##idInput", idInput, sizeof(idInput));
        ImGui::Spacing();

        // If the input text changes, clear the selected index
        if (inputChanged) {
            selectedID = -1;
        }

        // Loop through the list of IDs and display them (use idList, not featureList)
        for (int i = 0; i < idCount; ++i) {
            bool isSelected = (selectedID == i);
            if (ImGui::Selectable(idList[i], isSelected)) {
                selectedID = i;
                strcpy_s(idInput, sizeof(idInput), idList[i]);

                commandValues.id = idInput;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();

    }
    ImGui::Spacing();

    //Action text & dropdown
    ImGui::Text("Action:");
    ImGui::SameLine(300);
    ImGui::SetNextItemWidth(200);

    // Static variables for the input and combo box
    static int selectedAction = -1;
    static char actionInput[256] = "";
    const char* actionList[] = { "SetCameraState", "SetCameraPan", "SetCameraTilt", "SetCameraZoom", "SetCameraWhiteBalance", "SetCameraPreset" };
    int actionCount = IM_ARRAYSIZE(actionList);

    const char* comboLabel2 = (selectedAction == -1 && actionInput[0] == '\0') ? "Select or type" : actionInput;
    if (ImGui::BeginCombo("##actionDropdown", comboLabel2)) {

        // Input text at the top of the dropdown
        bool inputChanged = ImGui::InputText("##actionInput", actionInput, sizeof(actionInput));
        ImGui::Spacing();

        if (inputChanged) {
            selectedAction = -1;
        }

        // Loop through the list of actions and display them
        for (int i = 0; i < actionCount; ++i) {
            bool isSelected = (selectedAction == i);
            if (ImGui::Selectable(actionList[i], isSelected)) {
                selectedAction = i;
                strcpy_s(actionInput, sizeof(actionInput), actionList[i]);

                commandValues.action = actionInput;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Spacing();


    //Parameter text & dropdown
    ImGui::Text("Parameter:");
    ImGui::SameLine(300); ImGui::SetNextItemWidth(200);


    // Static variables for the input and combo box
    static int selectedParameter = -1;
    // Buffer for the input text
    static char parameterInput[256] = "";
    const char* parameterList[] = { "on", "off", "left", "right", "stop", "up", "down", "increase", "decrease", "auto-on", "auto-off", "1" };
    int parameterCount = IM_ARRAYSIZE(parameterList);


    const char* comboLabel3 = (selectedParameter == -1 && parameterInput[0] == '\0') ? "Select or type" : parameterInput;
    if (ImGui::BeginCombo("##parameterDropdown", comboLabel3)) {

        // Input text at the top of the dropdown
        bool inputChanged = ImGui::InputText("##parameterInput", parameterInput, sizeof(parameterInput));
        ImGui::Spacing();



        if (inputChanged) {
            selectedParameter = -1;
        }

        // Loop through the list of actions and display them
        for (int i = 0; i < parameterCount; ++i) {
            bool isSelected = (selectedParameter == i);
            if (ImGui::Selectable(parameterList[i], isSelected)) {
                selectedParameter = i;
                strcpy_s(parameterInput, sizeof(parameterInput), parameterList[i]);

                commandValues.action = parameterInput;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }


        ImGui::EndCombo();
    }


    //Identifiers
    static float timer = 0.0f;
    static bool commandAdded = false;
    static bool commandNotAdded = false;

    if (ImGui::Button("Add Command"))
    {
        // Check if input fields or dropdowns are filled
        bool featureFilled = (selectedFeature != -1 || featureInput[0] != '\0');
        bool idFilled = (selectedID != -1 || idInput[0] != '\0');
        bool actionFilled = (selectedAction != -1 || actionInput[0] != '\0');
        bool parameterFilled = (selectedParameter != -1 || parameterInput[0] != '\0');
       

        // Only allow adding the command if all fields are filled (either selected or typed)
        if (featureFilled && idFilled && actionFilled && parameterFilled)
        {

            std::string file = "C:/Users/taylo/Documents/_Repos/template-imguiWinAppDX11-with-rest-and-ws-client/preset.json";

            presets::save(file, commandValues);


            // Construct the command string from the input fields
            std::string command = std::string(featureInput) + ":" + std::string(idInput) + ":" +
                std::string(actionInput) + ":" + std::string(parameterInput);

            // Add the command to the list
            commands.emplace_back(command);
            commandAdded = true;
            commandNotAdded = false;
        }
        else
        {
            commandAdded = false;
            commandNotAdded = true;
        }
    }
   

    // Displays "Command Added" message if the flag is set
    if (commandAdded)
    {
        //Displays in orange
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
        ImGui::Text("Command Added!");
        timer += ImGui::GetIO().DeltaTime;
        ImGui::PopStyleColor();


        //Resets after 3 seconds
        if (timer > 3.0f)
        {
            commandAdded = false;
        }
        ImGui::PopStyleColor();
    }

    // Displays "Command Not Added" message if a dropdown or input is empty
    else if (commandNotAdded)
    {

        //Displays in orange
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        ImGui::Text("Command NOT Added! Please fill out all dropdowns or input fields.");
        ImGui::PopStyleColor();
    }



    //show a load button
    //if load button is pressed
    //push back each entry onto the commands vector

    if (ImGui::Button("Load"))
    {

        std::string path = "C:/Users/taylo/Documents/_Repos/template-imguiWinAppDX11-with-rest-and-ws-client/preset.json";
        std::vector<std::string> temp = presets::load(path);
        for (size_t i = 0; i < temp.size(); i++)
        {
            commands.emplace_back(temp[i]);
        }


    }

    //////////////////////////SUCCESS / FAIL//////////////////////////////////



    ImGui::Begin("Control Window");

    ///// POWER ON /////
    if (ImGui::Button("Power On")) {
        // Power on logic here
    }

    ImGui::SameLine(100);

    // Variable to hold the selected radio button value for power on
    static int selectedOn = -1;

    // Create unique scope for radio buttons using PushID
    ImGui::PushID("PowerOn");


    //Changes the selected colour to green
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.f, 1.f, 0.f, 1.f));
    if (ImGui::RadioButton("Success", selectedOn == 0)) {
        // Set selected value to 0, which means success
        selectedOn = 0;
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();

    //Changes the selected colour to red
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.f, 0.f, 0.f, 1.f));
    if (ImGui::RadioButton("Fail", selectedOn == 1)) {
        // Set selected value to 1, which means fail
        selectedOn = 1;
    }
    ImGui::PopStyleColor();

    ImGui::PopID(); 

    ImGui::Spacing();

    ///// POWER OFF /////
    if (ImGui::Button("Power Off")) {
        // Power off logic will go here
    }

    ImGui::SameLine(100);

    // Variable to hold the selected radio button value for power off
    static int selectedOff = -1;

    // Create unique scope for radio buttons using PushID
    ImGui::PushID("PowerOff");

    //Changes the selected colour to green
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.f, 1.f, 0.f, 1.f));
    if (ImGui::RadioButton("Success", selectedOff == 0)) {
        // Set selected value to 0, which means success for power off
        selectedOff = 0;
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();

    //Changes the selected colour to red
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.f, 0.f, 0.f, 1.f));
    if (ImGui::RadioButton("Fail", selectedOff == 1)) {
        // Set selected value to 1, which means fail for power off
        selectedOff = 1;
    }
    ImGui::PopStyleColor();

    ImGui::PopID();

    ImGui::Spacing();

    ///// PAN LEFT /////
    if (ImGui::Button("Pan Left")) {
        // Power off logic will go here
    }

    ImGui::SameLine(100);

    // Variable to hold the selected radio button value for pan left
    static int selectedLeft = -1;

    // Create unique scope for radio buttons using PushID
    ImGui::PushID("PanLeft");

    //Changes the selected colour to green
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.f, 1.f, 0.f, 1.f));
    if (ImGui::RadioButton("Success", selectedLeft == 0)) {
        // Set selected value to 0, which means success for pan left
        selectedLeft = 0;
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();

    //Changes the selected colour to red
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.f, 0.f, 0.f, 1.f));
    if (ImGui::RadioButton("Fail", selectedLeft == 1)) {
        // Set selected value to 1, which means fail for pan left
        selectedLeft = 1;
    }
    ImGui::PopStyleColor();

    ImGui::PopID();


    ///// PAN RIGHT /////
    if (ImGui::Button("Pan Right")) {
        // Power off logic will go here
    }

    ImGui::SameLine(100);

    // Variable to hold the selected radio button value for pan right
    static int selectedRight = -1;

    // Create unique scope for radio buttons using PushID
    ImGui::PushID("PanRight");

    //Changes the selected colour to green
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.f, 1.f, 0.f, 1.f));
    if (ImGui::RadioButton("Success", selectedRight == 0)) {
        // Set selected value to 0, which means success for pan right
        selectedRight = 0;
    }
    ImGui::PopStyleColor();

     ImGui::SameLine();

     //Changes the selected colour to red
     ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.f, 0.f, 0.f, 1.f));
    if (ImGui::RadioButton("Fail", selectedRight == 1)) {
        // Set selected value to 1, which means fail for pan right
        selectedRight = 1;
    }
    ImGui::PopStyleColor();

    ImGui::PopID();


    ///// TILT UP /////
    if (ImGui::Button("Tilt Up")) {
        
        

    }

    ImGui::SameLine(100);


    // Variable to hold the selected radio button value for tilt up
    static int TiltUp = -1;

    // Create unique scope for radio buttons using PushID
    ImGui::PushID("TiltUp");



    //Changes the selected colour to green
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.f, 1.f, 0.f, 1.f));
    if (ImGui::RadioButton("Success", TiltUp == 0)) {
        // Set selected value to 0, which means success for tilt up
        TiltUp = 0;
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();


    //Changes the selected colour to red
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.f, 0.f, 0.f, 1.f));
    if (ImGui::RadioButton("Fail", TiltUp == 1)) {
        // Set selected value to 1, which means fail for tilt up
        TiltUp = 1;
    }
    ImGui::PopStyleColor();

    ImGui::PopID();

    ///// TILT DOWN /////
    if (ImGui::Button("Tilt Down")) {
        // Power off logic will go here
    }

    ImGui::SameLine(100);


    // Variable to hold the selected radio button value for tilt down
    static int TiltDown = -1;

    // Create unique scope for radio buttons using PushID
    ImGui::PushID("TiltDown");


    //Changes the selected colour to green
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.f, 1.f, 0.f, 1.f));
    if (ImGui::RadioButton("Success", TiltDown == 0)) {
        // Set selected value to 0, which means success for tilt down
        TiltDown = 0;
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();

    //Changes the selected colour to red
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.f, 0.f, 0.f, 1.f));
    if (ImGui::RadioButton("Fail", TiltDown == 1)) {
        // Set selected value to 1, which means fail for tilt down
        TiltDown = 1;
    }
    ImGui::PopStyleColor();

    ImGui::PopID();


    ///// ZOOM INCREASE /////
    if (ImGui::Button("Zoom In")) {
        
    }

    ImGui::SameLine(100);


    // Variable to hold the selected radio button value for zoom in
    static int ZoomIn = -1;

    // Create unique scope for radio buttons using PushID
    ImGui::PushID("ZoomIn");


    //Changes the selected colour to green
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.f, 1.f, 0.f, 1.f));
    if (ImGui::RadioButton("Success", ZoomIn == 0)) {
        // Set selected value to 0, which means success for zoom in
        ZoomIn = 0;
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();


    //Changes the selected colour to red
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.f, 0.f, 0.f, 1.f));
    if (ImGui::RadioButton("Fail", ZoomIn == 1)) {
        // Set selected value to 1, which means fail for zoom in
        ZoomIn = 1;
    }
    ImGui::PopStyleColor();

    ImGui::PopID();

    ///// ZOOM DECREASE /////
    if (ImGui::Button("Zoom Out")) {
        // Power off logic will go here
    }

    ImGui::SameLine(100);

    // Variable to hold the selected radio button value for zoom out
    static int ZoomOut = -1;

    // Create unique scope for radio buttons using PushID
    ImGui::PushID("ZoomOut");

    //Changes the selected colour to green
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.f, 1.f, 0.f, 1.f));
    if (ImGui::RadioButton("Success", ZoomOut == 0)) {
        // Set selected value to 0, which means success for zoom out
        ZoomOut = 0;
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();

    //Changes the selected colour to red
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.f, 0.f, 0.f, 1.f));
    if (ImGui::RadioButton("Fail", ZoomOut == 1)) {
        // Set selected value to 1, which means fail for zoom out
        ZoomOut = 1;
    }
    ImGui::PopStyleColor();

    ImGui::PopID();


    ///// ZOOM STOP /////
    if (ImGui::Button("Zoom Stop")) {
        // Power off logic will go here
    }

    ImGui::SameLine(100);

    // Variable to hold the selected radio button value for zoom stop
    static int ZoomStop = -1;

    // Create unique scope for radio buttons using PushID
    ImGui::PushID("ZoomStop");

    //Changes the selected colour to green
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.f, 1.f, 0.f, 1.f));
    if (ImGui::RadioButton("Success", ZoomStop == 0)) {
        // Set selected value to 0, which means success for zoom stop
        ZoomStop = 0;
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();


    //Changes the selected colour to red
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.f, 0.f, 0.f, 1.f));
    if (ImGui::RadioButton("Fail", ZoomStop == 1)) {
        // Set selected value to 1, which means fail for zoom stop
        ZoomStop = 1;
    }
    ImGui::PopStyleColor();

    ImGui::PopID();


    ///// SET PRESET /////
    if (ImGui::Button("Set Preset")) {
        
    }

    ImGui::SameLine(100);

    // Variable to hold the selected radio button value for set preset
    static int SetPreset = -1;

    // Create unique scope for radio buttons using PushID
    ImGui::PushID("SetPreset");


    //Changes the selected colour to green
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.f, 1.f, 0.f, 1.f));
    if (ImGui::RadioButton("Success", SetPreset == 0)) {
        // Set selected value to 0, which means success for  set preset
        SetPreset = 0;
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();


    //Changes the selected colour to red
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.f, 0.f, 0.f, 1.f));
    if (ImGui::RadioButton("Fail", SetPreset == 1)) {
        // Set selected value to 1, which means fail for  set preset
        SetPreset = 1;
    }
    ImGui::PopStyleColor();

    ImGui::PopID();


    ///// GET PRESET /////
    if (ImGui::Button("Get Preset")) {

    }

    ImGui::SameLine(100);

    // Variable to hold the selected radio button value for get preset
    static int GetPreset = -1;

    // Create unique scope for radio buttons using PushID
    ImGui::PushID("GetPreset");


    //Changes the selected colour to green
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.f, 1.f, 0.f, 1.f));
    if (ImGui::RadioButton("Success", GetPreset == 0)) {
        // Set selected value to 0, which means success for  get preset
        GetPreset = 0;
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();


    //Changes the selected colour to red
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.f, 0.f, 0.f, 1.f));
    if (ImGui::RadioButton("Fail", GetPreset == 1)) {
        // Set selected value to 1, which means fail for  get preset
        GetPreset = 1;
    }
    ImGui::PopStyleColor();

    ImGui::PopID();


    ///// AUTO ON /////
    if (ImGui::Button("Auto On")) {

    }

    ImGui::SameLine(100);

    // Variable to hold the selected radio button value for auto on
    static int AutoOn = -1;

    // Create unique scope for radio buttons using PushID
    ImGui::PushID("AutoOn");


    //Changes the selected colour to green
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.f, 1.f, 0.f, 1.f));
    if (ImGui::RadioButton("Success", AutoOn == 0)) {
        // Set selected value to 0, which means success for  auto on
        AutoOn = 0;
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();

    //Changing the selected colour to red
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.f, 0.f, 0.f, 1.f));
    if (ImGui::RadioButton("Fail", AutoOn == 1)) {
        // Set selected value to 1, which means fail for auto on
        AutoOn = 1;
    }
    ImGui::PopStyleColor();

    ImGui::PopID();


    ///// AUTO OFF /////
    if (ImGui::Button("Auto Off")) {

    }

    ImGui::SameLine(100);

    // Variable to hold the selected radio button value for get preset
    static int AutoOff = -1;

    // Create unique scope for radio buttons using PushID
    ImGui::PushID("AutoOff");

    //Changes the selected colour to green
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.f, 1.f, 0.f, 1.f));
    if (ImGui::RadioButton("Success", AutoOff == 0)) {
        // Set selected value to 0, which means success for  get preset
        AutoOff = 0;
    }
    ImGui::PopStyleColor();
    

    ImGui::SameLine();

    //Changes the selected colour to red
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.f, 0.f, 0.f, 1.f));
    if (ImGui::RadioButton("Fail", AutoOff == 1)) {
        // Set selected value to 1, which means fail for  get preset
        AutoOff = 1;
    }
    ImGui::PopStyleColor();

    ImGui::PopID();

    ImGui::Spacing();
    ImGui::Dummy(ImVec2(0, 10));

   
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 1.0f)); // RGB (0, 255, 0)
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f)); // Black text
    if (ImGui::Button("Export")) {


         
    }

    ImGui::PopStyleColor();
    ImGui::PopStyleColor();

    ImGui::End();

    
    //////////////////////////SUCCESS / FAIL//////////////////////////////////

    

 
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

   /* ImGui::ShowDemoWindow();*/

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

std::string openJsonFileDialog(const char* title)
{
    char filename[MAX_PATH] = "";
    OPENFILENAMEA ofn = { sizeof(OPENFILENAMEA) };
    ofn.hwndOwner = nullptr; // If you have a window handle, set it here
    ofn.lpstrFilter = "Json Files\0*.json\0All Files\0*.*\0";
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


// ui.cpp