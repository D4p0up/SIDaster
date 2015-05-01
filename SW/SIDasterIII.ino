//==================================
// SIDaster Beta 1
// 25/09/2011
// http://bit.ly/fullmaj
// distributed under the ZLIB licensing policy : http://www.opensource.org/licenses/zlib-license.php
//==================================

// May de SID be with You

#include <SID.h>
#include <MIDI.h>

// Pulse width must be less than 0x0FFF (4095)
// note - must also be higher than nothing if using a square wave
//
#define PULSE_WIDTH 0x0
#define LED 2
// These all must be below 0x0F (15) 


// Midi Implementation Chart
// defining global Control Change implementation
#define CC_MASTER 7	//Master Volume
#define CC_INPUT  8	//Ext Input
#define CC_MODE   9 //Synth Mode
#define CC_LP     10	//LP
#define CC_BP     11	//BP
#define CC_CUT    12	//Cut
#define CC_RES    13	//Res
#define CC_HP     14	//HP
#define CC_TUNE   15	//Master Tuning
#define CC_WAVE1  16	//C1 Wave
#define CC_COARSE1  17	//C1 Coarse
#define CC_FINE1  18	//C1 Fine
#define CC_DUTY1  19	//C1 Duty
#define CC_RING1  20	//C1 Ring
#define CC_SYNC1  21	//C1 Sync
#define CC_ATTACK1  22	//C1 Attack
#define CC_DECAY1 23	//C1 Decay
#define CC_SUSTAIN1 24	//C1 Sustain
#define CC_RELEASE1  25 //C1 Release
#define CC_OUT1 26	//C1 Output
#define CC_WAVE2  32	//C2 Wave
#define CC_COARSE2  33	//C2 Coarse
#define CC_FINE2  34	//C2 Fine
#define CC_DUTY2  35	//C2 Duty
#define CC_RING2  36	//C2 Ring
#define CC_SYNC2  37	//C2 Sync
#define CC_ATTACK2  38	//C2 Attack
#define CC_DECAY2 39	//C2 Decay
#define CC_SUSTAIN2 40	//C2 Sustain
#define CC_RELEASE2 41	//C2 Release
#define CC_OUT2 42	//C2 Output
#define CC_WAVE3  48	//C3 Wave
#define CC_COARSE3  49	//C3 Coarse
#define CC_FINE3  50	//C3 Fine
#define CC_DUTY3  51	//C3 Duty
#define CC_RING3  52	//C3 Ring
#define CC_SYNC3  53	//C3 Sync
#define CC_ATTACK3  54	//C3 Attack
#define CC_DECAY3 55	//C3 Decay
#define CC_SUSTAIN3  56	//C3 Sustain
#define CC_RELEASE3 57	//C3 Release
#define CC_OUT3 58	//C3 Output

static int note_state = 0;

static const unsigned char BASE_NOTE = 12;
static const unsigned int NOTES[] = {
  291, 308, 326, 346, 366, 388, 411, 435, 461, 489, 518, 549, 
  581, 616, 652, 691, 732, 776, 822, 871, 923, 978, 1036, 1097, 1163, 1232, 1305, 1383, 1465, 1552, 1644, 
  1742, 1845, 1955, 2071, 2195, 2325, 2463, 2610, 2765, 2930, 3104, 3288, 3484, 3691, 3910, 4143, 4388, 4650,  
  4927, 5220, 5530, 5859, 6207, 6577, 6968, 7382, 7821, 8286, 8779, 9301, 9854, 10440, 11060, 11718, 12415, 
  13153, 13935, 14764, 15642, 16572, 17557, 18601, 19709, 20897, 22121, 23436, 24830, 26306, 27871, 29528, 
  31234, 33144, 35115, 37203, 39415, 41759, 44242, 46873, 49660, 52613, 55741, 59056, 62567};

//==============================
// GLOBAL SYNTH VARIABLES INIT
//==============================
// Coarse Frequency, base 32, value from 0 (-16) to 31 (+15)
static byte coarse1 = 16;
static byte coarse2 = 16;
static byte coarse3 = 16;

// Fine Frequency, base 0 value from 0  to 127 
static byte fine1 = 0;
static byte fine2 = 0;
static byte fine3 = 0;


// Default midi channel
static byte mchannel = 1;

// Synth State Managment
static boolean trigged = false; // indicating if synth is playing
static boolean retrigger = true; // indicating if retriggering ADSR is on

// Local Osc Frequency 
// Registers @ B00000 and B00001
// Default 440hz = 0x1CD5
static word	freq1 = 0x1CD5;
static word	freq2 = 0x1CD5;
static word	freq3 = 0x1CD5;

// PWM
// Default Square Wave = 0x800  
// Formula : PWout = (PWn/40.95) %
static word	pw1 = 0x800;
static word	pw2 = 0x800;
static word	pw3 = 0x800;

// Control Register
// Default: Saw
// Content NOI|SQR|SAW|TRI|TST|MOD|SYN|GAT
static byte	ctrl1 = 0X10;
static byte	ctrl2 = 0X00;
static byte	ctrl3 = 0X00;

// Attack and Decay
// Default : Shortest
static byte	ad1 = 0x00;
static byte	ad2 = 0x00;
static byte	ad3 = 0x00;

// Sustain and Release
// Default: Max sustain and no release
static byte	sr1 = 0xF0;
static byte	sr2 = 0xF0;
static byte	sr3 = 0xF0;

// Cutoff Frequency
static word	freq = 0x7FF;

// Resonnance and CTRL
// Res= 0 no resonnance, Res=F max resonnance
// Routing 0 is no filter, 1 is filter
// RES(4)|EXT|F3|F2|F1
static byte	res = 0x00;

// Mode 
// Content EXT|HP|BP|LP|VOL(4)
static byte	mode = 0x0F;

// Mono Note Memory
static byte last_note = 0;
static byte current_note = 0;
static byte notes = 0;

// ########################################################

SID sid;

// Give voices some initial params
//
void synth_init() 
{
  // Freq init
  sid.send(SID_FREQ1LO,char(freq1));
  sid.send(SID_FREQ1HI,char(freq1>>8));
  sid.send(SID_FREQ2LO,char(freq2));
  sid.send(SID_FREQ2HI,char(freq2>>8));
  sid.send(SID_FREQ3LO,char(freq3));
  sid.send(SID_FREQ3HI,char(freq3>>8));

  // PWM Init
  sid.send(SID_PW1LO,char(pw1));
  sid.send(SID_PW1HI,char(pw1>>8));
  sid.send(SID_PW2LO,char(pw2));
  sid.send(SID_PW2HI,char(pw2>>8));
  sid.send(SID_PW3LO,char(pw3));
  sid.send(SID_PW3HI,char(pw3>>8));

  // CTRL
  sid.send(SID_CTRL1,ctrl1);
  sid.send(SID_CTRL2,ctrl2);
  sid.send(SID_CTRL3,ctrl3);

  // ADSR
  sid.send(SID_AD1,ad1);
  sid.send(SID_SR1,sr1);
  sid.send(SID_AD2,ad2);
  sid.send(SID_SR2,sr2);
  sid.send(SID_AD3,ad3);
  sid.send(SID_SR3,sr3);

  // Filter + Main
  sid.send(SID_FCLO,char(freq));
  sid.send(SID_FCHI,char(freq>>3));
  sid.send(SID_RES,res);
  sid.send(SID_MODE,mode);
}

// Play sound at frequency specified on voice specified
//
void synth_on(int note) 
{
  freq1=NOTES[note+coarse1-16]+fine1;
  sid.send(SID_FREQ1LO,char(freq1));
  sid.send(SID_FREQ1HI,char(freq1>>8));
  freq2=NOTES[note+coarse2-16]+fine2;
  sid.send(SID_FREQ2LO,char(freq2));
  sid.send(SID_FREQ2HI,char(freq2>>8));
  freq3=NOTES[note+coarse3-16]+fine3;
  sid.send(SID_FREQ3LO,char(freq3));
  sid.send(SID_FREQ3HI,char(freq3>>8));
  ctrl1|=1;
  sid.send(SID_CTRL1,ctrl1);
  ctrl2|=1;
  sid.send(SID_CTRL2,ctrl2);
  ctrl3|=1;
  sid.send(SID_CTRL3,ctrl3);
}

void synth_off(int note) 
{
  ctrl1&=~1;
  sid.send(SID_CTRL1,ctrl1);
  ctrl2&=~1;
  sid.send(SID_CTRL2,ctrl2);
  ctrl3&=~1;
  sid.send(SID_CTRL3,ctrl3);
}

void setup() 
{
  pinMode(LED,OUTPUT);
  pinMode(A5,INPUT);
  digitalWrite(LED,HIGH);
  delay(1000);
  // Set up the SID chip
  sid.clk_setup();
  sid.SPI_setup();

  // Give the voices some initial values
  // Set up the MIDI input
  MIDI.begin();
  MIDI.setHandleNoteOn(DoHandleNoteOn);
  MIDI.setHandleNoteOff(DoHandleNoteOff);
  MIDI.setHandleControlChange(DoHandleControlChange);

  synth_init();
  digitalWrite(LED,LOW);
}

// Main program loop
//
void loop() {
  MIDI.read();
/*  if (!digitalRead(A5)) 
    { synth_on(40);
      digitalWrite(LED,HIGH);
    } else {
      synth_off(40);
      digitalWrite(LED,LOW);
    }*/

}

// Note On Handler
// PS : The if / else is implemented because some Midi equipements do not use Note Off, but use instead note On with velocity = 0 
void DoHandleNoteOn (byte channel, byte note, byte velocity) {
  if( velocity==0 ) { 
    notes--;
    if (notes==0) {
      synth_off(note);
      digitalWrite(LED,LOW);
    }
    else {
      if (note==current_note) {
        synth_on(last_note);
        current_note=last_note;
      }
      current_note=last_note;
    }
  }
  else { 
    digitalWrite(LED,HIGH); 
    synth_on(note); 
    last_note = current_note;
    current_note = note;
    notes++;
  }

}


void DoHandleNoteOff (byte channel, byte note, byte velocity) {
  notes--;
  if (notes==0) {
    synth_off(note);
    digitalWrite(LED,LOW);
  }
  else {
    if (note==current_note) {
      synth_on(last_note);
      current_note=last_note;
    }
    current_note=last_note;
  }
}

void DoHandleControlChange (byte channel, byte number, byte value) {
  switch (number) {

  case CC_CUT:
    freq = value<<4;
    sid.send(SID_FCLO,char(freq));
    sid.send(SID_FCHI,char(freq>>3));
    break;

  case CC_RES:
    value = (value<<1)&0xF0;
    res &= 0x0F;
    res |= value;
    sid.send(SID_RES,res);
    break;

  case CC_LP:
    if (value>63) {
      mode|=16; 
    }
    else {  
      mode&=~16; 
    }
    sid.send(SID_MODE,mode);
    break;

  case CC_BP:
    if (value>63) {
      mode|=32; 
    }
    else {  
      mode&=~32; 
    }
    sid.send(SID_MODE,mode);
    break;

  case CC_HP:
    if (value>63) {
      mode|=64; 
    }
    else {  
      mode&=~64; 
    }
    sid.send(SID_MODE,mode);
    break;

    // channel 1

  case CC_ATTACK1:
    ad1 &= 0x0F;
    value = (value<<1)&0xF0;
    ad1 |= value;
    sid.send(SID_AD1,ad1);
    break;

  case CC_DECAY1:
    ad1 &= 0xF0;
    ad1 |= (value>>3);
    sid.send(SID_AD1,ad1);
    break;

  case CC_SUSTAIN1:
    sr1 &= 0x0F;
    value = (value<<1)&0xF0;
    sr1 |= value;
    sid.send(SID_SR1,sr1);
    break;

  case CC_RELEASE1:
    sr1 &= 0xF0;
    sr1 |= (value>>3);
    sid.send(SID_SR1,sr1);
    break;

  case CC_COARSE1:
    coarse1 = value>>2;
    freq1=NOTES[current_note+coarse1-16]+fine1;
    sid.send(SID_FREQ1LO,char(freq1));
    sid.send(SID_FREQ1HI,char(freq1>>8));
    break;

  case CC_FINE1:
    fine1 = value;
    freq1=NOTES[current_note+coarse1-16]+fine1;
    sid.send(SID_FREQ1LO,char(freq1));
    sid.send(SID_FREQ1HI,char(freq1>>8));
    break;

  case CC_SYNC1:
    if (value>63) {
      ctrl1|=2; 
    }
    else {  
      ctrl1&=~2; 
    }
    sid.send(SID_CTRL1,ctrl1);
    break;

  case CC_RING1:
    if (value>63) {
      ctrl1|=4; 
    }
    else {  
      ctrl1&=~4; 
    }
    sid.send(SID_CTRL1,ctrl1);
    break;    

  case CC_WAVE1:
    value= value >>4;
    ctrl1&=0x0F;
    switch (value) {
    case 0:
      value = 0;
      break;
    case 1:
      value = 16;
      break;
    case 2:
      value = 32;
      break;
    case 3:
      value = 64;
      break;
    default:
      value = 128;
      break;
    }
    ctrl1|=value;
    sid.send(SID_CTRL1,ctrl1);
    break; 

  case CC_OUT1:
    if (value>63) {
      res|=1; 
    }
    else {  
      res&=~1; 
    }
    sid.send(SID_RES,res);
    break;

  case CC_DUTY1:
    pw1 = value<<3+1;  
    sid.send(SID_PW1LO,char(pw1));
    sid.send(SID_PW1HI,char(pw1>>8));
    break;

    //channel 2

  case CC_ATTACK2:
    ad2 &= 0x0F;
    value = (value<<1)&0xF0;
    ad2 |= value;
    sid.send(SID_AD2,ad2);
    break;

  case CC_DECAY2:
    ad2 &= 0xF0;
    ad2 |= (value>>3);
    sid.send(SID_AD2,ad2);
    break;

  case CC_SUSTAIN2:
    sr2 &= 0x0F;
    value = (value<<1)&0xF0;
    sr2 |= value;
    sid.send(SID_SR2,sr2);
    break;

  case CC_RELEASE2:
    sr2 &= 0xF0;
    sr2 |= (value>>3);
    sid.send(SID_SR2,sr2);
    break;

  case CC_COARSE2:
    coarse2 = value>>2;
    freq2=NOTES[current_note+coarse2-16]+fine2;
    sid.send(SID_FREQ2LO,char(freq2));
    sid.send(SID_FREQ2HI,char(freq2>>8));
    break;

  case CC_FINE2:
    fine2 = value;
    freq2=NOTES[current_note+coarse2-16]+fine2;
    sid.send(SID_FREQ2LO,char(freq2));
    sid.send(SID_FREQ2HI,char(freq2>>8));
    break;

  case CC_SYNC2:
    if (value>63) {
      ctrl2|=2; 
    }
    else {  
      ctrl2&=~2; 
    }
    sid.send(SID_CTRL2,ctrl2);
    break;

  case CC_RING2:
    if (value>63) {
      ctrl2|=4; 
    }
    else {  
      ctrl2&=~4; 
    }
    sid.send(SID_CTRL2,ctrl2);
    break;    

  case CC_WAVE2:
    value= value >>4;
    ctrl2&=0x0F;
    switch (value) {
    case 0:
      value = 0;
      break;
    case 1:
      value = 16;
      break;
    case 2:
      value = 32;
      break;
    case 3:
      value = 64;
      break;
    default:
      value = 128;
      break;
    }
    ctrl2|=value;
    sid.send(SID_CTRL2,ctrl2);
    break; 

  case CC_OUT2:
    if (value>63) {
      res|=2; 
    }
    else {  
      res&=~2; 
    }
    sid.send(SID_RES,res);
    break;    

  case CC_DUTY2:
    pw2 = value<<3+1;  
    sid.send(SID_PW2LO,char(pw2));
    sid.send(SID_PW2HI,char(pw2>>8));
    break;    

    //channel 3

  case CC_ATTACK3:
    ad3 &= 0x0F;
    value = (value<<1)&0xF0;
    ad3 |= value;
    sid.send(SID_AD3,ad3);
    break;

  case CC_DECAY3:
    ad3 &= 0xF0;
    ad3 |= (value>>3);
    sid.send(SID_AD3,ad3);
    break;

  case CC_SUSTAIN3:
    sr3 &= 0x0F;
    value = (value<<1)&0xF0;
    sr3 |= value;
    sid.send(SID_SR3,sr3);
    break;

  case CC_RELEASE3:
    sr3 &= 0xF0;
    sr3 |= (value>>3);
    sid.send(SID_SR3,sr3);
    break;

  case CC_COARSE3:
    coarse3 = value>>2;
    freq3=NOTES[current_note+coarse3-16]+fine3;
    sid.send(SID_FREQ3LO,char(freq3));
    sid.send(SID_FREQ3HI,char(freq3>>8));
    break;

  case CC_FINE3:
    fine3 = value;
    freq3=NOTES[current_note+coarse3-16]+fine3;
    sid.send(SID_FREQ3LO,char(freq3));
    sid.send(SID_FREQ3HI,char(freq3>>8));
    break;

  case CC_SYNC3:
    if (value>63) {
      ctrl3|=2; 
    }
    else {  
      ctrl3&=~2; 
    }
    sid.send(SID_CTRL3,ctrl3);
    break;

  case CC_RING3:
    if (value>63) {
      ctrl3|=4; 
    }
    else {  
      ctrl3&=~4; 
    }
    sid.send(SID_CTRL3,ctrl3);
    break;    

  case CC_WAVE3:
    value= value >>4;
    ctrl3&=0x0F;
    switch (value) {
    case 0:
      value = 0;
      break;
    case 1:
      value = 16;
      break;
    case 2:
      value = 32;
      break;
    case 3:
      value = 64;
      break;
    default:
      value = 128;
      break;
    }
    ctrl3|=value;
    sid.send(SID_CTRL3,ctrl3);
    break; 

  case CC_OUT3:
    if (value>63) {
      res|=4; 
    }
    else {  
      res&=~4; 
    }
    sid.send(SID_RES,res);
    break;    

  case CC_DUTY3:
    pw3 = value<<3+1;  
    sid.send(SID_PW3LO,char(pw3));
    sid.send(SID_PW3HI,char(pw3>>8));
    break;    

  }

}

/* notebook : handlers and their call type
 void HandleNoteOff (byte channel, byte note, byte velocity)
 void HandleNoteOn (byte channel, byte note, byte velocity)
 void HandleAfterTouchPoly (byte channel, byte note, byte pressure)
 void HandleControlChange (byte channel, byte number, byte value)
 void HandleProgramChange (byte channel, byte number)
 void HandleAfterTouchChannel (byte channel, byte pressure)
 void HandlePitchBend (byte channel, int bend)
 void HandleSystemExclusive (byte *array, byte size)
 void HandleTimeCodeQuarterFrame (byte data)
 void HandleSongPosition (unsigned int beats)
 void HandleSongSelect (byte songnumber)
 void HandleTuneRequest (void)
 void HandleClock (void)
 void HandleStart (void)
 void HandleContinue (void)
 void HandleStop (void)
 void HandleActiveSensing (void)
 */












