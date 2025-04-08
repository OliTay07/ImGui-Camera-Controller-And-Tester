// ui.ixx

module;

#include "imgui.h"
#include <d3d11.h>
#include <windows.h>
#include <dxgi1_4.h>
#include <string>
#include <vector>

#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

export module ui;

// Exported functions and declarations
export void renderUI(ID3D11DeviceContext* g_pd3dDeviceContext, 
	ID3D11RenderTargetView* g_mainRenderTargetView,
	IDXGISwapChain* g_pSwapChain);

void renderConsoleWindow();

std::string openFolderPicker(const char* dialogTitle);
std::string openFileOpenDialog(const char* title);
std::string openFileSaveDialog(const char* title);

// Helper function to convert std::string to std::wstring
std::wstring stringToWString(const std::string& s);
std::string wStringToString(const std::wstring& ws);

// ui.ixx