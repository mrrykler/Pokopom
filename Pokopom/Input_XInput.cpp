/*  Pokopom - Input Plugin for PSX/PS2 Emulators
 *  Copyright (C) 2012  KrossX
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "General.h"
#include "../../Common/SCPExtensions.cpp"
#include "Input.h"
#include "Input_Shared.h"

#ifdef _WIN32

//#include "Controller.h"
//#include "General.h"
#include "nullDC_Devices.h"
//#include "Chankast.h"
//#include "Zilmar_Devices.h"

#include <XInput.h>
#pragma comment(lib, "Xinput.lib")

namespace Input
{

////////////////////////////////////////////////////////////////////////
// General
////////////////////////////////////////////////////////////////////////

bool FASTCALL Recheck(u8 port)
{
	XINPUT_STATE state;
	DWORD result = XInputGetState(port, &state);

	return (result == ERROR_SUCCESS);
}

void FASTCALL Pause(bool pewpew) { XInputEnable(!pewpew); }

void FASTCALL StopRumble(u8 port, bool &gamepadPlugged)
{
	XINPUT_STATE state;
	DWORD result = XInputGetState(port, &state);

	if(result != ERROR_SUCCESS)
	{
		gamepadPlugged = false;
		return;
	}

	XINPUT_VIBRATION vib;

	vib.wLeftMotorSpeed = 0;
	vib.wRightMotorSpeed = 0;

	XInputSetState(port, &vib);
}

bool FASTCALL CheckAnalogToggle(u8 port)
{
	return !!(GetAsyncKeyState(0x31 + port) >> 1);
}

void FASTCALL SetAnalogLed(u8 port, bool digital)
{
	bool ledScrollLock = GetKeyState(VK_SCROLL)&0x1;

	if((!digital && !ledScrollLock) || (digital && ledScrollLock))
	{
		keybd_event( VK_SCROLL, 0x45, KEYEVENTF_EXTENDEDKEY, 0 );
		keybd_event( VK_SCROLL, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0 );
	}
}

bool FASTCALL InputGetState(_Pad& pad, _Settings &set)
{
	XINPUT_STATE state;
	DWORD result = XInputGetState(set.xinputPort, &state);

	if(result == ERROR_SUCCESS)
	{
		pad.buttons[X360_DUP] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP);
		pad.buttons[X360_DDOWN] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) >> 1;
		pad.buttons[X360_DLEFT] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) >> 2;
		pad.buttons[X360_DRIGHT] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) >> 3;

		pad.buttons[X360_START] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_START) >> 4;
		pad.buttons[X360_BACK] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) >> 5;

		pad.buttons[X360_LS] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) >> 6;
		pad.buttons[X360_RS] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) >> 7;
		pad.buttons[X360_LB] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) >> 8;
		pad.buttons[X360_RB] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) >> 9;

		pad.buttons[X360_A] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_A) >> 12;
		pad.buttons[X360_B] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_B) >> 13;
		pad.buttons[X360_X] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_X) >> 14;
		pad.buttons[X360_Y] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) >> 15;

		pad.analog[X360_STICKLX] = state.Gamepad.sThumbLX;
		pad.analog[X360_STICKLY] = state.Gamepad.sThumbLY;
		pad.analog[X360_STICKRX] = state.Gamepad.sThumbRX;
		pad.analog[X360_STICKRY] = state.Gamepad.sThumbRY;

		pad.analog[X360_TRIGGERL] = state.Gamepad.bLeftTrigger;
		pad.analog[X360_TRIGGERR] = state.Gamepad.bRightTrigger;

		pad.stickL.X = pad.analog[X360_STICKLX];
		pad.stickL.Y = pad.analog[X360_STICKLY];
		pad.stickR.X = pad.analog[X360_STICKRX];
		pad.stickR.Y = pad.analog[X360_STICKRY];

		set.axisValue[GP_AXIS_LY] = pad.analog[X360_STICKLY] * (set.axisInverted[GP_AXIS_LY] ? -1 : 1);
		set.axisValue[GP_AXIS_LX] = pad.analog[X360_STICKLX] * (set.axisInverted[GP_AXIS_LX] ? -1 : 1);
		set.axisValue[GP_AXIS_RY] = pad.analog[X360_STICKRY] * (set.axisInverted[GP_AXIS_RY] ? -1 : 1);
		set.axisValue[GP_AXIS_RX] = pad.analog[X360_STICKRX] * (set.axisInverted[GP_AXIS_RX] ? -1 : 1);

		pad.modL.X = set.axisValue[set.axisRemap[GP_AXIS_LX]];
		pad.modL.Y = set.axisValue[set.axisRemap[GP_AXIS_LY]];
		pad.modR.X = set.axisValue[set.axisRemap[GP_AXIS_RX]];
		pad.modR.Y = set.axisValue[set.axisRemap[GP_AXIS_RY]];

		GetRadius(pad.stickL); GetRadius(pad.stickR);
		GetRadius(pad.modL); GetRadius(pad.modR);
	}
	
	return result == ERROR_SUCCESS;
};

////////////////////////////////////////////////////////////////////////
// DualShock
////////////////////////////////////////////////////////////////////////

void FASTCALL DualshockRumble(u8 smalldata, u8 bigdata, _Settings &set, bool &gamepadPlugged)
{
	XINPUT_STATE state;
	DWORD result = XInputGetState(set.xinputPort, &state);

	if(result == ERROR_SUCCESS)
	{
		//printf("Vibrate! [%X] [%X]\n", smalldata, bigdata);

		static XINPUT_VIBRATION vib;
		static DWORD timerS = 0, timerB = 0;

		if(smalldata)
		{
			vib.wRightMotorSpeed = Clamp(0xFFFF * set.rumble);
			timerS = GetTickCount();
		}
		else if (vib.wRightMotorSpeed && GetTickCount() - timerS > 150)
		{
			vib.wRightMotorSpeed = 0;
		}

		/*
		3.637978807091713*^-11 +
  156.82454281087692 * x + -1.258165252213538 *  x^2 +
  0.006474549734772402 * x^3;
  */

		if(bigdata)
		{
			f64 broom = 0.006474549734772402 * pow(bigdata, 3.0) -
				1.258165252213538 *  pow(bigdata, 2.0) +
				156.82454281087692 * bigdata +
				3.637978807091713e-11;


			/*
			u32 broom = bigdata;

			if(bigdata <= 0x2C) broom *= 0x72;
			else if(bigdata <= 0x53) broom = 0x13C7 + bigdata * 0x24;
			else broom *= 0x205;
			*/

			vib.wLeftMotorSpeed = Clamp(broom * set.rumble);
			timerB = GetTickCount();
		}
		else if (vib.wLeftMotorSpeed && GetTickCount() - timerB > 150)
		{
			vib.wLeftMotorSpeed = 0;
		}

		/*

		vib.wRightMotorSpeed = smalldata == 0? 0 : 0xFFFF;
		vib.wLeftMotorSpeed = bigdata * 0x101;

		vib.wRightMotorSpeed = Clamp(vib.wRightMotorSpeed * settings.rumble);
		vib.wLeftMotorSpeed = Clamp(vib.wLeftMotorSpeed * settings.rumble);
		*/

		//printf("Vibrate! [%X] [%X]\n", vib.wLeftMotorSpeed, vib.wRightMotorSpeed);


		XInputSetState(set.xinputPort, &vib);
	}
	else
		gamepadPlugged = false;
}

bool FASTCALL DualshockPressure(u8 * bufferOut, u32 mask, _Settings &set, bool &gamepadPlugged)
{
	SCP_EXTN pressure;

	if(XInputGetExtended(set.xinputPort, &pressure) == ERROR_SUCCESS)
	{
		//Right, left, up, down
		bufferOut[0x00] = (mask & 0x01) ? (u8)(pressure.SCP_RIGHT* 255) : 0x00;
		bufferOut[0x01] = (mask & 0x02) ? (u8)(pressure.SCP_LEFT * 255) : 0x00;
		bufferOut[0x02] = (mask & 0x04) ? (u8)(pressure.SCP_UP   * 255) : 0x00;
		bufferOut[0x03] = (mask & 0x08) ? (u8)(pressure.SCP_DOWN * 255) : 0x00;

		//triangle, circle, cross, square
		bufferOut[0x04] = (mask & 0x10) ? (u8)(pressure.SCP_T * 255) : 0x00;
		bufferOut[0x05] = (mask & 0x20) ? (u8)(pressure.SCP_C * 255) : 0x00;
		bufferOut[0x06] = (mask & 0x40) ? (u8)(pressure.SCP_X * 255) : 0x00;
		bufferOut[0x07] = (mask & 0x80) ? (u8)(pressure.SCP_S * 255) : 0x00;

		//l1, r1, l2, r2
		bufferOut[0x08] = (mask & 0x100) ? (u8)(pressure.SCP_L1 * 255) : 0x00;
		bufferOut[0x09] = (mask & 0x200) ? (u8)(pressure.SCP_R1 * 255) : 0x00;
		bufferOut[0x0A] = (mask & 0x400) ? (u8)(pressure.SCP_L2 * 255) : 0x00;
		bufferOut[0x0B] = (mask & 0x800) ? (u8)(pressure.SCP_R2 * 255) : 0x00;

		return true;
	}
	else
		return false;
}

////////////////////////////////////////////////////////////////////////
// Dreamcast
////////////////////////////////////////////////////////////////////////

void FASTCALL VibrationWatchdog(LPVOID param)
{
	PuruPuruPack::_thread *pochy = (PuruPuruPack::_thread*)param;
	Sleep(pochy->wait);
	StopRumble(pochy->port, pochy->gamepadPlugged);
}

void FASTCALL DreamcastRumble(s16 intensity, bool freqH, bool freqL, LPVOID thread,
	_Settings &set, bool &gamepadPlugged)
{
	XINPUT_STATE state;
	DWORD result = XInputGetState(set.xinputPort, &state);

	if(result != ERROR_SUCCESS)
	{
		gamepadPlugged = false;
		return;
	}

	PuruPuruPack::_thread *th = (PuruPuruPack::_thread*)thread;
	XINPUT_VIBRATION vib;

	u16 uIntensity = intensity < 0 ? -intensity : intensity;

	vib.wLeftMotorSpeed = freqH ? 0 : (WORD)((uIntensity * 9362) * set.rumble);
	vib.wRightMotorSpeed = freqL ? 0 : (WORD)((uIntensity * 8192 + 8190) * set.rumble);

	if(th->hThread)
	{
		TerminateThread(th->hThread, 0);
		CloseHandle(th->hThread);
		th->hThread = NULL;
	}

	th->hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)VibrationWatchdog, thread, 0, NULL);

	XInputSetState(set.xinputPort, &vib);
}

////////////////////////////////////////////////////////////////////////
// Zilmar
////////////////////////////////////////////////////////////////////////

void FASTCALL N64rumbleSwitch(u8 port, bool &rumble, bool &gamepadPlugged)
{
	XINPUT_STATE state;
	DWORD result = XInputGetState(port, &state);

	static bool pressed[4] = {false};

	if(result == ERROR_SUCCESS)
	{
		if(!pressed[port] && (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK))
		{
			pressed[port] = true;
			rumble = !rumble;
		}
		else if(pressed[port] && !(state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK))
		{
			pressed[port] = false;
		}
	}
	else
		gamepadPlugged = false;

	if(gamepadPlugged)
	{
		bool ledScrollLock = GetKeyState(VK_SCROLL)&0x1;

		if((!rumble && !ledScrollLock) || (rumble && ledScrollLock))
		{
			keybd_event( VK_SCROLL, 0x45, KEYEVENTF_EXTENDEDKEY, 0 );
			keybd_event( VK_SCROLL, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0 );
		}
	}
}



void FASTCALL N64rumble(bool on, _Settings &set, bool &gamepadPlugged)
{
	XINPUT_STATE state;
	DWORD result = XInputGetState(set.xinputPort, &state);

	if(result == ERROR_SUCCESS)
	{
		static XINPUT_VIBRATION vib;

		if(on)
		{
			vib.wRightMotorSpeed = Clamp(0xFFFF * set.rumble);
			vib.wLeftMotorSpeed = Clamp(0xFFFF * set.rumble);
		}
		else
		{
			vib.wRightMotorSpeed = 0;
			vib.wLeftMotorSpeed = 0;
		}

		XInputSetState(set.xinputPort, &vib);
	}
	else
		gamepadPlugged = false;
}

} // End namespace Input
#endif // WIN32
