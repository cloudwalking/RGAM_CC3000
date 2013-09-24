#include <RGAM_CC3000.h>

#include <ccspi.h>
#include <string.h>

#include "utility/debug.h"

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
  
  // Serial.print("IP      : "); _cc3k.printIPdotsRev(ip);
  // Serial.print("\nNetmask : "); _cc3k.printIPdotsRev(netmask);
  // Serial.print("\nGateway : "); _cc3k.printIPdotsRev(gateway);
  // Serial.print("\nDHCP    : "); _cc3k.printIPdotsRev(dhcp);
  // Serial.print("\nDNS     : "); _cc3k.printIPdotsRev(dns);
  
  Serial.print("Free RAM: "); Serial.println(getFreeRam(), DEC);
  
  return _cc3k.checkConnected() && _cc3k.checkDHCP();
}

bool RGAM_CC3000::isConnected() {
  return _cc3k.checkConnected();
}

uint32_t RGAM_CC3000::ipForHost(const char *host) {
  uint32_t ip = 0;
  char *host_ = (char *) malloc(strlen(host) * sizeof(char *));
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
  _cc3k.setPrinter(&Serial);
  _cc3k.printIPdotsRev(ip);
}

bool RGAM_CC3000::GETRequest(const char *host, const char *path, const char *headers, char *result) {
  uint32_t ip = ipForHost(host);
  if (!ip) return false;
  
  Serial.print("Free RAM: "); Serial.println(getFreeRam(), DEC);
  
  char *requestString = constructGETRequest(host, path, headers);
  
  _cc3k.setPrinter(NULL);
  Adafruit_CC3000_Client client = _cc3k.connectTCP(ip, 80);
  _cc3k.setPrinter(&Serial);
  if (!client.connected()) return false;
  
  client.print(requestString);
  free(requestString);
  
  uint32_t time = millis();
  while(!client.available() && millis() - time < 5000.0) { }
  
  Serial.println("A");
  
  Serial.print("Free RAM: "); Serial.println(getFreeRam(), DEC);

  uint32_t bytesReceived = 0;
  char read;
  bool start = false;
  // bytesReceived = client.read(result_, sizeof(result_), 0);
  while ( (read = (char)client.read()) != 0 ) {
    if (read != '{' && !start) {
      Serial.print(read);
      continue;
    } else if (!start) {
      Serial.println("BEGIN BUFFERING");
      start = true;
    } 
    
    Serial.print(read);
    result[bytesReceived++] = read;
  }
  Serial.println();
  
  Serial.print("Free RAM: "); Serial.println(getFreeRam(), DEC);
  
  Serial.println("B");
  
  // while ( (read = (char)client.read()) != 0 ) {
  //   
  // }
  
  // result[0] = NULL;
  // int oneBufferSize = 512;
  // int currentBufferSize = 0;
  // int bufferPointer = 0;
  // while ( (read = (char)client.read()) != 0 ) {
  //   Serial.println(read);
  //   // Serial.println((int)read);
  //   // If the pointer is pointing at the second to last character in the buffer
  //   // (gotta keep that null terminator around), grow the buffer.
  //   if (bufferPointer >= currentBufferSize - 2) {
  //     Serial.print("EXPANDING BUFFER FROM ");
  //     Serial.println(currentBufferSize);
  //     //result = (char *)realloc(result, oneBufferSize * sizeof(char));
  //     result = (char *)malloc(oneBufferSize * sizeof(char));
  //     currentBufferSize += oneBufferSize;
  //   }
  // 
  //   // bufferPointer++;
  //   result[bufferPointer] = read;
  //   bufferPointer++;
  // }  
  
  client.close();
  
  Serial.print("Free RAM: "); Serial.println(getFreeRam(), DEC);
  
  Serial.println("C");
  
  return bytesReceived;
}

void RGAM_CC3000::disconnectFromNetwork() {
  _cc3k.disconnect();
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

// Returns a malloc'd request string.
// EG "GET /json/ HTTP/1.0\r\nConnection: close\r\n\r\n"
// Pass additional headers in the format "Header: xyz\r\nHeader:xyz".
char * RGAM_CC3000::constructGETRequest(const char *host, const char *path, const char *passedHeaders) {
  const char *method = "GET";
  const char *http = "HTTP/1.0";
  const char *connectionHeader = "Connection: close";
  const char *lineBreak = "\r\n";
  
  const int strlen_lineBreak = strlen(lineBreak);
  
  // Construct headers.
  // We always use `Connection: close`.
  // We always use `Host: <host>`.
  // Plus whatever is passed to us.
  
  int headerLength = 0;
  
  headerLength += strlen(connectionHeader);
  headerLength += strlen_lineBreak;
  
  headerLength += strlen("Host: ");
  headerLength += strlen(host);
  headerLength += strlen_lineBreak;
  
  if (passedHeaders != NULL) {
    headerLength += strlen(passedHeaders);
    headerLength += strlen_lineBreak;
  }
  
  // Terminating null.
  headerLength += 1;
  
  // Create the header string.
  char *headers = (char *) calloc(headerLength, sizeof(char));
  
  strcat(headers, connectionHeader);
  strcat(headers, lineBreak);
  
  strcat(headers, "Host: ");
  strcat(headers, host);
  strcat(headers, lineBreak);
  
  if (passedHeaders != NULL) {
    strcat(headers, passedHeaders);
    strcat(headers, lineBreak);
  }
  
  // Count the byte length of the request string.
  // The `+ 1` is for space between sections.
  // EG "GET /json/ HTTP/1.0\r\nConnection: close\r\n\r\n"
  int requestLenght = 0;
  requestLenght += strlen(method) + 1; // "GET "
  requestLenght += strlen(path) + 1; // "/PATH/ "
  requestLenght += strlen(http);
  requestLenght += strlen_lineBreak;
  requestLenght += strlen(headers);
  requestLenght += strlen_lineBreak;
  requestLenght += strlen_lineBreak;
  
  // Terminating null.
  requestLenght += 1;
  
  // Create the string.
  // EG "GET /json/ HTTP/1.0\r\nConnection: close\r\n\r\n"
  char *requestString = (char *) calloc(requestLenght, sizeof(char));
  
  strcat(requestString, method);
  strcat(requestString, " ");
  strcat(requestString, path);
  strcat(requestString, " ");
  strcat(requestString, http);
  strcat(requestString, lineBreak);
  strcat(requestString, headers);
  strcat(requestString, lineBreak);
  strcat(requestString, lineBreak);
  
  free(headers);
  
  return requestString;
}

int RGAM_CC3000::getFreeRam() {
  return getFreeRam();
}
