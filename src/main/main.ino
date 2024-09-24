#include <WiFi.h>
#include <RCSwitch.h>

#define RX 16
#define TX 4

#define RD 34

#define LED 2

int ledStatus = 0;
bool recordStatus = 0;

#define MAX_CODES 10
#define MAX_CODE_LENGTH 33  // 32 bits + null terminator

char txCodes[MAX_CODES][MAX_CODE_LENGTH] = {
  "11100110001101010100001", // your open rf code
  "11101001001101110100101", // your pause rf code
  "11111000101001110100111", // your close rf code
  "1"                        // last record rf code
};

int codeCount = 4;


RCSwitch mySwitch = RCSwitch();

const char* ssid = "example-ssid";       // your wifi ssid
const char* password = "example-passwd"; // your wifi password
NetworkServer server(80);

const char* script = R"delimiter(<script>document.querySelectorAll(".button-container a").forEach(e=>{e.addEventListener("click",function(t){t.preventDefault(),fetch(e.href).then(e=>{e.ok?console.log("Fetch request succeeded:",e.status):console.error("Fetch request failed:",e.status)}).catch(e=>console.error("Fetch request error:",e))})});</script>)delimiter";
const char* htmlParts[] = {
  "HTTP/1.1 200 OK",
  "Content-type:text/html",
  "",
  "<!doctypehtml><html lang=tr><meta charset=UTF-8><meta content=\"width=device-width,initial-scale=1\"name=viewport><title>Pergola Awning Control Panel</title><style>*{box-sizing:border-box;margin:0;padding:0}body{font-family:Arial,sans-serif;line-height:1.6;padding:20px;max-width:100%;margin:0 auto}h1{color:#333;margin-bottom:15px}p{margin-bottom:15px}.button-container{margin-bottom:15px;display:flex;flex-direction:column;gap:10px;width:100%}a{display:block;padding:10px;background-color:#007bff;color:#fff;text-decoration:none;border-radius:5px;font-size:16px;text-align:center;width:100%}a:hover{background-color:#0056b3}@media (min-width:768px){body{max-width:600px}.button-container{flex-direction:row;flex-wrap:wrap}a{flex:1 1 calc(33.3333% - 10px);margin-bottom:10px}}</style>",
  "<h1>Pergola Awning Control Panel</h1>",
  "<p>Use the links below to check the awning:</p>",
  "<div class=button-container>",
  "<a href=/open>Open the awning</a> ",
  "<a href=/pause>Stop Awning</a> ",
  "<a href=/close>Close the awning</a> ",
  "<a href=/play>Play Last Recorded Signal</a> ",
  "<a href=/record>Start New Signal Record</a>",
  "</div>",
  script
};

void setup() {
  Serial.begin(115200);
  delay(10);

  //WIFI SETUP
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();

  //PINS SETUP
  pinMode(LED, OUTPUT);
  //RAIN DETECTOR SETUP
  pinMode(RD, INPUT);

  mySwitch.enableReceive(RX);
  mySwitch.enableTransmit(TX);
  static const RCSwitch::Protocol customprotocol = { 290, { 1, 15 }, { 2, 1 }, { 1, 1 }, false };
  mySwitch.setProtocol(customprotocol);

  Serial.println("Modül Hazır!");
}

void loop() {
  NetworkClient client = server.accept();  // listen for incoming clients

  if (client) {                     // if you get a client,
    Serial.println("New Client.");  // print a message out the serial port
    String currentLine = "";        // make a String to hold incoming data from the client
    while (client.connected()) {    // loop while the client's connected
      if (client.available()) {     // if there's bytes to read from the client,
        char c = client.read();     // read a byte, then
        Serial.write(c);            // print it out the serial monitor
        if (c == '\n') {            // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            for (int i = 0; i < sizeof(htmlParts) / sizeof(htmlParts[0]); i++) {
              client.println(htmlParts[i]);
            }
            for (int i = 0; i < codeCount; i++) {
              client.print("Code: ");
              client.print(charArrayToStr(txCodes[i]));
              client.println("<br>");
            }

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {  // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /open")) {
          ledBlink();
          sendCode(0);
        }
        if (currentLine.endsWith("GET /pause")) {
          ledBlink();
          sendCode(1);
        }
        if (currentLine.endsWith("GET /close")) {
          ledBlink();
          sendCode(2);
        }
        if (currentLine.endsWith("GET /play")) {
          ledBlink();
          sendCode(3);
        }
        if (currentLine.endsWith("GET /record")) {
          ledBlink();
          recordStatus = 1;
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }

  if (digitalRead(RD) == LOW) {
    ledBlink();
    sendCode(2);
  }

  if (recordStatus && mySwitch.available()) {
    recordStatus = 0;
    unsigned long receivedValue = mySwitch.getReceivedValue();
    int bitLength = mySwitch.getReceivedBitlength();

    if (bitLength < MAX_CODE_LENGTH) {
      char* receivedBit = intToBitString(receivedValue, bitLength);
      Serial.print("Value Received: ");
      Serial.println(charArrayToStr(receivedBit));
      setCustomCode(receivedBit);
    } else {
      Serial.println("The data received is too long!");
    }

    mySwitch.resetAvailable();
    ledBlink();
  }

  delay(200);
}

void ledToggle() {
  digitalWrite(LED, !ledStatus);
  ledStatus = !ledStatus;
}

void ledBlink() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED, i % 2);
    delay(100);
  }
}

char* intToBitString(unsigned long value, int numBits) {
  if (numBits > 32 || numBits <= 0) {
    return strdup("Invalid bit count");
  }

  char* bitString = (char*)malloc(numBits + 1);  // +1 for null terminator
  if (bitString == NULL) {
    return strdup("Memory allocation failed");
  }

  for (int i = numBits - 1; i >= 0; i--) {
    bitString[numBits - 1 - i] = (value & (1UL << i)) ? '1' : '0';
  }
  bitString[numBits] = '\0';  // Null-terminate the string

  return bitString;
}

String charArrayToStr(char* value) {
  String str = "";
  for (int i = 0; i < strlen(value); i++) {
    str += value[i];
  }
  return str;
}

void addCode(const char* newCode) {
  if (codeCount < MAX_CODES) {
    strncpy(txCodes[codeCount], newCode, MAX_CODE_LENGTH - 1);
    txCodes[codeCount][MAX_CODE_LENGTH - 1] = '\0';  // Null-terminate
    codeCount++;
  }
}

void setCustomCode(const char* newCode) {
  strncpy(txCodes[3], newCode, MAX_CODE_LENGTH - 1);
  txCodes[3][MAX_CODE_LENGTH - 1] = '\0';
}

void sendCode(int i) {
  char* c = txCodes[i];
  mySwitch.send(c);
  Serial.println("Message Sent!");
  Serial.println("Message Value:");
  Serial.println(charArrayToStr(c));
  Serial.println();
  ledToggle();
};