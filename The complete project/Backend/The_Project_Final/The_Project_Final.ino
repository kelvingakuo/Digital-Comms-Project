#include <ESP8266WiFi.h> // Deal with WiFi
#include <Wire.h> // Deal with MPU via I2C
#include <ArduinoJson.h> // Encode and decode JSON

//------------------------ Config MPU -----------------------------
// MPU6050 Slave Device Address
const uint8_t MPU6050SlaveAddress = 0x68;

// Select SDA and SCL pins for I2C communication 
const uint8_t scl = D6;
const uint8_t sda = D7;

// sensitivity scale factor respective to full scale setting provided in datasheet 
const uint16_t AccelScaleFactor = 16384;
const uint16_t GyroScaleFactor = 131;

// MPU6050 few configuration register addresses
const uint8_t MPU6050_REGISTER_SMPLRT_DIV   =  0x19;
const uint8_t MPU6050_REGISTER_USER_CTRL    =  0x6A;
const uint8_t MPU6050_REGISTER_PWR_MGMT_1   =  0x6B;
const uint8_t MPU6050_REGISTER_PWR_MGMT_2   =  0x6C;
const uint8_t MPU6050_REGISTER_CONFIG       =  0x1A;
const uint8_t MPU6050_REGISTER_GYRO_CONFIG  =  0x1B;
const uint8_t MPU6050_REGISTER_ACCEL_CONFIG =  0x1C;
const uint8_t MPU6050_REGISTER_FIFO_EN      =  0x23;
const uint8_t MPU6050_REGISTER_INT_ENABLE   =  0x38;
const uint8_t MPU6050_REGISTER_ACCEL_XOUT_H =  0x3B;
const uint8_t MPU6050_REGISTER_SIGNAL_PATH_RESET  = 0x68;

int16_t AccelX, AccelY, AccelZ, Temperature, GyroX, GyroY, GyroZ;
// ---------------------------------------------------------------------------------------

const char* ssid = "Arboretum_Statehouse_netpap";
const char* password = "statehouse@123";

int connectPin = D1;
int clientPin = D5;
WiFiServer server(80); //Port 80

StaticJsonBuffer<200> jsonBuffer; // For creating JSON. 200 bytes
JsonObject& values = jsonBuffer.createObject();

void setup() {
  WiFi.mode(WIFI_OFF); // Counteract issues with Wire.h
  // Initialise serial comm, LED and MPU6050
  Serial.begin(115200);
  pinMode(connectPin, OUTPUT);
  pinMode(clientPin, OUTPUT);

  Wire.begin(sda, scl);
  MPU6050_Init();

  // Connect to WiFi
  Serial.println(" ");
  Serial.print("Connecting to: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // Check if connected to WiFi
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(". ");
    digitalWrite(connectPin, LOW);
  }

  // Set NodeMCU as TCP server
  Serial.println();
  server.begin();
  Serial.println("Setting NodeMCU as TCP server...");

  // Print this server's local IP
  Serial.print("Connect via this URL as client: ");
  Serial.println(WiFi.localIP());
  digitalWrite(connectPin, HIGH);

}

void loop() {
  // Wait for client to connect i.e. user opens localIP() in browser
  WiFiClient client = server.available();
  if(client){
    Serial.println("Client connected");
    digitalWrite(clientPin, HIGH);
    while(client.connected()){
           // Read data from MPU6050 and convert to JSON-like string
           double Ax, Ay, Az, temp, Gx, Gy, Gz;
           Read_RawValue(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_XOUT_H);

          //divide each with their sensitivity scale factor
          Ax = (double)AccelX/AccelScaleFactor;
          Ay = (double)AccelY/AccelScaleFactor;
          Az = (double)AccelZ/AccelScaleFactor;
          temp = (double)Temperature/340+36.53; //temperature formula
          Gx = (double)GyroX/GyroScaleFactor;
          Gy = (double)GyroY/GyroScaleFactor;
          Gz = (double)GyroY/GyroScaleFactor;

          // Create JSON
          values["ax"] = Ax;
          values["ay"] = Ay;
          values["az"] = Az;
          values["gx"] = Gx;
          values["gy"] = Gy;
          values["gz"] = Gz;
          values["temp"] = temp;

          // Convert to string
          char jsonMessage[200];
          values.prettyPrintTo(jsonMessage, sizeof(jsonMessage));
                    
         // Send the data
         client.println(prepData(jsonMessage));
         client.flush();
    }

    
  }else{
    Serial.println("Client not connected");
    digitalWrite(clientPin, LOW);
    client.stop();
  }

  delay(1000); //Send data every 1sec
}


// Place data in a packet to for sending
String prepData(char toSend[]){
   String pg =
     String("HTTP/1.1 200 OK\r\n") +
            "Content-Type: application/json\r\n" +
            "\r\n" +
            "data:  " + toSend +
            "\r\n\n";

  return pg;
}



// ---------------------------- MPU Functions ------------------
void I2C_Write(uint8_t deviceAddress, uint8_t regAddress, uint8_t data){
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.write(data);
  Wire.endTransmission();
}

// read all 14 registers
void Read_RawValue(uint8_t deviceAddress, uint8_t regAddress){
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.endTransmission();
  Wire.requestFrom(deviceAddress, (uint8_t)14);
  AccelX = (((int16_t)Wire.read()<<8) | Wire.read());
  AccelY = (((int16_t)Wire.read()<<8) | Wire.read());
  AccelZ = (((int16_t)Wire.read()<<8) | Wire.read());
  Temperature = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroX = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroY = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroZ = (((int16_t)Wire.read()<<8) | Wire.read());

}

//configure MPU6050
void MPU6050_Init(){
  delay(150);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SMPLRT_DIV, 0x07);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_1, 0x01);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_2, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_CONFIG, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_GYRO_CONFIG, 0x00);//set +/-250 degree/second full scale
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_CONFIG, 0x00);// set +/- 2g full scale
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_FIFO_EN, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_INT_ENABLE, 0x01);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SIGNAL_PATH_RESET, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_USER_CTRL, 0x00);
}





