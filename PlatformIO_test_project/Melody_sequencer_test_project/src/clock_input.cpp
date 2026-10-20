#include "clock_input.h"

#include <Arduino.h>

#include "board_pins.h"

namespace {

constexpr uint16_t ADC_MAX = 1023;
constexpr unsigned long MIN_CLOCK_TIMEOUT_MS = 2500;
constexpr uint32_t MIN_PULSE_INTERVAL_US = 1000;

volatile unsigned long lastPulseMicros = 0;
volatile unsigned long pulseIntervalMicros = 0;
volatile unsigned long lastPulseMillis = 0;
volatile uint32_t pulseCount = 0;
volatile bool havePulse = false;

uint32_t processedPulseCount = 0;
unsigned long lastClockMillis = 0;
unsigned long measuredIntervalMs = 0;
uint8_t divisionPhase = 0;
uint8_t pendingSubsteps = 0;
unsigned long nextSubstepMs = 0;
unsigned long substepIntervalMs = 0;
int8_t previousRatio = 0;
bool haveProcessedPulse = false;
bool externalClockActive = false;

void receiveClockPulse() {
	const unsigned long pulseTime = micros();
	if (havePulse) {
		const unsigned long interval = pulseTime - lastPulseMicros;
		if (interval < MIN_PULSE_INTERVAL_US) {
			return;
		}
		pulseIntervalMicros = interval;
	}

	lastPulseMicros = pulseTime;
	lastPulseMillis = millis();
	havePulse = true;
	++pulseCount;
}

int8_t ratioFromReading(uint16_t reading) {
	const uint32_t ratioIndex =
			(static_cast<uint32_t>(reading) * 8 + ADC_MAX / 2) / ADC_MAX;
	return static_cast<int8_t>(ratioIndex) - 4;
}

void scheduleSubsteps(unsigned long now, int8_t ratio) {
	pendingSubsteps = ratio == 0 ? 0 : (1U << ratio) - 1;
	substepIntervalMs = measuredIntervalMs >> ratio;
	if (substepIntervalMs == 0) {
		substepIntervalMs = 1;
	}
	nextSubstepMs = now + substepIntervalMs;
}

unsigned long clockTimeoutMs() {
	const unsigned long timeout = measuredIntervalMs * 3;
	return timeout > MIN_CLOCK_TIMEOUT_MS ? timeout : MIN_CLOCK_TIMEOUT_MS;
}

} // namespace

void clockInputBegin() {
	pinMode(BoardPins::CLOCK_IN, INPUT);
	attachInterrupt(
			digitalPinToInterrupt(BoardPins::CLOCK_IN), receiveClockPulse, RISING);
}

bool clockInputUpdate(unsigned long now, uint16_t tempoReading) {
	unsigned long pulseTimeMs;
	unsigned long intervalUs;
	uint32_t currentPulseCount;
	bool hasReceivedPulse;

	noInterrupts();
	pulseTimeMs = lastPulseMillis;
	intervalUs = pulseIntervalMicros;
	currentPulseCount = pulseCount;
	hasReceivedPulse = havePulse;
	interrupts();

	const bool newPulse = currentPulseCount != processedPulseCount;
	const uint32_t unprocessedPulses = currentPulseCount - processedPulseCount;
	if (newPulse) {
		processedPulseCount = currentPulseCount;
		lastClockMillis = pulseTimeMs;
		if (intervalUs != 0) {
			measuredIntervalMs = (intervalUs + 500) / 1000;
		}
	}

	externalClockActive =
			hasReceivedPulse && now - lastClockMillis <= clockTimeoutMs();
	const int8_t ratio = ratioFromReading(tempoReading);
	bool tick = false;

	if (!externalClockActive) {
		pendingSubsteps = 0;
		divisionPhase = 0;
		previousRatio = ratio;
		haveProcessedPulse = false;
		return false;
	}

	if (ratio != previousRatio) {
		divisionPhase = 0;
		pendingSubsteps = 0;
		previousRatio = ratio;
	}

	if (newPulse) {
		if (!haveProcessedPulse) {
			tick = true;
			divisionPhase = 0;
			haveProcessedPulse = true;
			if (ratio >= 0 && measuredIntervalMs > 0) {
				scheduleSubsteps(now, ratio);
			}
		} else if (ratio < 0) {
			const uint8_t division = 1U << -ratio;
			tick = unprocessedPulses
					>= static_cast<uint32_t>(division - divisionPhase);
			divisionPhase =
					(divisionPhase + unprocessedPulses % division) % division;
		} else {
			tick = true;
			scheduleSubsteps(now, ratio);
		}
	} else if (pendingSubsteps > 0
			&& static_cast<int32_t>(now - nextSubstepMs) >= 0) {
		tick = true;
		--pendingSubsteps;
		nextSubstepMs += substepIntervalMs;
	}

	return tick;
}

bool clockInputIsExternal() {
	return externalClockActive;
}

uint16_t clockInputInternalBpm(uint16_t tempoReading) {
	const uint32_t bpm =
			50 + (static_cast<uint32_t>(tempoReading) * 450 + ADC_MAX / 2) / ADC_MAX;
	return static_cast<uint16_t>(bpm);
}
