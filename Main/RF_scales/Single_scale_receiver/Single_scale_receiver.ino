#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>

RF24 radio(9, 8); // CE, CSN
const byte address[10] = "ADDRESS01";

void setup() {
  Serial.begin(57600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    float receivedWeight;
    radio.read(&receivedWeight, sizeof(receivedWeight));
    Serial.print("Received weight: ");
    Serial.println(receivedWeight, 2);
  }
}
