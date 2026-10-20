#pragma once

#include <stdint.h>

void potentiometerBegin();
// Returns the filtered ADC reading for the tempo control.
uint16_t potentiometerUpdate();
