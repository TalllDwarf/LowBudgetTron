#include "stdafx.h"
#include "LittleEngineD12.h"
#include "StepTimer.h"
#include "Input.h"
#include <windowsx.h>

// Handle to the window.
HWND m_hWnd;

//name of the window, this is not the title
LPCTSTR WindowName = "LittleEngineApp";

//title of the window
LPCTSTR WindowTitle = "Little Engine";

// callback function for windows messages
LRESULT CALLBACK WndProc(HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam);

//Directx 12
std::unique_ptr<LittleEngineD12> littleEngine;

//Handles keyboard mouse input
Input playerInput;

//Timer
DX::StepTimer timer;

bool running;

bool InitializeWindow(HINSTANCE hInstance, int showWnd, int width, int height, bool fullscreen)
{
	timer.SetFixedTimeStep(true);
	timer.SetTargetElapsedSeconds(1.0 / 50);

	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WindowName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "Error registering class", "ERROR", MB_OK | MB_ICONERROR);
		return false;
	}

	m_hWnd = CreateWindowEx(NULL,
		WindowName,
		WindowTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (!m_hWnd)
	{
		MessageBox(NULL, "Error creating window", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	if (fullscreen)
	{
		SetWindowLong(m_hWnd, GWL_STYLE, 0);
	}

	ShowWindow(m_hWnd, showWnd);
	UpdateWindow(m_hWnd);

	if (playerInput.RegisterDevices() != 0)
	{
		return false;
	}

	return true;
}

void mainLoop(int width, int height)
{
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	ShowCursor(false);

	width /= 2;
	height /= 2;

	while (littleEngine->IsRunning() && running)
	{
		timer.Tick([&]()
		{
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);

				if (msg.message == WM_INPUT)
				{
					playerInput.GetData(msg.lParam);
				}			
				else if (msg.message == WM_QUIT)
				{
					running = false;
				}
			}
			
			if(running)
			{
				//Player update
				playerInput.Update();
				SetCursorPos(width, height);

				//Game
				littleEngine->Update(timer.GetElapsedSeconds());
				littleEngine->Render();

				playerInput.ResetAllKeyData();
			}
		});
	}

	ShowCursor(true);

}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HMONITOR hmon = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi = { sizeof(mi) };
	GetMonitorInfo(hmon, &mi);

	int Width = mi.rcMonitor.right - mi.rcMonitor.left;
	int Height = mi.rcMonitor.bottom - mi.rcMonitor.top;

	//is fullscreen
	bool Fullscreen = false;

	if (!InitializeWindow(hInstance, nCmdShow, Width, Height, Fullscreen))
	{
		MessageBox(0, "Window Initialization - Failed", "Error", MB_OK);
		return 0;
	}

	//Directx 
	littleEngine = std::make_unique<LittleEngineD12>();

	if (!littleEngine->InitD3D(m_hWnd, Width, Height, Fullscreen, &playerInput))
	{
		return 2;
	}

	running = true;

	mainLoop(Width, Height);

	//Finished
	littleEngine->Cleanup();

	return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd,	UINT msg,	WPARAM wParam,	LPARAM lParam)
{
	switch (msg)
	{

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {

			ShowCursor(true);

			if (MessageBox(0, "Are you sure you want to exit?",
				"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
			{
				PostQuitMessage(0);
				//DestroyWindow(hwnd);
			}
			else
				ShowCursor(false);
				
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd,
		msg,
		wParam,
		lParam);
}