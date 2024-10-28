#include <HX711_ADC.h>
#include <EEPROM.h>
#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>

// Pin definitions for load cell
const int HX711_dout = 4;
const int HX711_sck = 5;
HX711_ADC LoadCell(HX711_dout, HX711_sck);

// RF24 setup
RF24 radio(9, 8); // CE, CSN
RF24Network network(radio);
const uint16_t this_node = 01;  // Change to 01 for Node 1 and 02 for Node 2
const uint16_t master_node = 00; // Master node address

// EEPROM address for calibration
const int calVal_eepromAddress = 0;

// Timing variables
unsigned long t = 0;
const int serialPrintInterval = 1000;

void setup() {
  Serial.begin(57600);
  LoadCell.begin();

  // Load calibration value from EEPROM
  float calibrationValue;
  EEPROM.get(calVal_eepromAddress, calibrationValue);
  LoadCell.setCalFactor(calibrationValue);

  LoadCell.start(2000);
  LoadCell.tareNoDelay();

  // Initialize RF24 and network
  SPI.begin();
  radio.begin();
  network.begin(90, this_node);
  radio.setPALevel(RF24_PA_MIN);
}

void loop() {
  network.update();
  static bool newDataReady = false;

  if (LoadCell.update()) newDataReady = true;

  if (millis() > t + serialPrintInterval) {
    float weight = 0;

    if (newDataReady) {
      weight = LoadCell.getData();
      newDataReady = false;
    }

    if (!isnan(weight)) {
      Serial.print("Node ");
      Serial.print(this_node);
      Serial.print(" Weight: ");
      Serial.println(weight);

      // Transmit weight to master node
      RF24NetworkHeader header(master_node);
      network.write(header, &weight, sizeof(weight));

      t = millis(); // Reset interval timer
    }
  }
}
