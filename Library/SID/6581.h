/*
MOS6581 SID CHIP Library

See SID.h for Change Log
*/

#ifndef _6581__H
#define _6581__H

// Arduino Pinout Configuration
#define SID_ARDUINO_MOS_CS    7
#define SID_ARDUINO_MOS_CLK   9
#define SID_ARDUINO_SPI_RCK   10
#define SID_ARDUINO_SPI_MOSI	11
#define SID_ARDUINO_SPI_CLK   13

//===========================================
// 1 - MOS6581 Memory Mapping
// This is the mapping of the 8 bit registers 
// Source MOS6581 Datasheet
#define SID_FREQ1LO  0
#define SID_FREQ1HI  1
#define SID_PW1LO    2
#define SID_PW1HI    3
#define SID_CTRL1    4
#define SID_AD1      5
#define SID_SR1      6
#define SID_FREQ2LO  7
#define SID_FREQ2HI  8
#define SID_PW2LO    9
#define SID_PW2HI    10
#define SID_CTRL2    11
#define SID_AD2      12
#define SID_SR2      13
#define SID_FREQ3LO  14
#define SID_FREQ3HI  15
#define SID_PW3LO    16
#define SID_PW3HI    17
#define SID_CTRL3    18
#define SID_AD3      19
#define SID_SR3      20
#define SID_FCLO     21
#define SID_FCHI     22
#define SID_RES      23
#define SID_MODE     24

// Some 6581 data mappings
#define SID_GATE     1
#define SID_TRIANGLE 16
#define SID_SAW      32
#define SID_SQUARE   64       // note - pulse width must be set to something!
#define SID_NOISE    128

#endif 
