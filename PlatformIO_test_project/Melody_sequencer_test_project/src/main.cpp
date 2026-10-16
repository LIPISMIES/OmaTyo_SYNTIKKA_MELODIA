#include <Arduino.h>

#include "mcp4821.h"
#include "potentiometer.h"
#include "rotary_encoder.h"
#include "sequence.h"

void setup() {
	Serial.begin(57600);

	mcp4821Begin();
	potentiometerBegin();
	sequenceBegin();
	rotaryEncoderBegin();

	Serial.println(F("Ready. Press start/stop to run or stop the sequence."));
}

void loop() {
	const unsigned long now = millis();

	sequenceSetStepIntervalMs(potentiometerUpdate());
	sequenceUpdate(now);
	sequenceApplyEncoderDelta(rotaryEncoderUpdate());
}
