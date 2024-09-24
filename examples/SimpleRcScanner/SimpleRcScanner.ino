#define SAMPLESIZE 500

static unsigned int timings[SAMPLESIZE];
static unsigned int pos = 0;
static unsigned long lastTime = 0;

static int receiverPin = 4;
static int interruptPin = 0;

void setup() {
  interruptPin = digitalPinToInterrupt(receiverPin);
  Serial.begin(115200);
  attachInterrupt(interruptPin, handleInterrupt, CHANGE);
  Serial.println("433 MHZ RX READY!");
}

void loop() {
    for (int i = 5; i>0; i--) {
      Serial.print(i);
      Serial.print("... ");
      delay(1000);
    }
    Serial.println();
      
    detachInterrupt(interruptPin);
  
    int finalstate = digitalRead(receiverPin);
    
    char s = Serial.read();
    
    for (unsigned int i = pos + finalstate; i< SAMPLESIZE; i++) {
      Serial.print( timings[i] );
      Serial.print(",");
    }
 
    for (unsigned int i = 0; i < pos; i++) {
      Serial.print( timings[i] );
      Serial.print(",");
    }

    Serial.println("");
    Serial.println("Reset your Arduino to scan again...");

    while(true) {} 
  
}

void handleInterrupt() {
  const long time = micros();
  timings[pos] = time - lastTime;
  lastTime = time;
  if (++pos > SAMPLESIZE-1) {
    pos = 0;
  }
}
