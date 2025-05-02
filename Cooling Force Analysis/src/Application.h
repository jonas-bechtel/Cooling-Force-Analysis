#pragma once
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include "Curve.h"

class Application
{
public:
	void InitImGui();
	void Init();
	void Run();
	void ShowMainWindow();
	void ShutdownImGui();

private:
	void ShowPlots();
	void ShowControls();

private:
	// main data 
	Curve curve;

	// Imgui things
	WNDCLASSEXW wc;
	HWND hwnd;
};
