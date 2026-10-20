#include "potentiometer.h"

#include <Arduino.h>

#include "board_pins.h"

namespace {

constexpr uint16_t ADC_MAX = 1023;
constexpr uint16_t ADC_HYSTERESIS_COUNTS = 3;
constexpr unsigned long READ_INTERVAL_MS = 50;

uint16_t currentReading = ADC_MAX / 2;
unsigned long lastReadMs = 0;
bool initialized = false;

} // namespace

void potentiometerBegin() {
	pinMode(BoardPins::TEMPO, INPUT);
	currentReading = analogRead(BoardPins::TEMPO);
	lastReadMs = millis();
	initialized = true;
}

uint16_t potentiometerUpdate() {
	const unsigned long now = millis();
	if (!initialized || now - lastReadMs < READ_INTERVAL_MS) {
		return currentReading;
	}

	lastReadMs = now;
	const uint16_t reading = analogRead(BoardPins::TEMPO);
	const uint16_t difference = reading > currentReading
			? reading - currentReading
			: currentReading - reading;
	if (difference >= ADC_HYSTERESIS_COUNTS) {
		currentReading = reading;
	}

	return currentReading;
}
