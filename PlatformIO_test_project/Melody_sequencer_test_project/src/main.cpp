#include <Arduino.h>

#include "clock_input.h"
#include "gate_output.h"
#include "mcp4821.h"
#include "potentiometer.h"
#include "rotary_encoder.h"
#include "sequence.h"

void setup() {
	Serial.begin(57600);

	mcp4821Begin();
	potentiometerBegin();
	clockInputBegin();
	gateOutputBegin();
	sequenceBegin();
	rotaryEncoderBegin();

	Serial.println(F("Ready. Press start/stop to run or stop the sequence."));
}

void loop() {
	const unsigned long now = millis();
	const uint16_t tempoReading = potentiometerUpdate();
	const bool externalTick = clockInputUpdate(now, tempoReading);

	sequenceSetTempoBpm(clockInputInternalBpm(tempoReading));
	sequenceUpdate(now, clockInputIsExternal(), externalTick);
	gateOutputUpdate(now);
	sequenceApplyEncoderDelta(rotaryEncoderUpdate());
}
