#pragma once

#include <stdint.h>

void sequenceBegin();
void sequenceUpdate(unsigned long now, bool externalClock, bool externalTick);
void sequenceApplyEncoderDelta(int8_t delta);
void sequenceSetTempoBpm(uint16_t bpm);
