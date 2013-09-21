#include <RGAM_CC3000.h>


#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include "utility/debug.h"

RGAM_CC3000::RGAM_CC3000(int IRQ_, int VBEN_, int CS_) {
  Serial.println("RGAM_CC3000");
  
  IRQ = IRQ_;
  VBEN = VBEN_;
  CS = CS_;
}

bool RGAM_CC3000::setup() {
  Serial.print("Starting hardware... ");
  _cc3000 = Adafruit_CC3000(CS, IRQ, VBEN, SPI_CLOCK_DIV2);

  if(!_cc3000.begin()) {
    Serial.println("failed");
    return false;
  }

  Serial.println("done");
  return true;
}