#include <Wire.h>
//int pin = ;
//int bytes = ;
int reading = 0;
int dev = 39;  //0x27 = 39, default address of sensor 
//notes on HIH800 operation 
//write = 0
//read = 1
//ACK = 0
//NACK = 1
void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Serial.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:
  //sending MR command
  Wire.beginTransmission(dev);
  Wire.write(byte(0x00)); // sends slave address:000 0000 and to write(0)
  Wire.endTransmission();

  delay(40); // wait for readings to occur, data sheet says will be 36.65 ms
  //read first byte  
  Wire.requestFrom(dev,1);

  if( 1 <= Wire.available() ) {
    reading = Wire.read();
  }
  Serial.println("Reading is");
  Serial.println(reading);
  
  //write acknowledge 
  //read second byte
  //write acknowledge 
    

}
