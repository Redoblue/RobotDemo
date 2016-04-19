#include "stdafx.h"
#include "serialframe.h"
#include "robotcontroller.h"

#include <iostream>
#include <windows.h>

#define PI 3.14f
#define ELAPSE 100
#define FILTER_THRESHOLD 3000
//stick control
#define STICK_MID 32767
#define STICK_MAX 65535
//stick button
#define JOY_A		4
#define	JOY_B		2
#define	JOY_X		8
#define	JOY_Y		1
#define JOY_LT		64
#define JOY_RT		128
#define JOY_LB		16
#define JOY_RB		32
#define JOY_BACK	256
#define JOY_START	512

using namespace std;

VOID CALLBACK TimerProc(HWND, UINT, UINT, DWORD);
int OpenPort();
void ClosePort();
void OpenJoystick();
void CloseJoystick();
void SetWalkFrame(const float speed, const float directionAngle, const float rotationSpeed);

UINT g_serialPort = 0;
JOYINFOEX g_joyInfo;

RobotController g_controller;
WalkFrame g_walkFrame;
LiftFrame g_liftFrame;

int _tmain(int argc, _TCHAR* argv[])
{
init_port:
	cout << "Serial Port: ";
	cin >> g_serialPort;
	if (OpenPort() != 0)
	{
		goto init_port;
	}

	OpenJoystick();
	
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (msg.message == WM_TIMER)
		{
			DispatchMessage(&msg);
		}
	}

	/*g_walkFrame.m_lineSpeed = 0.2f;
	g_walkFrame.m_directionAngle = 3*PI/4;
	g_walkFrame.m_angularSpeed = 0.0f;
	
	g_controller.Walk(g_walkFrame);*/

	return 0;
}

VOID CALLBACK TimerProc(HWND hWnd, UINT nMsg, UINT nTimerId, DWORD dwTime)
{
	cout << "in" << endl;
	int state = JOYERR_UNPLUGGED;
	state = joyGetPosEx(JOYSTICKID1, &g_joyInfo);

	if (state != JOYERR_NOERROR)
	{
		KillTimer(0, 0);
		LPWSTR a = new TCHAR[5];
		wsprintf(a, L"%d", state);
		cout << "JoyStick error." << endl;
	}

	int which = 0;

	signed long dx = g_joyInfo.dwXpos - STICK_MID;
	signed long dy = g_joyInfo.dwYpos - STICK_MID;

	cout << "dx: " << dx << endl;
	cout << "dy: " << dy << endl;

	g_walkFrame.m_lineSpeed = 0.0f;
	g_walkFrame.m_angularSpeed = 0.0f;
	g_walkFrame.m_directionAngle = PI / 2;

	if (abs(dx) <= FILTER_THRESHOLD && abs(dy) <= FILTER_THRESHOLD)		//filter central disfusion
	{
		g_controller.StopWalk();
	}
	else
	{
		which = 1;

		g_walkFrame.m_lineSpeed = 0.5f * sqrt(pow(dx, 2) + pow(dy, 2)) / STICK_MID;
		g_walkFrame.m_directionAngle = (dx < 0) ? (PI + atan(-dy / dx)) : atan(-dy / dx);
	}

	switch (g_joyInfo.dwButtons)
	{
	case JOY_A:
		OpenPort();
		Sleep(100);
		break;
	case JOY_B:
		ClosePort();
		Sleep(100);
		break;
	case JOY_X:
		OpenJoystick();
		Sleep(100);
		break;
	case JOY_Y:
		CloseJoystick();
		Sleep(100);
		break;
	case JOY_LB:
		g_walkFrame.m_angularSpeed = RobotController::MIDDLE_ROTATE_SPEED;// 这个需要设置为归一化的角速度
		which = 1;
		break;
	case JOY_RB:
		g_walkFrame.m_angularSpeed = -RobotController::MIDDLE_ROTATE_SPEED;
		which = 1;
		break;
	case JOY_LT:
		g_liftFrame.m_lineSpeed = RobotController::MIDDLE_LIFT_SPEED;
		which = 2;
		break;
	case JOY_RT:
		g_liftFrame.m_lineSpeed = RobotController::MIDDLE_LIFT_SPEED;
		which = 2;
		break;
	default:
		break;
	}

	if (which == 1)
	{
		g_controller.Walk(g_walkFrame);
	}
	if (which == 2)
	{
		g_controller.Lift(g_liftFrame);
	}
	state = 0;
}

int OpenPort()
{
	if (g_controller.InitPort(g_serialPort) == 0)
	{
		cout << "Port connected." << endl;
		return 0;
	}
	else {
		cout << "Connection failed." << endl;
		return 1;
	}
}

void ClosePort()
{
	g_controller.ClosePort();
	KillTimer(0, 0);
	cout << "Port disconnected." << endl;
}

void OpenJoystick()
{
	g_joyInfo.dwFlags = JOY_RETURNALL;
	g_joyInfo.dwSize = sizeof(JOYINFOEX);
	g_joyInfo.dwXpos = 0;
	g_joyInfo.dwYpos = 0;
	SetTimer(0, 0, ELAPSE, TimerProc);
	cout << "Joystick connected." << endl;
}

void CloseJoystick()
{
	g_controller.StopWalk();
	g_controller.StopLift();
	g_controller.StopRotate();
	KillTimer(0, 0);
	cout << "Joystick disconnected." << endl;
}
