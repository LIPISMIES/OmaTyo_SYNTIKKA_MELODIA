#pragma once

#include <stdint.h>

void clockInputBegin();
bool clockInputUpdate(unsigned long now, uint16_t tempoReading);
bool clockInputIsExternal();
uint16_t clockInputInternalBpm(uint16_t tempoReading);
