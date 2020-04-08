#pragma once
#include <Windows.h>
#include <Xinput.h>


enum
{
	VKey_A = 0x41,
	VKey_B,
	VKey_C,
	VKey_D,
	VKey_E,
	VKey_F,
	VKey_G,
	VKey_H,
	VKey_I,
	VKey_J,
	VKey_K,
	VKey_L,
	VKey_M,
	VKey_N,
	VKey_O,
	VKey_P,
	VKey_Q,
	VKey_R,
	VKey_S,
	VKey_T,
	VKey_U,
	VKey_V,
	VKey_W,
	VKey_X,
	VKey_Y,
	VKey_Z
};

enum
{
	VKey_LeftArrow = 0x25,
	VKey_UpArrow,
	VKey_RightArrow,
	VKey_DownArrow
};

class Input
{
	//
	//
	//Mouse and keyboard input
	//
	//
public:
	Input();
	~Input();

	INT RegisterDevices();

	//Called everytime we get WM_INPUT
	void GetData(LPARAM lParam);

	bool IsLeftMouseButtonDown() { return m_bLMB; }
	bool IsRightMouseButtonDown() { return m_bRMB; }

	bool OnLeftMouseButtonDown() { return (!m_oldbLMB && m_bLMB); }
	bool OnRightMouseButtonDown() { return (!m_oldbRMB && m_bRMB); }

	int GetMouseChangeX() { int mouseChange = m_nMouseXChange; m_nMouseXChange = 0; return mouseChange; }
	int GetMouseChangeY() { int mouseChange = m_nMouseYChange; m_nMouseYChange = 0; return mouseChange; }

	// To obtain exact mouse coords check the uMsg in your Application's MsgProc
	// for WM_MOUSEMOVE, and use HIWORD() LOWORD() functions to extract the mouse X,Y
	// from lParam. Store them in the below variables.

	int m_nMouseX;
	int m_nMouseY;

	bool IsAlphabeticKeyDown(int letter) { return m_baAlphabet[(letter - 0x41)]; }

	bool IsArrowKeyDown(int arrowDirection) { return m_baArrows[(arrowDirection - 0x25)]; }
	
	bool KeyPress(int letter) { return (m_baAlphabet[(letter - 0x41)] && !m_baOldAlphabet[(letter - 0x41)]); }

	bool IsShiftKeyDown() { return m_bShift; }

	bool IsEnterKeyDown() { return m_bEnter; }

	void ResetAllKeyData();

	//Easy movement access

	float Forward(float deltaTime);
	float Right(float deltaTime);
	float Up(float deltaTime);
	float GetRightChangeX(float deltaTime);
	float GetRightChangeY(float deltaTime);

private:

	//Two input devices are covered mouse and keyboard
	RAWINPUTDEVICE m_Rid[2];

	//Mouse size is 40 keyboard is 32 
	//We will fill it with one at a time
	UINT8 m_buffer[48];
	//BYTE m_buffer[40];

	int m_nMouseXChange;
	int m_nMouseYChange;

	bool m_bLMB;
	bool m_bRMB;

	bool m_oldbLMB;
	bool m_oldbRMB;

	// In the source file you will see how my enumerations come into play for knowing what index
	// to access.
	bool           m_baAlphabet[25];
	bool           m_baOldAlphabet[25];
	bool           m_baArrows[4];
	bool		   m_bShift;
	bool		   m_bEnter;

	// The CheckKeyPress function is because of these issues:

	// 1. The Mouse
	// The flag is only set once when the button is pressed and so to determine if the mouse button is being held down you need to record it as down
	// until you get a RI_MOUSE_LEFT_BUTTON_UP flag.

	// 2. The Keyboard
	// When a key is held down the raw->data.heyboard.Flags has the RI_KEY_MAKE  bit set (actually none as the value is 0) and when the key is released 
	// the  RI_KEY_BREAK  bit is set (value is 1). 

	// 1 and 2 is what I got directly from ToyMaker's website. I believe for the most part CheckKeyPress takes what was said into account.
	bool           CheckKeyPress(bool bLastKeyState, bool bThisKeyState);

	//
	//
	//
	//Gamepad input
	//
	//
	//
public:

	void Update();
	bool GamepadConnected();
	bool IsButtonDown(unsigned short int button);
	bool WasButtonPressDetected(unsigned short int button);
	bool WasButtonReleaseDetcted(unsigned short int button);

	void UpdateLeftAnalog();
	void UpdateRightAnalog();

	void SetVibration(int left, int right);

	float GetLeftMagnitude();
	float GetRightMagnitude();

	float GetLeftStickX() { return m_leftX; };
	float GetLeftStickY() { return m_leftY; };

	float GetRightStickX() { return m_rightX; };
	float GetRightStickY() { return m_rightY; };

private:

	XINPUT_STATE m_gamepadState;
	XINPUT_STATE m_gamepadStatePrevious;
	bool m_gamepadConnected;
	float m_leftNormalizedMagnitude;
	float m_rightNormalizedMagnitude;

	float m_leftX, m_leftY, m_rightX, m_rightY;

};

