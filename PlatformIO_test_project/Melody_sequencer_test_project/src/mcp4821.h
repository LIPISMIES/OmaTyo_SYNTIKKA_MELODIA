#pragma once

#include <Arduino.h>

struct Mcp4821WriteResult {
	uint16_t requestedMilliVolts;
	uint16_t dacCode;
	uint16_t commandWord;
};

void mcp4821Begin();
Mcp4821WriteResult mcp4821WriteVoltageMilliVolts(uint16_t milliVolts);
