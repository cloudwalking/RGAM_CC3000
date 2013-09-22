#include <RGAM_CC3000.h>

#include <ccspi.h>
#include <string.h>
// #include "utility/debug.h"

#define CLOCK_SPEED SPI_CLOCK_DIV2
#define DHCP_TIMEOUT 60000 // 60 seconds
#define CONNECTION_TIMEOUT 15000 // 15 seconds

RGAM_CC3000::RGAM_CC3000(int IRQ_, int VBEN_, int CS_)
  : _cc3k(CS_, IRQ_, VBEN_, CLOCK_SPEED) {
  Serial.println("RGAM_cc3k");
}

bool RGAM_CC3000::setup() {
  Serial.print("Starting hardware... ");

  if(!_cc3k.begin()) {
    Serial.println("failed");
    return false;
  }

  Serial.println("done");
  return true;
}

bool RGAM_CC3000::connectToNetwork(const char *SSID, const char *PASS, uint8_t SECURITY) {
  Serial.print("Connecting to network... ");
  
  _cc3k.setPrinter(NULL);
  _cc3k.connectToAP(SSID, PASS, SECURITY);
  _cc3k.setPrinter(&Serial);
    
  if (_cc3k.checkConnected()) {
    Serial.println("done");
  } else {
    Serial.println("failed");
    return false;
  }
  
  Serial.print("Requesting DHCP... ");
  
  uint32_t time = millis();
  while (!_cc3k.checkDHCP() && millis() - time < DHCP_TIMEOUT) {
    delay(1000);
  }
  
  if (_cc3k.checkDHCP()) {
    Serial.println("done");
  } else {
    Serial.println("failed");
    return false;
  }
  
  Serial.print("Checking IP... ");
  
  uint32_t ip, netmask, gateway, dhcp, dns;
  if (!_cc3k.getIPAddress(&ip, &netmask, &gateway, &dhcp, &dns)) {
    Serial.println("failed");
    return false;
  } else {
    Serial.println("done");
  }
  
  Serial.print("IP      : "); _cc3k.printIPdotsRev(ip);
  Serial.print("\nNetmask : "); _cc3k.printIPdotsRev(netmask);
  Serial.print("\nGateway : "); _cc3k.printIPdotsRev(gateway);
  Serial.print("\nDHCP    : "); _cc3k.printIPdotsRev(dhcp);
  Serial.print("\nDNS     : "); _cc3k.printIPdotsRev(dns);
  
  return _cc3k.checkConnected() && _cc3k.checkDHCP();
}

bool RGAM_CC3000::isConnected() {
  return _cc3k.checkConnected();
}

uint32_t RGAM_CC3000::ipForHost(const char *host) {
  uint32_t ip = 0;
  char *host_ = (char *)malloc(strlen(host) * sizeof(char *));
  host_ = strcpy(host_, host);

  uint32_t time = millis();
  while (ip == 0 && millis() - time < CONNECTION_TIMEOUT) {
    bool success = _cc3k.getHostByName(host_, &ip);
    if (success) break;
    delay(1000);
  }
  
  return ip;
}

void RGAM_CC3000::printIPForHost(const char *host) {
  uint32_t ip = ipForHost(host);
  Serial.println();
  _cc3k.printIPdotsRev(ip);
}
