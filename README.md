# COMPONENTS USED
NodeMCU - Think of it as a WiFi module + GPIO pins OR Arduino with integrated WiFi
MPU6050 - Gyroscope + Accelerometer + Thermometer. All use MEMS technology
A bunch or wires and LEDs


# FOLDERS
## Relevant material/
Make sure you read the PDF inside it and: 
1. http://www.esp8266-projects.com/2015/12/mailbag-mpu6050-module-i2c-driver-init.html/
2. https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/server-examples.html

## LIBRARIES NEEDED BY ARDUINO/
For the NodeMCU to connect and all to work properly, the libraries in here need to be transfered to Arduino Libraries folder.

## The complete project\Backend\The_Project_Final
Contains one .ino file to be run on the NodeMCU. This turns the NodeMCU into a TCP server which sends data to a client every 1sec
**You should know: ** The NodeMCU lacks a easily usable library to work with MPU6050, hence code to do direct I2C communication had to be written

# USAGE
1. Prepare the environment
2. Perform connections:
```
SCL - D6
SDA - D7

LED to show WiFi connected - D1
LED to show client connected - D5

```
3. Run the code on NodeMCU
4. Connect a phone or PC to the same WiFi as NodeMCU
5. Access '192.168.88.90' on your device. Data should start streaming in.

# TODO
1. Create a VueJS frontend that queries for data from the TCP server periodically and updates a table


