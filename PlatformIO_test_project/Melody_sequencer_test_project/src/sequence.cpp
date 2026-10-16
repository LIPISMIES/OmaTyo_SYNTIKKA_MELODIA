#include "sequence.h"

#include <Arduino.h>

#include "board_pins.h"
#include "mcp4821.h"

namespace {

constexpr uint8_t STEP_COUNT = 3;
constexpr unsigned long DEBOUNCE_MS = 35;
constexpr unsigned long BUTTON_SCAN_INTERVAL_MS = 10;
constexpr unsigned int CHANNEL_SETTLE_US = 100;

struct ButtonState {
	uint8_t pin;
	bool stableState;
	bool lastReading;
	unsigned long lastChangeMs;
};

ButtonState startButton{BoardPins::START_STOP, HIGH, HIGH, 0};
ButtonState stepButtons[STEP_COUNT] = {
		{BoardPins::STEP_IN, HIGH, HIGH, 0},
		{BoardPins::STEP_IN, HIGH, HIGH, 0},
		{BoardPins::STEP_IN, HIGH, HIGH, 0},
};

bool running = false;
uint8_t currentStep = 0;
uint8_t selectedStep = 0;
int8_t voltageTicks[STEP_COUNT] = {12, 12, 12};
uint16_t stepIntervalMs = 500;
unsigned long lastStepMs = 0;
unsigned long lastButtonScanMs = 0;

bool pressedEvent(ButtonState &button, unsigned long now) {
	const bool reading = digitalRead(button.pin);

	if (reading != button.lastReading) {
		button.lastReading = reading;
		button.lastChangeMs = now;
	}

	if (reading != button.stableState && now - button.lastChangeMs >= DEBOUNCE_MS) {
		button.stableState = reading;
		return reading == LOW;
	}

	return false;
}

void selectLedChannel(uint8_t channel) {
	digitalWrite(BoardPins::DEMUX_A, channel & 0x01);
	digitalWrite(BoardPins::DEMUX_B, (channel >> 1) & 0x01);
	digitalWrite(BoardPins::DEMUX_C, (channel >> 2) & 0x01);
}

uint16_t stepVoltageMilliVolts(uint8_t step) {
	return 1000 + (static_cast<uint16_t>(voltageTicks[step]) * 1000 + 6) / 12;
}

void outputStepVoltage(uint8_t step) {
	const uint16_t milliVolts = stepVoltageMilliVolts(step);
	const Mcp4821WriteResult result = mcp4821WriteVoltageMilliVolts(milliVolts);
	Serial.print(F("[DAC] Step "));
	Serial.print(step + 1);
	Serial.print(F(": target "));
	Serial.print(static_cast<float>(milliVolts) / 1000.0f, 3);
	Serial.print(F(" V, code "));
	Serial.print(result.dacCode);
	Serial.print(F(", command 0x"));
	Serial.println(result.commandWord, HEX);
}

void scanStepButtons(unsigned long now) {
	for (uint8_t step = 0; step < STEP_COUNT; ++step) {
		selectLedChannel(step);
		delayMicroseconds(CHANNEL_SETTLE_US);

		if (pressedEvent(stepButtons[step], now)) {
			selectedStep = step;
			if (!running) {
				currentStep = step;
				outputStepVoltage(currentStep);
			}
			Serial.print(F("Step "));
			Serial.print(step + 1);
			Serial.print(F(" pressed (LED channel Y"));
			Serial.print(step);
			Serial.println(F(")"));
			Serial.print(F("Step "));
			Serial.print(step + 1);
			Serial.println(F(" selected for voltage editing"));
		}
	}

	selectLedChannel(currentStep);
}

} // namespace

void sequenceBegin() {
	pinMode(BoardPins::DEMUX_A, OUTPUT);
	pinMode(BoardPins::DEMUX_B, OUTPUT);
	pinMode(BoardPins::DEMUX_C, OUTPUT);
	pinMode(BoardPins::GATE_OUT, OUTPUT);
	digitalWrite(BoardPins::GATE_OUT, LOW);
	pinMode(BoardPins::CLOCK_IN, INPUT);
	pinMode(BoardPins::ENCODER_SWITCH, INPUT_PULLUP);
	pinMode(BoardPins::START_STOP, INPUT_PULLUP);
	pinMode(BoardPins::STEP_IN, INPUT_PULLUP);
	selectLedChannel(currentStep);
	outputStepVoltage(currentStep);
}

void sequenceUpdate(unsigned long now) {
	if (pressedEvent(startButton, now)) {
		running = !running;
		if (running) {
			lastStepMs = now;
			selectLedChannel(currentStep);
			outputStepVoltage(currentStep);
			Serial.println("Sequence started");
		} else {
			Serial.println("Sequence stopped");
		}
	}

	if (now - lastButtonScanMs >= BUTTON_SCAN_INTERVAL_MS) {
		lastButtonScanMs = now;
		scanStepButtons(now);
	}

	if (running && now - lastStepMs >= stepIntervalMs) {
		currentStep = (currentStep + 1) % STEP_COUNT;
		selectLedChannel(currentStep);
		outputStepVoltage(currentStep);
		lastStepMs = now;
	}
}

void sequenceApplyEncoderDelta(int8_t delta) {
	if (delta == 0) {
		return;
	}

	const int16_t newTicks = voltageTicks[selectedStep] + delta;
	if (newTicks < 0 || newTicks > 24) {
		return;
	}

	voltageTicks[selectedStep] = static_cast<int8_t>(newTicks);
	Serial.print(F("[EDIT] Step "));
	Serial.print(selectedStep + 1);
	Serial.print(F(" set to "));
	Serial.print(
			static_cast<float>(stepVoltageMilliVolts(selectedStep)) / 1000.0f, 3);
	Serial.println(F(" V"));

	if (!running || selectedStep == currentStep) {
		outputStepVoltage(selectedStep);
	}
}

void sequenceSetStepIntervalMs(uint16_t intervalMs) {
	stepIntervalMs = intervalMs;
}
