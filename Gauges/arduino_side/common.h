#pragma once

#include <stdint.h>
#define FREQ 9600

#define SYNC_CODE 0xFF

enum LED_CODE : uint8_t {
	OFF = 0,
	GREEN = 1,
	ORANGE = 2,
	RED = 3
};

#pragma pack(push, 1)
struct Out {
	uint8_t synch;
	uint8_t speed_angle;
	uint8_t rev_angle;
	LED_CODE led;
};
#pragma pack(pop)