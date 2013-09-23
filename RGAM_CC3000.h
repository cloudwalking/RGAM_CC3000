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
#include "Adafruit_CC3000.h"
#endif

#include <SPI.h>

class RGAM_CC3000 {
  public:

    // Constructor
    // Pins:
    // IRQ - must be interrupt pin
    // VBEN, CS - any two pins
    RGAM_CC3000(int IRQ, int VBEN, int CS);

    // Starts up the hardware.
    bool setup();
    
    // Connect to a wifi network and configures DHCP.
    // SSID max 32 characters.
    // Security is one of (see wlan.h):
    // WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA, WLAN_SEC_WPA2
    bool connectToNetwork(const char *SSID, const char *PASS, uint8_t SECURITY);
    bool isConnected();
    
    void printIPForHost(const char *host);
    
    // Constructs and passes a GET request to the given host.
    // EG: _wifi.GETRequest("freegeoip.net", "/json/", NULL, result);
    // The string passed in as 'result' will contain the response of the request.
    // Returns true if the request succeeded, false on failure.
    // "Succeeded" is currently define as "request returned any response from the server".
    bool GETRequest(const char *host, const char *path, const char *headers, char *result);
    
    // Disconnect from the wifi network.
    void disconnectFromNetwork();
    
  private:
    Adafruit_CC3000 _cc3k;
    
    uint32_t ipForHost(const char *host);
    char * constructGETRequest(const char *host, const char *path, const char *headers);
};

#endif