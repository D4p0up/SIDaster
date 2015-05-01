/*
MOS6581 SID CHIP Library

See SID.h for Change Log
*/

#include "Arduino.h"
#include "SID.h"
#include "6581.h"

volatile byte SID_clr;

// 3.1 - 1MHz Clock Setup 
// ----------------------
// 1MHz generation on OC1A - Clk 16 MHz - set pin 9 as OC1A output
// Reset settings of Timer/Counter register 1
// Set compare match output A to toogle
// Set waveform generation mode to CTC (Clear Counter on Match)
// Set clock select to clock/1024 (from prescaler)
// Set output compare register A to 8 (i.e. OC1A Toggle every 7+1=8 Clk pulses)
void SID::clk_setup(void) 
{
  pinMode(SID_ARDUINO_MOS_CLK,OUTPUT);
  TCCR1A &= ~((1<<COM1A1) | (1<<COM1A0) | (1<<WGM11) | (1<<WGM10));
  TCCR1B &= ~((1<<WGM13) | (1<<WGM12) | (1<<CS12) | (1<<CS11) | (1<<CS10));
  TCCR1A |= (0<<COM1A1) | (1<<COM1A0);
  TCCR1A |= (0<<WGM11) | (0<<WGM10);
  TCCR1B |= (0<<WGM13) | (1<<WGM12);
  TCCR1B |= (0<<CS12) | (0<<CS11) | (1<<CS10);
  OCR1A = 7;
}

// 3.2 - MOS SPI Link Setup
// --------------------
// IOs configuration / MOS CS setup
// Fastest rate possible by prescaling
// MISO / CLK in Tx mode only
void SID::SPI_setup(void) 
{
  pinMode(SID_ARDUINO_MOS_CS,OUTPUT);
  digitalWrite(SID_ARDUINO_MOS_CS, HIGH);
  pinMode(SID_ARDUINO_SPI_RCK,OUTPUT);
  digitalWrite(SID_ARDUINO_SPI_RCK, HIGH);
  pinMode(SID_ARDUINO_SPI_MOSI,OUTPUT);
  pinMode(SID_ARDUINO_SPI_CLK,OUTPUT);
  SPCR |= ( (1<<SPE) | (1<<MSTR) ); // enable SPI as master
  SPCR &= ~( (1<<SPR1) | (1<<SPR0) | (1<<DORD) ); // clear prescaler bits
  SPCR |= (1<<DORD);
  SID_clr=SPSR; 
  SID_clr=SPDR; 
  SPSR |= (1<<SPI2X); 
}

// 3.3 - SID chip SPI write
// ------------------------
// Double byte write to Serial SPI out
// First Adress, second Data
// Latch on 74HC595
// Latch on SID
// Delay to allow SID to handle the Data
byte SID::send(byte address, byte data) 
{
  PORTD |= 128;
  SPDR = address;              
  loop_until_bit_is_set(SPSR, SPIF);
  SID_clr=SPDR;
  SPDR = data;              
  loop_until_bit_is_set(SPSR, SPIF);
  SID_clr=SPDR;
  PORTB &= ~(4); 
  PORTB |= 4;
  PORTD &= ~(128); 
  delayMicroseconds(300);
}

