#pragma once
#include <Arduino.h>

/*

Physical DIP pin   Pin function                     Code pin        Device function

1                  PC6 (PCINT14/RESET)              -               RESET
2                  PD0 (PCINT16/RXD)                0               RXD
3                  PD1 (PCINT17/TXD)                1               TXD
4                  PD2 (PCINT18/INT0)               2               ENC_A
5                  PD3 (PCINT19/OC2B/INT1)          3               CLK_IN
6                  PD4 (PCINT20/XCK/T0)             4               ENC_S
7                  VCC                              -
8                  GND                              -
9                  PB6 (PCINT6/XTAL1/TOSC1)         -               -
10                 PB7 (PCINT7/XTAL2/TOSC2)         -               -
11                 PD5 (PCINT21/OC0B/T1)            5               STEP_IN
12                 PD6 (PCINT22/OC0A/AIN0)          6               DEMUX_A
13                 PD7 (PCINT23/AIN1)               7               DEMUX_B
14                 PB0 (PCINT0/CLKO/ICP1)           8               DEMUX_C
15                 PB1 (OC1A/PCINT1)                9               GATE_OUT
16                 PB2 (SS/OC1B/PCINT2)             10              DAC_CS
17                 PB3 (MOSI/OC2A/PCINT3)           11              SPI_MOSI
18                 PB4 (MISO/PCINT4)                12              SPI_MISO
19                 PB5 (SCK/PCINT5)                 13              SPI_SCK
20                 AVCC                             -
21                 AREF                             -
22                 GND                              -
23                 PC0 (ADC0/PCINT8)                A0              START_STOP
24                 PC1 (ADC1/PCINT9)                A1              TEMPO
25                 PC2 (ADC2/PCINT10)               A2              -
26                 PC3 (ADC3/PCINT11)               A3              ENC_B
27                 PC4 (ADC4/SDA/PCINT12)           A4              SDA
28                 PC5 (ADC5/SCL/PCINT13)           A5              SCL

*/

namespace BoardPins {
    // ========================================================
    // Digital Demux Control Bus & System IO
    // ========================================================
    constexpr uint8_t DEMUX_A = 6;
    constexpr uint8_t DEMUX_B = 7;
    constexpr uint8_t DEMUX_C = 8;

    constexpr uint8_t CLOCK_IN = 3;
    constexpr uint8_t STEP_IN = 5;

    // ========================================================
    // Rotary Encoder Inputs
    // ========================================================
    constexpr uint8_t ENCODER_A = 2;
    constexpr uint8_t ENCODER_SWITCH = 4;
    constexpr uint8_t ENCODER_B = A2;

    // ========================================================
    // Controls & Gate Pulse Input
    // ========================================================
    constexpr uint8_t START_STOP = A0;
    constexpr uint8_t TEMPO = A1;
    constexpr uint8_t GATE_OUT = 9;


    // ========================================================
    // Hardware SPI Interface
    // ========================================================
    constexpr uint8_t DAC_CS = 10;
    constexpr uint8_t DAC_MOSI = 11;
    constexpr uint8_t DAC_SCK = 13;
}
