# Gas cylinder monitoring system with GSM alert

*Oxygen gas-cylinder monitoring system with GSM alert*
   The known mass cylinder is placed on load-cell, as the gas gets used up its(cylinder) weight decreases.
   When the approximate gas available is about **10%**, the resiponsible personels to change the cylinder are  notified via sms
   also a buzzer gives a notification to alert a person

   NOTE: The initial weight of the empty cylinder is determined in oreder to get actual weight of the gas.
                This value will be deducted from load cell readings
                

   Using the calibration example that comes with HX711_ADC library, calibrate the load cell. 
   modified read load cell example to read the value of the cell

   Read load cell... then send notification

   **Note: i2c pins on arduino uno and nano are**
   
   sck---> A5
   
   dt----> A4


   Using built-in I2C scanner example *(File->Examples->Wire->i2c scanner)*, 
   scan its I2C address(this address is used when creating an instance of LiquidCrystal_I2C)


**Components used**
  - Arduino Nano
  - Load cell(5Kg) + HX711 Amplifier
  - SPST switch
  - 9V battery
  - GSM module
  - LCD(I2C)
  
  
  Complete project image
  
![Complete project image](https://github.com/jovyinny/Gas-cylinder-monitoring-system/blob/master/load%20cell%20measurement%20with%20gsm%20alert%20.jpg)
