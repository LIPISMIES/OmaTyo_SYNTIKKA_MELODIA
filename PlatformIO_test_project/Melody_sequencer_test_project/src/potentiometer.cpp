#include "potentiometer.h"

#include <Arduino.h>

#include "board_pins.h"

namespace {

constexpr uint16_t ADC_MAX = 1023;
constexpr uint16_t MIN_INTERVAL_MS = 50;
constexpr uint16_t MAX_INTERVAL_MS = 10000;
constexpr uint16_t INTERVAL_INCREMENT_MS = 50;
constexpr uint16_t ADC_HYSTERESIS_COUNTS = 3;
constexpr unsigned long READ_INTERVAL_MS = 50;
constexpr uint16_t INTERVAL_COUNT =
		(MAX_INTERVAL_MS - MIN_INTERVAL_MS) / INTERVAL_INCREMENT_MS;

uint16_t currentIntervalMs = 500;
uint16_t currentIntervalIndex = 0;
unsigned long lastReadMs = 0;
bool initialized = false;

uint16_t intervalIndexFromReading(uint16_t reading) {
	const uint32_t scaled =
			(static_cast<uint32_t>(reading) * INTERVAL_COUNT + ADC_MAX / 2) / ADC_MAX;
	return scaled > INTERVAL_COUNT ? INTERVAL_COUNT : static_cast<uint16_t>(scaled);
}

uint16_t boundaryForTransition(uint16_t lowerIndex) {
	const uint32_t numerator =
			(static_cast<uint32_t>(lowerIndex) * 2 + 1) * ADC_MAX;
	const uint32_t denominator = static_cast<uint32_t>(INTERVAL_COUNT) * 2;
	return static_cast<uint16_t>((numerator + denominator / 2) / denominator);
}

void updateIntervalFromReading(uint16_t reading) {
	while (currentIntervalIndex < INTERVAL_COUNT) {
		const uint16_t boundary = boundaryForTransition(currentIntervalIndex);
		const uint16_t threshold =
				boundary + ADC_HYSTERESIS_COUNTS > ADC_MAX
						? ADC_MAX
						: boundary + ADC_HYSTERESIS_COUNTS;
		if (reading < threshold) {
			break;
		}
		++currentIntervalIndex;
	}

	while (currentIntervalIndex > 0) {
		const uint16_t boundary = boundaryForTransition(currentIntervalIndex - 1);
		const uint16_t threshold = boundary > ADC_HYSTERESIS_COUNTS
				? boundary - ADC_HYSTERESIS_COUNTS
				: 0;
		if (reading > threshold) {
			break;
		}
		--currentIntervalIndex;
	}

	currentIntervalMs =
			MIN_INTERVAL_MS + currentIntervalIndex * INTERVAL_INCREMENT_MS;
}

} // namespace

void potentiometerBegin() {
	pinMode(BoardPins::TEMPO, INPUT);

	currentIntervalIndex = intervalIndexFromReading(analogRead(BoardPins::TEMPO));
	currentIntervalMs =
			MIN_INTERVAL_MS + currentIntervalIndex * INTERVAL_INCREMENT_MS;
	lastReadMs = millis();
	initialized = true;
	Serial.print(F("[TIMING] Step interval: "));
	Serial.print(currentIntervalMs);
	Serial.println(F(" ms"));
}

uint16_t potentiometerUpdate() {
	const unsigned long now = millis();
	if (!initialized || now - lastReadMs < READ_INTERVAL_MS) {
		return currentIntervalMs;
	}

	lastReadMs = now;
	const uint16_t previousIntervalMs = currentIntervalMs;
	updateIntervalFromReading(analogRead(BoardPins::TEMPO));
	if (currentIntervalMs != previousIntervalMs) {
		Serial.print(F("[TIMING] Step interval: "));
		Serial.print(currentIntervalMs);
		Serial.println(F(" ms"));
	}

	return currentIntervalMs;
}
