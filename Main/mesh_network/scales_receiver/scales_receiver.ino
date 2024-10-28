#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>

// RF24 setup
RF24 radio(9, 8); // CE, CSN
RF24Network network(radio);

const uint16_t this_node = 00; // Master node address
const uint16_t node01 = 01;    // Transmitter Node 1 address
const uint16_t node02 = 02;    // Transmitter Node 2 address

void setup() {
  Serial.begin(57600);

  // Initialize RF24 and network
  SPI.begin();
  radio.begin();
  network.begin(90, this_node);
  radio.setPALevel(RF24_PA_MIN);
}

void loop() {
  network.update();

  // Check for incoming data
  while (network.available()) {
    RF24NetworkHeader header;
    float incomingWeight;
    network.read(header, &incomingWeight, sizeof(incomingWeight));

    // Identify source and print weight
    if (header.from_node == node01) {
      Serial.print("Node 01 Weight: ");
      Serial.println(incomingWeight);
    } else if (header.from_node == node02) {
      Serial.print("Node 02 Weight: ");
      Serial.println(incomingWeight);
    }
  }
}
