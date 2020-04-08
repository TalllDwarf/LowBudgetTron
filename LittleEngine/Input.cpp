#include "stdafx.h"
#include "Input.h"


Input::Input() : m_nMouseXChange(0), m_nMouseYChange(0), m_nMouseX(0), m_nMouseY(0), m_bLMB(0), m_bRMB(0)
{
	m_gamepadConnected = false;
}


Input::~Input()
{
}

INT Input::RegisterDevices()
{
	//Mouse
	m_Rid[0].usUsagePage = 0x01;
	m_Rid[0].usUsage = 0x02;
	m_Rid[0].dwFlags = 0x00;
	m_Rid[0].hwndTarget = 0;

	//Keyboard
	m_Rid[1].usUsagePage = 0x01;
	m_Rid[1].usUsage = 0x06;
	m_Rid[1].dwFlags = 0x00;
	m_Rid[1].hwndTarget = 0;

	ResetAllKeyData();

	if (RegisterRawInputDevices(m_Rid, 2, sizeof(RAWINPUTDEVICE)) == false)
		return 1;

	return 0;
}

void Input::GetData(LPARAM lParam)
{
	for (int alph = 0; alph < 25; ++alph)
		m_baOldAlphabet[alph] = m_baAlphabet[alph];

	// Determine how big the buffer should be
	UINT bufferSize;
	GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &bufferSize, sizeof(RAWINPUTHEADER));

	// Create a buffer of the correct size - but see note below

	// Call the function again, this time with the buffer to get the data
	if (bufferSize <= 48)
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, (LPVOID)m_buffer, &bufferSize, sizeof(RAWINPUTHEADER));

	RAWINPUT *raw = (RAWINPUT*)m_buffer;

	// The mouse has not been tested extensively,
	// but I believe it works.
	if (raw->header.dwType == RIM_TYPEMOUSE)
	{
		m_oldbLMB = m_bLMB;
		m_oldbRMB = m_bRMB;

		// Get values from the mouse member (of type RAWMOUSE)
		m_nMouseXChange = raw->data.mouse.lLastX;
		m_nMouseYChange = raw->data.mouse.lLastY;

		bool bStateDown = raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN;
		bool bStateUp = raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP;

		if (bStateDown == true && bStateUp == false)
		{
			m_bLMB = true;
		}

		if (bStateUp == true)
		{
			m_bLMB = false;
		}


		bool bStateDownTwo = raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN;
		bool bStateUpTwo = raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP;

		if (bStateDownTwo == true && bStateUpTwo == false)
		{
			m_bRMB = true;
		}

		if (bStateUpTwo == true)
		{
			m_bRMB = false;
		}


	}

	if (raw->header.dwType == RIM_TYPEKEYBOARD)
	{
		// Get key value from the keyboard member (of type RAWKEYBOARD)
		USHORT keyCode = raw->data.keyboard.VKey;
		bool keyUp = raw->data.keyboard.Flags & RI_KEY_BREAK;

		// Ok because I put the most effort into this section of my code,
		// I will briefly explain.

		// We need a boolean pointer to start.
		bool* pbToKey = NULL;

		// The keycode can be one of many possibilities,
		// which is why a switch case is to exhaustive to
		// use and I believe for the most part switch case 
		// would not be efficient.

		// So instead of manually typing every possible
		// case value, we can just start by looping through
		// an expected range of keys the keycode might be.

		for (int iii = 0; iii < 25; ++iii)
		{
			// We add the hex-code 0x41 because that is the
			// value MSDN lists for virtual keycode 'A'.
			if (keyCode == iii + 0x41)
			{
				// However our alphabet or array of booleans that
				// represent it, begins at 0 and is only 25 in length.
				// So iii itself is the appropritate index.
				pbToKey = &m_baAlphabet[iii];

				// At this point we have assigned our boolean pointer variable
				// a new address which is whatever index iii would be accessing.
				// We break because there is no need to go further since we found the
				// matching keycode!
				break;
			}
		}

		// However, if the whole for loop above did not find any matches
		// to the keycode, then we can expect our boolean pointer to be the same
		// value we initalized it to. Which would be NULL.

		if (pbToKey != NULL)
		{
			// if we entered this block of code, then the boolean pointer
			// is not NULL and has been assigned. The main idea is to
			// call the CheckKeyPress function to update the boolean pointed to
			// by the pbToKey, as well as pass in that boolean value itself.
			*pbToKey = CheckKeyPress(*pbToKey, keyUp);

			// Be sure to return ASAP!
			return;
		}

		// If the boolean pointer was null, then before giving up
		// we can check if the keycode falls in the arrow key range!

		for (int iii = 0; iii < 4; ++iii)
		{
			// Notice we add 0x25 to iii,
			// just like we added 0x41 to iii in the last for loop.

			if (keyCode == iii + 0x25)
			{
				// Yay found it!
				pbToKey = &m_baArrows[iii];
				break;
			}
		}

		//
		//Added check shift key
		//
		if (keyCode == VK_SHIFT)
		{
			pbToKey = &m_bShift;
		}

		if (keyCode == VK_RETURN)
		{
			pbToKey = &m_bEnter;
		}

		// Because the keycode may not match any in the ranges we
		// so far, we check to see if the boolean
		// pointer has been assigned anything other than NULL. If 
		// it is NULL then we do not want to call CheckKeyPress.
		// The reason for that is, it would error because our pointer
		// is invalid and CheckKeyPress would be fail.
		if (pbToKey != NULL)
		{

			*pbToKey = CheckKeyPress(*pbToKey, keyUp);

			// Be sure to return ASAP!
			return;
		}

		// TODO: Add more key ranges to check for! Just follow
		// how I have set things up so far.

	}
}

void Input::ResetAllKeyData()
{
	for (int alph = 0; alph < 25; ++alph)
	{
		m_baOldAlphabet[alph] = m_baAlphabet[alph];
		m_baAlphabet[alph] = 0;
	}

	for (int arrow = 0; arrow < 4; ++arrow)
		m_baArrows[arrow] = 0;

	m_bShift = 0;
}

float Input::Forward(float deltaTime)
{
	if (m_gamepadConnected)
	{
		float leftValue = (m_leftY * m_leftNormalizedMagnitude) * deltaTime;

		if (leftValue != 0)
			return leftValue;
	}

	if (IsAlphabeticKeyDown(VKey_W))
	{
		return (1.0f * deltaTime);
	}
	else if (IsAlphabeticKeyDown(VKey_S))
	{
		return (-1.0f * deltaTime);
	}

	return 0.0f;
}

float Input::Right(float deltaTime)
{
	if (m_gamepadConnected)
	{
		float leftValue = (m_leftX * m_leftNormalizedMagnitude) * deltaTime;

		if (leftValue != 0)
			return leftValue;
	}

	if (IsAlphabeticKeyDown(VKey_D))
	{
		return (1.0f * deltaTime);
	}
	else if (IsAlphabeticKeyDown(VKey_A))
	{
		return (-1.0f * deltaTime);
	}

	return 0.0f;
}

float Input::Up(float deltaTime)
{
	if (m_gamepadConnected)
	{
		if (IsButtonDown(XINPUT_GAMEPAD_A))
		{
			return (1.0f * deltaTime);
		}
		else if (IsButtonDown(XINPUT_GAMEPAD_B))
		{
			return (-1.0f * deltaTime);
		}
	}

	if (IsAlphabeticKeyDown(VKey_Q))
	{
		return (1.0f * deltaTime);
	}
	else if (IsAlphabeticKeyDown(VKey_E))
	{
		return (-1.0f * deltaTime);
	}

	return 0.0f;
}

float Input::GetRightChangeX(float deltaTime)
{
	if (m_gamepadConnected)
	{
		float rightValue = (m_rightX * m_rightNormalizedMagnitude) * 50;

		if (rightValue != 0)
			return rightValue;
	}

	return GetMouseChangeX();
}

float Input::GetRightChangeY(float deltaTime)
{
	if (m_gamepadConnected)
	{
		float rightValue = ((m_rightY * m_rightNormalizedMagnitude) * deltaTime) * 80;

		if (rightValue != 0)
			return -rightValue;
	}

	return GetMouseChangeY();
}

bool Input::CheckKeyPress(bool bLastKeyState, bool bThisKeyState)
{
	// The following may be overkill, but just know the value returned
	// is based upon the current state of the boolean in question, and the
	// new state we have processed.
	if (bThisKeyState == false)
	{
		if (bLastKeyState == true)
		{
			return true;
		}
		else
		{
			return true;
		}
	}
	else
	{
		if (bLastKeyState == false)
		{
			return false;
		}
		else
			return false;
	}
}

//
//
//Gamepad Input
//
//

#define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  7849
#define XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 8689
#define XINPUT_GAMEPAD_TRIGGER_THRESHOLD    30

void Input::Update()
{
	m_gamepadStatePrevious = m_gamepadState;

	ZeroMemory(&this->m_gamepadState, sizeof(XINPUT_STATE));

	DWORD dwResult = XInputGetState(0, &this->m_gamepadState);
	if (dwResult == ERROR_SUCCESS)
	{
		m_gamepadConnected = true;
		UpdateLeftAnalog();
		UpdateRightAnalog();
	}
	else
	{
		m_gamepadConnected = false;
	}
}

bool Input::GamepadConnected()
{
	return m_gamepadConnected;
}

bool Input::IsButtonDown(unsigned short int button)
{
	return (m_gamepadState.Gamepad.wButtons & button);
}

bool Input::WasButtonPressDetected(unsigned short int button)
{
	return (!(m_gamepadStatePrevious.Gamepad.wButtons & button) && (m_gamepadState.Gamepad.wButtons & button));
}

bool Input::WasButtonReleaseDetcted(unsigned short int button)
{
	return ((m_gamepadStatePrevious.Gamepad.wButtons & button) && !(m_gamepadState.Gamepad.wButtons & button));
}

void Input::UpdateLeftAnalog()
{
	float LX = m_gamepadState.Gamepad.sThumbLX;
	float LY = m_gamepadState.Gamepad.sThumbLY;

	// determine the magnitude (sum of squares)
	// c^2 = a^2 + b^2
	float magnitude = sqrt(LX*LX + LY * LY);

	m_leftX = LX / magnitude;
	m_leftY = LY / magnitude;

	//Check for nan

	if (m_leftX != m_leftX)
		m_leftX = 0;

	if (m_leftY != m_leftY)
		m_leftY = 0;

	m_leftNormalizedMagnitude = 0;

	//check if the controller is outside a circular dead zone
	if (magnitude > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
	{
		//clip the magnitude at its expected maximum value
		if (magnitude > 32767)
		{
			magnitude = 32767;
		}

		// adjust magnitude relative to the end of the dead zone
		magnitude -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;

		// normalized between 0 and 1
		m_leftNormalizedMagnitude = magnitude / (32767 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

	}
	else //if the controller is in the deadzone zero out the magnitude
	{
		magnitude = 0.0;
		m_leftNormalizedMagnitude = 0.0;
	}
}

void Input::UpdateRightAnalog()
{
	float RX = m_gamepadState.Gamepad.sThumbRX;
	float RY = m_gamepadState.Gamepad.sThumbRY;

	// determine the magnitude (sum of squares)
	// c^2 = a^2 + b^2
	float magnitude = sqrt(RX*RX + RY * RY);

	m_rightX = RX / magnitude;
	m_rightY = RY / magnitude;

	//check for nan
	if (m_rightX != m_rightX)
		m_rightX = 0.0f;

	if (m_rightY != m_rightY)
		m_rightY = 0.0f;

	m_rightNormalizedMagnitude = 0;

	//check if the controller is outside a circular dead zone
	if (magnitude > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
	{
		//clip the magnitude at its expected maximum value
		if (magnitude > 32767)
		{
			magnitude = 32767;
		}

		// adjust magnitude relative to the end of the dead zone
		magnitude -= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;

		// normalized between 0 and 1
		m_rightNormalizedMagnitude = magnitude / (32767 - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

	}
	else //if the controller is in the deadzone zero out the magnitude
	{
		magnitude = 0.0;
		m_rightNormalizedMagnitude = 0.0;
	}
}

void Input::SetVibration(int left, int right)
{
	// Create a Vibraton State
	XINPUT_VIBRATION vibration;

	// Zeroise the Vibration
	ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));

	// Set the Vibration Values
	vibration.wLeftMotorSpeed = left;
	vibration.wRightMotorSpeed = right;


	// Vibrate the controller
	XInputSetState(0, &vibration);
}

float Input::GetLeftMagnitude()
{
	return m_leftNormalizedMagnitude;
}

float Input::GetRightMagnitude()
{
	return m_rightNormalizedMagnitude;
}
