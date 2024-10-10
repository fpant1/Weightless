#include <HX711_ADC.h>
#if defined(ESP8266) || defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

#include <LiquidCrystal_I2C.h>

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

const int calVal_eepromAdress1 = 0;
const int calVal_eepromAdress2 = 10;
const int calVal_eepromAdress3 = 20;
const int calVal_eepromAdress4 = 30;

unsigned long t = 0;

LiquidCrystal_I2C lcd(0x3F, 16, 2);

void setup() {
  Serial.begin(57600);
  delay(10);
  Serial.println();
  Serial.println("Starting...");

  LoadCell1.begin();
  LoadCell2.begin();
  LoadCell3.begin();
  LoadCell4.begin();

  unsigned long stabilizingtime = 2000;
  boolean _tare = true;

  LoadCell1.start(stabilizingtime, _tare);
  LoadCell2.start(stabilizingtime, _tare);
  LoadCell3.start(stabilizingtime, _tare);
  LoadCell4.start(stabilizingtime, _tare);

  if (LoadCell1.getTareTimeoutFlag() || LoadCell1.getSignalTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations for LoadCell1");
    while (1);
  } else {
    LoadCell1.setCalFactor(1.0);
    Serial.println("LoadCell1 startup is complete");
  }

  if (LoadCell2.getTareTimeoutFlag() || LoadCell2.getSignalTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations for LoadCell2");
    while (1);
  } else {
    LoadCell2.setCalFactor(1.0);
    Serial.println("LoadCell2 startup is complete");
  }

  if (LoadCell3.getTareTimeoutFlag() || LoadCell3.getSignalTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations for LoadCell3");
    while (1);
  } else {
    LoadCell3.setCalFactor(1.0);
    Serial.println("LoadCell3 startup is complete");
  }

  if (LoadCell4.getTareTimeoutFlag() || LoadCell4.getSignalTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations for LoadCell4");
    while (1);
  } else {
    LoadCell4.setCalFactor(1.0);
    Serial.println("LoadCell4 startup is complete");
  }

  while (!LoadCell1.update());
  while (!LoadCell2.update());
  while (!LoadCell3.update());
  while (!LoadCell4.update());

  calibrate(LoadCell1, calVal_eepromAdress1, "LoadCell1");
  calibrate(LoadCell2, calVal_eepromAdress2, "LoadCell2");
  calibrate(LoadCell3, calVal_eepromAdress3, "LoadCell3");
  calibrate(LoadCell4, calVal_eepromAdress4, "LoadCell4");

  lcd.begin();
}

void loop() {
  static boolean newDataReady1 = false;
  static boolean newDataReady2 = false;
  static boolean newDataReady3 = false;
  static boolean newDataReady4 = false;

  const int serialPrintInterval = 1000; // Adjust this value to control print frequency

  if (LoadCell1.update()) newDataReady1 = true;
  if (LoadCell2.update()) newDataReady2 = true;
  if (LoadCell3.update()) newDataReady3 = true;
  if (LoadCell4.update()) newDataReady4 = true;

  if (millis() > t + serialPrintInterval) {
    float weight1 = 0;
    float weight2 = 0;
    float weight3 = 0;
    float weight4 = 0;

    if (newDataReady1) {
      weight1 = LoadCell1.getData();
      newDataReady1 = false;
    }

    if (newDataReady2) {
      weight2 = LoadCell2.getData();
      newDataReady2 = false;
    }

    if (newDataReady3) {
      weight3 = LoadCell3.getData();
      newDataReady3 = false;
    }

    if (newDataReady4) {
      weight4 = LoadCell4.getData();
      newDataReady4 = false;
    }

    // Calculate the sum of weights only if valid data is available from all load cells
    if (weight1 != NAN && weight2 != NAN && weight3 != NAN && weight4 != NAN) {
      float sumWeights = weight1 + weight2 + weight3 + weight4;
      Serial.print("weight1 = ");
      Serial.println(weight1);
      Serial.print("weight2 = ");
      Serial.println(weight2);
      Serial.print("weight3 = ");
      Serial.println(weight3);
      Serial.print("weight4 = ");
      Serial.println(weight4);
      Serial.print("Total weight: ");
      Serial.println(sumWeights);

      lcd.setCursor(0, 1);
      lcd.print(sumWeights);
      lcd.clear();

      t = millis(); // Update the timestamp for the next print interval
    }
  }

  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') {
      LoadCell1.tareNoDelay();
      LoadCell2.tareNoDelay();
      LoadCell3.tareNoDelay();
      LoadCell4.tareNoDelay();
    } else if (inByte == 'r') {
      calibrate(LoadCell1, calVal_eepromAdress1, "LoadCell1");
      calibrate(LoadCell2, calVal_eepromAdress2, "LoadCell2");
      calibrate(LoadCell3, calVal_eepromAdress3, "LoadCell3");
      calibrate(LoadCell4, calVal_eepromAdress4, "LoadCell4");
    } else if (inByte == 'c') {
      changeSavedCalFactor(LoadCell1, calVal_eepromAdress1, "LoadCell1");
      changeSavedCalFactor(LoadCell2, calVal_eepromAdress2, "LoadCell2");
      changeSavedCalFactor(LoadCell3, calVal_eepromAdress3, "LoadCell3");
      changeSavedCalFactor(LoadCell4, calVal_eepromAdress4, "LoadCell4");
    }
  }

  if (LoadCell1.getTareStatus() == true) {
    Serial.println("LoadCell1 Tare complete");
  }
  if (LoadCell2.getTareStatus() == true) {
    Serial.println("LoadCell2 Tare complete");
  }
  if (LoadCell3.getTareStatus() == true) {
    Serial.println("LoadCell3 Tare complete");
  }
  if (LoadCell4.getTareStatus() == true) {
    Serial.println("LoadCell4 Tare complete");
  }

  while (Serial.available() > 0) {
    lcd.write(Serial.read());
  }
}

void calibrate(HX711_ADC& loadCell, int calVal_eepromAdress, const char* loadCellName) {
  Serial.println("***");
  Serial.print("Start calibration for ");
  Serial.println(loadCellName);
  Serial.println("Place the load cell on a level stable surface.");
  Serial.println("Remove any load applied to the load cell.");
  Serial.println("Send 't' from serial monitor to set the tare offset.");

  boolean _resume = false;
  while (_resume == false) {
    loadCell.update();
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 't') loadCell.tareNoDelay();
    }
    if (loadCell.getTareStatus() == true) {
      Serial.println("Tare complete");
      _resume = true;
    }
  }

  Serial.println("Now, place your known mass on the load cell.");
  Serial.println("Then send the weight of this mass (i.e. 100.0) from the serial monitor.");

  float known_mass = 0;
  _resume = false;
  while (_resume == false) {
    loadCell.update();
    if (Serial.available() > 0) {
      known_mass = Serial.parseFloat();
      if (known_mass != 0) {
        Serial.print("Known mass is: ");
        Serial.println(known_mass);
        _resume = true;
      }
    }
  }

  loadCell.refreshDataSet();
  float newCalibrationValue = loadCell.getNewCalibration(known_mass);

  Serial.print("New calibration value for ");
  Serial.print(loadCellName);
  Serial.print(" has been set to: ");
  Serial.print(newCalibrationValue);
  Serial.println(", use this as calibration value (calFactor) in your project sketch.");
  Serial.print("Save this value to EEPROM address ");
  Serial.print(calVal_eepromAdress);
  Serial.println("? y/n");

  _resume = false;
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
#if defined(ESP8266) || defined(ESP32)
        EEPROM.begin(512);
#endif
        EEPROM.put(calVal_eepromAdress, newCalibrationValue);
#if defined(ESP8266) || defined(ESP32)
        EEPROM.commit();
#endif
        EEPROM.get(calVal_eepromAdress, newCalibrationValue);
        Serial.print("Value ");
        Serial.print(newCalibrationValue);
        Serial.print(" saved to EEPROM address: ");
        Serial.println(calVal_eepromAdress);
        _resume = true;
      } else if (inByte == 'n') {
        Serial.println("Value not saved to EEPROM");
        _resume = true;
      }
    }
  }

  Serial.println("End calibration for ");
  Serial.println(loadCellName);
  Serial.println("***");
  Serial.println("To re-calibrate, send 'r' from serial monitor.");
  Serial.println("For manual edit of the calibration value, send 'c' from serial monitor.");
  Serial.println("***");
}

void changeSavedCalFactor(HX711_ADC& loadCell, int calVal_eepromAdress, const char* loadCellName) {
  float oldCalibrationValue = loadCell.getCalFactor();
  boolean _resume = false;
  Serial.println("***");
  Serial.print("Current calibration value for ");
  Serial.print(loadCellName);
  Serial.print(" is: ");
  Serial.println(oldCalibrationValue);
  Serial.println("Now, send the new value from serial monitor, i.e. 696.0");

  float newCalibrationValue;
  while (_resume == false) {
    if (Serial.available() > 0) {
      newCalibrationValue = Serial.parseFloat();
      if (newCalibrationValue != 0) {
        Serial.print("New calibration value for ");
        Serial.print(loadCellName);
        Serial.print(" is: ");
        Serial.println(newCalibrationValue);
        loadCell.setCalFactor(newCalibrationValue);
        _resume = true;
      }
    }
  }
  _resume = false;
  Serial.print("Save this value to EEPROM address ");
  Serial.print(calVal_eepromAdress);
  Serial.println("? y/n");

  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
#if defined(ESP8266) || defined(ESP32)
        EEPROM.begin(512);
#endif
        EEPROM.put(calVal_eepromAdress, newCalibrationValue);
#if defined(ESP8266) || defined(ESP32)
        EEPROM.commit();
#endif
        EEPROM.get(calVal_eepromAdress, newCalibrationValue);
        Serial.print("Value ");
        Serial.print(newCalibrationValue);
        Serial.print(" saved to EEPROM address: ");
        Serial.println(calVal_eepromAdress);
        _resume = true;
      } else if (inByte == 'n') {
        Serial.println("Value not saved to EEPROM");
        _resume = true;
      }
    }
  }

  Serial.println("End change calibration value for ");
  Serial.println(loadCellName);
  Serial.println("***");
}
