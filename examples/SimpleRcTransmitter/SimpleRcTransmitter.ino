#include <RCSwitch.h>
#define TX 4

RCSwitch mySwitch = RCSwitch();

void setup() {

  Serial.begin(115200);
  
  // Transmitter is connected to ESP32 DEV KIT V1 Pin D4 
  mySwitch.enableTransmit(TX);
  
  static const RCSwitch::Protocol customprotocol = { 285, { 1, 15 }, { 2, 1 }, { 1, 1 }, false };
  mySwitch.setProtocol(customprotocol);
  Serial.println("433 MHZ TX READY!");
}

void loop() {

  mySwitch.send(1252539, 23);
  Serial.println("Message Sent!");
  delay(1000);  

}