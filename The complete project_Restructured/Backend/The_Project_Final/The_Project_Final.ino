#include <ESP8266WiFi.h> // Deal with WiFi
#include <Wire.h> // Deal with MPU via I2C
#include <ArduinoJson.h> // Encode and decode JSON

#include "mpu_control.h"

//------------------------ Config MPU -----------------------------

// Select SDA and SCL pins for I2C communication 
const uint8_t scl = D6;
const uint8_t sda = D7;

// sensitivity scale factor respective to full scale setting provided in datasheet 
const uint16_t AccelScaleFactor = 16384;
const uint16_t GyroScaleFactor = 131;

//int16_t AccelX, AccelY, AccelZ, Temperature, GyroX, GyroY, GyroZ;
// ---------------------------------------------------------------------------------------

const char* ssid = "Arboretum_Statehouse_netpap";
const char* password = "statehouse@123";

int connectPin = D1;
int clientPin = D5;
WiFiServer server(80); //Port 80

StaticJsonBuffer<200> jsonBuffer; // For creating JSON. 200 bytes
JsonObject& values = jsonBuffer.createObject();


// ------------------------------ Prepare data for sending ---------------
String prepData(char toSend[]){
   String pg =
     String("HTTP/1.1 200 OK\r\n") +
            "Content-Type: application/json\r\n" +
            "\r\n" +
            "The updated data: " + toSend +
            "\r\n\n";

  return pg;
}

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
    double tempAx, tempAy, tempAz, temptemp, tempGx, tempGy, tempGz; 
    tempAx = tempAy = tempAz = temptemp = tempGx = tempGy = tempGz = 0.00; // For checking if data is new
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

          if((Ax - tempAx > 0.5) || (Ay - tempAy > 0.5) || (Az - tempAz > 0.5) || (Gx - tempGx > 0.5) || (Gy - tempGy > 0.5) || (Gz - tempGz > 0.5)){// Only send data if it has changed
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
                     
           // Update for comparison
           tempAx = Ax;
           tempAy = Ay;
           tempAz = Az;
           tempGx = Gx;
           tempGy = Gy;
           tempGz = Gz;

           // Send the data
           client.println(prepData(jsonMessage));
           client.flush();
           //delay(1000); //Check data every 1sec
          }

    yield(); // Needed due to how NodeMCU handles while()
    }

    
  }else{
    Serial.print("!");
    digitalWrite(clientPin, LOW);
    client.stop();
    delay(1000);
  }
}












