#include "rotary_encoder.h"

#include <Arduino.h>

#include "board_pins.h"

namespace {

constexpr int8_t DETENT_TRANSITIONS = 4;

// One valid Gray-code cycle corresponds to one EC11E detent.
constexpr int8_t TRANSITION_TABLE[16] = {
		0, 1, -1, 0,
		-1, 0, 0, 1,
		1, 0, 0, -1,
		0, -1, 1, 0,
};

uint8_t previousState = 0;
int8_t transitionCount = 0;

uint8_t readEncoderState() {
	return (digitalRead(BoardPins::ENCODER_A) << 1)
			| digitalRead(BoardPins::ENCODER_B);
}

} // namespace

void rotaryEncoderBegin() {
	pinMode(BoardPins::ENCODER_A, INPUT_PULLUP);
	pinMode(BoardPins::ENCODER_B, INPUT_PULLUP);
	previousState = readEncoderState();
}

int8_t rotaryEncoderUpdate() {
	const uint8_t currentState = readEncoderState();
	const uint8_t oldState = previousState;
	const uint8_t transitionIndex = (oldState << 2) | currentState;
	previousState = currentState;

	const int8_t transition = TRANSITION_TABLE[transitionIndex];
	if (transition == 0) {
		if (currentState != oldState) {
			transitionCount = 0;
		}
		return 0;
	}

	transitionCount += transition;
	if (transitionCount == DETENT_TRANSITIONS || transitionCount == -DETENT_TRANSITIONS) {
		const int8_t direction = transitionCount > 0 ? 1 : -1;
		transitionCount = 0;
		return direction;
	}

	return 0;
}
