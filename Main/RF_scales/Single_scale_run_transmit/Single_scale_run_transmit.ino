#include <HX711_ADC.h>
#include <EEPROM.h>
#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>

// Pin definitions for load cell
const int HX711_dout = 4;
const int HX711_sck = 5;
HX711_ADC LoadCell(HX711_dout, HX711_sck);

// Pin definitions for RF24
RF24 radio(9, 8); // CE, CSN
const byte address[10] = "ADDRESS01";

// EEPROM address for calibration
const int calVal_eepromAddress = 0;

// Timing variables
unsigned long t = 0;
const int serialPrintInterval = 1000;  // Time interval for serial print

void setup() {
  Serial.begin(57600); // Baud rate to match calibration method
  LoadCell.begin();

  // Load calibration value from EEPROM
  float calibrationValue;
  EEPROM.get(calVal_eepromAddress, calibrationValue);
  LoadCell.setCalFactor(calibrationValue);  // Apply stored calibration value

  LoadCell.start(2000);  // Stabilizing time for load cell
  LoadCell.tareNoDelay(); // Perform tare

  // Initialize RF24
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}

void loop() {
  static boolean newDataReady = false;

  if (LoadCell.update()) newDataReady = true;

  // Print and transmit weight at the specified interval
  if (millis() > t + serialPrintInterval) {
    float weight = 0;

    if (newDataReady) {
      weight = LoadCell.getData();
      newDataReady = false;
    }

    if (weight != NAN) {
      // Print the weight value to the Serial Monitor
      Serial.print("Weight: ");
      Serial.println(weight);

      // Transmit the weight value via RF24
      radio.write(&weight, sizeof(weight));

      // Update the timestamp for the next print interval
      t = millis();
    }
  }


}
