#include "mcp4821.h"

#include <SPI.h>

#include "board_pins.h"

namespace {

constexpr uint32_t DAC_SPI_FREQUENCY = 4000000;
constexpr uint16_t DAC_ACTIVE_2X_GAIN = 0x1000;
constexpr uint16_t DAC_GAIN_DIVISOR = 2;
constexpr uint16_t DAC_REFERENCE_MILLIVOLTS = 2048;
constexpr uint16_t DAC_FULL_SCALE_COUNTS = 4096;
constexpr uint16_t DAC_MAX_CODE = 0x0FFF;

} // namespace

void mcp4821Begin() {
	pinMode(BoardPins::DAC_CS, OUTPUT);
	digitalWrite(BoardPins::DAC_CS, HIGH);
	SPI.begin();
}

Mcp4821WriteResult mcp4821WriteVoltageMilliVolts(uint16_t milliVolts) {
	const uint32_t denominator = DAC_REFERENCE_MILLIVOLTS * DAC_GAIN_DIVISOR;
	uint32_t calculatedCode =
			(static_cast<uint32_t>(milliVolts) * DAC_FULL_SCALE_COUNTS + denominator / 2)
			/ denominator;
	if (calculatedCode > DAC_MAX_CODE) {
		calculatedCode = DAC_MAX_CODE;
	}

	const uint16_t dacCode = static_cast<uint16_t>(calculatedCode);
	const uint16_t commandWord = DAC_ACTIVE_2X_GAIN | dacCode;

	SPI.beginTransaction(SPISettings(DAC_SPI_FREQUENCY, MSBFIRST, SPI_MODE0));
	digitalWrite(BoardPins::DAC_CS, LOW);
	SPI.transfer16(commandWord);
	digitalWrite(BoardPins::DAC_CS, HIGH);
	SPI.endTransaction();

	return {milliVolts, dacCode, commandWord};
}
