/**
 * RGAM_CC3000
 * More pleasant API for Adafruit's CC3000 library.
 * Copyright 2013 RGAM LLC.
 *
 * You must #include <Adafruit_CC3000.h> and <SPI.h>
 * in the  project.ino file before you #include this file.
 *
 * FYI you must use SPI pins for SCK, MISO, and MOSI.
 * On an UNO: SCK = 13, MISO = 12, and MOSI = 11
 */
 
#ifndef RGAM_CC3000_H
#define RGAM_CC3000_H

#ifndef WLAN_CONNECT_TIMEOUT
#include <Adafruit_CC3000.h>
#endif

#include <SPI.h>

class RGAM_CC3000 {
  public:

    int IRQ, VBEN, CS;

    // Constructor
    // Pins:
    // IRQ - must be interrupt pin
    // VBEN, CS - any two pins
    RGAM_CC3000(int IRQ, int VBEN, int CS);

    bool setup();
    
  private:
    Adafruit_CC3000 _cc3000;
    int _ip;
};

#endif