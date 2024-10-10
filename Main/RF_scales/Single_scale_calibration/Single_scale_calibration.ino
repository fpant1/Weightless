#include <HX711_ADC.h>
#if defined(ESP8266) || defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

#include <LiquidCrystal_I2C.h>

// Define pins for LoadCell1
const int HX711_dout1 = 4;
const int HX711_sck1 = 5;

// Create HX711 instance for LoadCell1
HX711_ADC LoadCell1(HX711_dout1, HX711_sck1);

const int calVal_eepromAdress1 = 0;

unsigned long t = 0;

LiquidCrystal_I2C lcd(0x3F, 16, 2);

void setup() {
  Serial.begin(57600);
  delay(10);
  Serial.println();
  Serial.println("Starting...");

  LoadCell1.begin();

  unsigned long stabilizingtime = 2000;
  boolean _tare = true;

  LoadCell1.start(stabilizingtime, _tare);

  if (LoadCell1.getTareTimeoutFlag() || LoadCell1.getSignalTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations for LoadCell1");
    while (1);
  } else {
    LoadCell1.setCalFactor(1.0); // Initial calibration factor
    Serial.println("LoadCell1 startup is complete");
  }

  while (!LoadCell1.update());

  calibrate(LoadCell1, calVal_eepromAdress1, "LoadCell1");

  lcd.begin();
}

void loop() {
  static boolean newDataReady1 = false;

  const int serialPrintInterval = 1000; // Adjust this value to control print frequency

  if (LoadCell1.update()) newDataReady1 = true;

  if (millis() > t + serialPrintInterval) {
    float weight1 = 0;

    if (newDataReady1) {
      weight1 = LoadCell1.getData();
      newDataReady1 = false;
    }

    if (weight1 != NAN) {
      Serial.print("Weight: ");
      Serial.println(weight1);

      lcd.setCursor(0, 1);
      lcd.print(weight1);
      lcd.clear();

      t = millis(); // Update the timestamp for the next print interval
    }
  }

  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') {
      LoadCell1.tareNoDelay();
    } else if (inByte == 'r') {
      calibrate(LoadCell1, calVal_eepromAdress1, "LoadCell1");
    } else if (inByte == 'c') {
      changeSavedCalFactor(LoadCell1, calVal_eepromAdress1, "LoadCell1");
    }
  }

  if (LoadCell1.getTareStatus() == true) {
    Serial.println("LoadCell1 Tare complete");
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
