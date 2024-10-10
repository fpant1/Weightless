//-------------------------------------------------------------------------------------
// HX711_ADC.h
// Arduino master library for HX711 24-Bit Analog-to-Digital Converter for Weigh Scales
// Olav Kallhovd sept2017
// This is an example sketch on how to use this library for four HX711 modules
//-------------------------------------------------------------------------------------

#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

// Define pins for each load cell 
const int HX711_dout1 = 4;
const int HX711_sck1 = 5;

const int HX711_dout2 = 9;
const int HX711_sck2 = 8;

const int HX711_dout3 = 11;
const int HX711_sck3 = 10;

const int HX711_dout4 = 13;
const int HX711_sck4 = 12;

// Create HX711 instances
HX711_ADC LoadCell1(HX711_dout1, HX711_sck1);
HX711_ADC LoadCell2(HX711_dout2, HX711_sck2);
HX711_ADC LoadCell3(HX711_dout3, HX711_sck3);
HX711_ADC LoadCell4(HX711_dout4, HX711_sck4);

// EEPROM addresses for each load cell's calibration value
const int calVal_eepromAdress1 = 0;
const int calVal_eepromAdress2 = 10;
const int calVal_eepromAdress3 = 20;
const int calVal_eepromAdress4 = 30;

unsigned long t = 0;

void setup() {
  Serial.begin(57600);
  delay(10);
  Serial.println("Starting...");

  // Load calibration values from EEPROM
  float calibrationValue1;
  float calibrationValue2;
  float calibrationValue3;
  float calibrationValue4;

  EEPROM.get(calVal_eepromAdress1, calibrationValue1);
  EEPROM.get(calVal_eepromAdress2, calibrationValue2);
  EEPROM.get(calVal_eepromAdress3, calibrationValue3);
  EEPROM.get(calVal_eepromAdress4, calibrationValue4);

  // Initialize the load cells
  LoadCell1.begin();
  LoadCell2.begin();
  LoadCell3.begin();
  LoadCell4.begin();

  // Allow time for stabilization
  unsigned long stabilizingtime = 2000; // 2 seconds stabilizing time
  bool tare = true;

  // Start load cells with tare
  byte loadcell1_rdy = 0;
  byte loadcell2_rdy = 0;
  byte loadcell3_rdy = 0;
  byte loadcell4_rdy = 0;

  while ((loadcell1_rdy + loadcell2_rdy + loadcell3_rdy + loadcell4_rdy) < 4) {
    if (!loadcell1_rdy) loadcell1_rdy = LoadCell1.startMultiple(stabilizingtime, tare);
    if (!loadcell2_rdy) loadcell2_rdy = LoadCell2.startMultiple(stabilizingtime, tare);
    if (!loadcell3_rdy) loadcell3_rdy = LoadCell3.startMultiple(stabilizingtime, tare);
    if (!loadcell4_rdy) loadcell4_rdy = LoadCell4.startMultiple(stabilizingtime, tare);
  }

  // Check for startup errors
  if (LoadCell1.getTareTimeoutFlag()) Serial.println("Timeout on load cell 1.");
  if (LoadCell2.getTareTimeoutFlag()) Serial.println("Timeout on load cell 2.");
  if (LoadCell3.getTareTimeoutFlag()) Serial.println("Timeout on load cell 3.");
  if (LoadCell4.getTareTimeoutFlag()) Serial.println("Timeout on load cell 4.");

  // Set calibration factors
  LoadCell1.setCalFactor(calibrationValue1);
  LoadCell2.setCalFactor(calibrationValue2);
  LoadCell3.setCalFactor(calibrationValue3);
  LoadCell4.setCalFactor(calibrationValue4);

  Serial.println("All load cells ready.");
}

void loop() {
  static bool newDataReady = 0;
  const int serialPrintInterval = 500; // print every 500ms

  // Check for new data
  if (LoadCell1.update()) newDataReady = true;
  LoadCell2.update();
  LoadCell3.update();
  LoadCell4.update();

  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      // Get data from all load cells and sum the values
      float weight1 = LoadCell1.getData();
      float weight2 = LoadCell2.getData();
      float weight3 = LoadCell3.getData();
      float weight4 = LoadCell4.getData();

      float totalWeight = weight1 + weight2 + weight3 + weight4;

      // Print individual and total weights
      Serial.print("Weight 1: "); Serial.print(weight1);
      Serial.print(" | Weight 2: "); Serial.print(weight2);
      Serial.print(" | Weight 3: "); Serial.print(weight3);
      Serial.print(" | Weight 4: "); Serial.print(weight4);
      Serial.print(" | Total Weight: "); Serial.println(totalWeight);

      newDataReady = 0;
      t = millis();
    }
  }

  // Command to tare all load cells by sending 't'
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') {
      LoadCell1.tareNoDelay();
      LoadCell2.tareNoDelay();
      LoadCell3.tareNoDelay();
      LoadCell4.tareNoDelay();
    }
  }

  // Check if tare operation is complete for each load cell
  if (LoadCell1.getTareStatus()) Serial.println("Tare load cell 1 complete.");
  if (LoadCell2.getTareStatus()) Serial.println("Tare load cell 2 complete.");
  if (LoadCell3.getTareStatus()) Serial.println("Tare load cell 3 complete.");
  if (LoadCell4.getTareStatus()) Serial.println("Tare load cell 4 complete.");
}
