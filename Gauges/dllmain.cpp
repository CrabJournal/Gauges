// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "stdafx.h"

#include "FromGame.h"
#include "./arduino_side/common.h"
#include "MemWriter.h"

#ifndef R_memcpy
#define R_memcpy(DST, SRC, _SIZE)  __movsb((BYTE*)(DST), (BYTE*)(SRC), _SIZE)
#endif

HANDLE hPort = INVALID_HANDLE_VALUE;
inline BOOL GaugesWrite(Out *out);

void AttachGauges() {
	hPort = CreateFileA("COM3", GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
	if (hPort != INVALID_HANDLE_VALUE) {
		DCB dcb;
		SecureZeroMemory(&dcb, sizeof(DCB));
		dcb.DCBlength = sizeof(dcb);
		if (GetCommState(hPort, &dcb)) {
			dcb.BaudRate = FREQ;
			dcb.ByteSize = 8;
			dcb.Parity = NOPARITY;
			dcb.StopBits = ONESTOPBIT;
			SetCommState(hPort, &dcb);
		}
	}
}

void ClosePort() {
	if (hPort != INVALID_HANDLE_VALUE) {
		CloseHandle(hPort);
		hPort = INVALID_HANDLE_VALUE;
	}
}

void DetachGauges() {
	if (hPort != INVALID_HANDLE_VALUE) {
		Out out = { SYNC_CODE, 180, 180, LED_CODE::OFF };
		GaugesWrite(&out);
		CloseHandle(hPort);
		hPort = INVALID_HANDLE_VALUE;
	}
}

inline BOOL GaugesWrite(Out *out) {
	DWORD written;
	return WriteFile(hPort, out, sizeof(*out), &written, 0);
}

#define MAX_RPM 10000
#define MAX_SPEED_KMPH 360

T_World_DoTimestep World_DoTimestep_backup;

void _stdcall Hook(World* world, float timeElapsed) {
	World_DoTimestep_backup(world, timeElapsed);

	Out out;
	out.synch = SYNC_CODE;
	float speed_kmph;
	float rpm;

	Car* car = (*pCurrentWorld)->Cars[0];
	float speed_mps = car->state.speed_mps_R;

	rpm = car->state.RPM_R;
	speed_kmph = speed_mps * 3.6;
	out.led = LED_CODE::OFF;
	if (car->MovmentMode == 1 ) {
		int gear = car->state.GearCode_R;
		if (gear == 0) {
			if (rpm > *(float*)((BYTE*)car->unk0x20 + 0x1F4))
				out.led = LED_CODE::RED;
		}
		else {
			float mb_red_zone1 = car->mover->driveTrain->mbRedZoneStarts[gear];
			float mbBlueZoneWide1 = mbRPM_BlueZoneWides[gear];
			float unk1 = *(float*)0x6FFF58; // .data = 0.95
			float tmp = mbBlueZoneWide1 * (unk1);
			float mb_green_zone = mb_red_zone1 - (tmp * 0.5f);
			//	float mb_blue_zone = mb_green_zone - 1500.0f; mb start of side blue zone

			if (rpm > mb_red_zone1 + (1 - unk1) * mbBlueZoneWide1) {
				out.led = LED_CODE::RED;
			}
			else if (rpm > mb_green_zone) {
				out.led = LED_CODE::GREEN;
			}
			else if (rpm > mb_red_zone1 - tmp) {
				out.led = LED_CODE::ORANGE;
			}
		}
	}


	if (speed_kmph > MAX_SPEED_KMPH)
		speed_kmph = MAX_SPEED_KMPH;
	if (rpm > MAX_RPM)
		rpm = MAX_RPM;
	out.speed_angle = ((float)MAX_SPEED_KMPH - speed_kmph) / ((float)MAX_SPEED_KMPH / (float)180);
	out.rev_angle = ((float)MAX_RPM - rpm) / ((float)MAX_RPM / (float)180);

	if (hPort != INVALID_HANDLE_VALUE) {
		if (!GaugesWrite(&out)) {
			ClosePort();
		}
	}
	else {
		AttachGauges();
	}
}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		World_DoTimestep_backup = (T_World_DoTimestep)WriteProtectedMemEIPRelativeAdderess_ReAddr((int*)call_World_DoTimestep, Hook);
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		DetachGauges();
	}
	return TRUE;
}