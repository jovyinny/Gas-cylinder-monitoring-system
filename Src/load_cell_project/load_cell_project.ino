/*
   Oxygen gas-cylinder monitoring system with GSM alert
   The known mass cylinder is placed on load-cell, as the gas gets used up its(cylinder) weight decreases.
   When the approximate gas available is about 10%, the resiponsible personels to change the cylinder are  notified via sms
   also a buzzer gives a notification to alert a person

   NOTE: The initial weight of the empty cylinder is determined in oreder to get actual weight of the gas.
                This value will be deducted from load cell readings

   Using the calibration example that comes with HX711_ADC library, calibrate the load cell. 
   modified read load cell example to read the value of the cell

   Read load cell... then send notification for values indicating less than 75% reduction

   **Note: i2c pins on arduino uno and nano are
   sck---> A5
   dt----> A4

   Using built-in I2C scanner example(File->Examples->Wire->i2c scanner)
   scan its I2C address(this address is used when creating an instance of LiquidCrystal_I2C)

   =============Components used=================
   Board used: Arduino NANO
   Sensor:        Load-cell(5kg for demonstration)
   Other:          GSM module
                        Buzzer+ 1k resistor
                        I2C LCD

*/

#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

//some global variable declaration and assignment...
const int HX711_dout = 4;                                       //mcu > HX711 dout pin
const int HX711_sck = 5;                                        //mcu > HX711 sck pin
const int col = 16, row = 2, buzzer = 7;
const int calVal_eepromAdress = 0;
unsigned long t = 0;

float initial_reading, sensor_readings,  initial_cylinder_weight, previous_percent;
String notification = "";
String phone_number[] =  {"+255xxxxxxxxx", "+255xxxxxxxx", "+255xxxxxxxx", "+255xxxxxxxxxx"};
int  nums = sizeof(phone_number) / sizeof(phone_number[0]); // nums used as the total number that will receive the notification
bool sms_State = true; // to keep track to ensure sms is sent only once

//Objects:
HX711_ADC LoadCell(HX711_dout, HX711_sck);
LiquidCrystal_I2C screen(0x27, col, row);                       //i2c address has to be scanned to obtain the accurate address used
SoftwareSerial gsm_module(2, 3);                                //GSM Tx & Rx is connected to Arduino #7 & #8

//custom function declaration....
void send_message(float);
float read_load_cell();
void update_lcd(float, float);
void send_on_status();


void setup()
{
  Serial.begin(115200);
  gsm_module.begin(115200);
  screen.begin();
  screen.backlight();
  pinMode(buzzer, OUTPUT);

  screen.clear();
  screen.setCursor(3, 0);
  screen.print("PROJECT BY:");
  screen.setCursor(5, 1);
  screen.print("JOVINE");
  delay(2000);
  screen.clear();
  screen.setCursor(1, 1);
  screen.print("Calibrating...");

  LoadCell.begin();
  float calibrationValue;

  EEPROM.get(calVal_eepromAdress, calibrationValue);    // fetch the calibration value from eeprom
  unsigned long stabilizingtime = 2000;                 // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = true;                                 //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell.setCalFactor(calibrationValue);
    screen.clear();
    screen.setCursor(1, 1);
    screen.print("Done Calibrating");
  }

  //Read its initial value gram values( used 1.5Kg bottle with empty bottle weighing 80g)
  initial_reading = 1500;
  initial_cylinder_weight = 80;

  sensor_readings = read_load_cell();
  send_on_status();

  delay(1000);
  screen.clear();
  screen.setCursor(0, 0);
  screen.print("INITIAL WEIGHT:");
  screen.setCursor(0, 1);
  screen.print(initial_reading);
  screen.print(" grams");
  delay(2000);
  screen.clear();
}


void loop()
{
  sensor_readings = read_load_cell();
  float value_mapped = map(sensor_readings, 0, initial_reading, 0, 100);

  if (value_mapped <= 10 && sms_State) {
    update_lcd(value_mapped, sensor_readings);
    notification = "Nearly empty\n Come change the cylinder";
    screen.clear();
    screen.setCursor(0, 0);
    screen.print("SENDING TEXT:");
    send_message(sensor_readings);
    tone(buzzer, 250, 2000);
    sms_State = false;
  }
  else {
    //allow sending sms again
    update_lcd(value_mapped, sensor_readings);
    sms_State = true;
  }
}


// custom function definition
float read_load_cell() {
  float  readings;
  static boolean newDataReady = 0;
  if (LoadCell.update()) newDataReady = true;

  if (newDataReady) {
    readings = LoadCell.getData();
    newDataReady = 0;
  }
  return (readings - initial_cylinder_weight);

}


void send_message(float percentage) {
  gsm_module.println("AT+CMGF=1");                                 // Configuring into TEXT mode
  delay(500);
  for (int i = 0; i < nums; i++) {
    gsm_module.println("AT+CMGS=\"" + phone_number[i] + "\"\r");
    delay(500);
    //text content
    gsm_module.print("PROJECT BY JOVINE:\n");
    gsm_module.print("SYSTEM PERCENTAGE: ");
    gsm_module.print(percentage);
    gsm_module.print("%");
    gsm_module.print("\n");
    gsm_module.print(notification);
    gsm_module.write(char(26));
    delay(2000); // delay to prepare module to send another sms
  }
}


void update_lcd(float percentage, float weight) {
  //  update LCD whenever there is a change in the value and having deviation of  (+/-)2 from previous value
  if (previous_percent != percentage  && abs(previous_percent - percentage) > 2) {
    //  screen.clear();
    screen.setCursor(0, 0);
    screen.print("Weight:  ");
    screen.print(weight);
    screen.print(" g");
    screen.setCursor(0, 1);
    screen.print("Percentage: ");
    screen.print(percentage);
    previous_percent = percentage;
  }
}


void send_on_status() {
  for (int i = 0; i < nums; i++) {
    gsm_module.println("AT+CMGF=1");
    delay(500);
    gsm_module.println("AT+CMGS=\"" + phone_number[i] + "\"\r");
    delay(500);
    gsm_module.print("PROJECT BY JOVINE:\n");
    gsm_module.print("SYSTEM IS ON: ");
    gsm_module.println(char(26));
    delay(3000);
  }
}
