#include "gate_output.h"

#include <Arduino.h>

#include "board_pins.h"

namespace {

constexpr unsigned long GATE_PULSE_WIDTH_MS = 10;

unsigned long gatePulseStartMs = 0;
bool gatePulseActive = false;

} // namespace

void gateOutputBegin() {
	pinMode(BoardPins::GATE_OUT, OUTPUT);
	digitalWrite(BoardPins::GATE_OUT, LOW);
}

void gateOutputTrigger(unsigned long now) {
	gatePulseStartMs = now;
	gatePulseActive = true;
	digitalWrite(BoardPins::GATE_OUT, HIGH);
}

void gateOutputUpdate(unsigned long now) {
	if (gatePulseActive && now - gatePulseStartMs >= GATE_PULSE_WIDTH_MS) {
		digitalWrite(BoardPins::GATE_OUT, LOW);
		gatePulseActive = false;
	}
}
