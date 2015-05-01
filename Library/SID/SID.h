/*
MOS6581 SID CHIP Library
Configurating Registers and using the MOS 6581 Chip

- Beta 0.3 19 Sep/2011, Vince
* Changed delay to lowest acceptable value (through testing) : 300us (margin taken, it fails with 200us and works with 250us)

- Beta 0.21 1 Sep/2011, Matthew Dodkins
* Separated definition and declaration
* Added 10ms delay to SID_send()
* Added DORD ('Data Order') bit set in SPI setup

- Beta 0.2
 Correcting Data Order in SPI configuration, modified to MSB First

- Beta 0.1
Initial release
*/

#ifndef SID__H
#define SID__H

#include "Arduino.h"
#include "6581.h"

class SID
{
  public:
    void clk_setup(void);
    void SPI_setup(void);
    byte send(byte address, byte data);
};


#endif // SID__H
