#include <RCSwitch.h>
#define RX 4

RCSwitch mySwitch = RCSwitch();

void setup() {
  Serial.begin(115200);

  // We connect the 433MHz receiver module to GPIO 4
  mySwitch.enableReceive(RX);  // We use GPIO 4
  Serial.println("433MHz Receiver Ready");
}

void loop() {
  if (mySwitch.available()) {
    long receivedValue = mySwitch.getReceivedValue();

    Serial.print("Value Received: ");
    Serial.println(receivedValue);
    Serial.print("Bit Count: ");
    Serial.println(mySwitch.getReceivedBitlength());
    Serial.print("Protokol: ");
    Serial.println(mySwitch.getReceivedProtocol());
    Serial.print("Delay: ");
    Serial.println(mySwitch.getReceivedDelay());

    if (receivedValue != 0) {
      Serial.print("Raw Data: ");
      unsigned int *rawData = mySwitch.getReceivedRawdata();
      for (int i = 0; i < RCSWITCH_MAX_CHANGES; i++) {
        Serial.print(rawData[i]);
        Serial.print(" ");
      }
      Serial.println();
    } else {
      Serial.println("Unknown encoding, but signal detected");
    }

    mySwitch.resetAvailable();
  }
}