#pragma once

#include <stdint.h>

void sequenceBegin();
void sequenceUpdate(unsigned long now);
void sequenceApplyEncoderDelta(int8_t delta);
void sequenceSetStepIntervalMs(uint16_t intervalMs);
