#include <Wire.h>
#include <stdint.h>

//int pin = ;
byte byteAvailable = 0;
int r_data[4] = {0,0,0,0};
float t_temp[10];
float t_humi[10];
float t_pres[10];
float temperature;
float humidity;
float pressure;
int th_stat = 0;
int count = 0;


//conversion factors
//value provided by data sheet 2^14-2
#define TH_CONSTANT 16382.0
// 1/kPa 
#define B_PRESSURECONST (-0.4871)
// unitless 
#define A_PRESSURECONST 0.00876
//default value should be 60000 for one reading every minute(ish)
#define DELAYBETWEENREADINGS 1000
//0x27 = 39, default address of sensor
#define SENSORADDRESS 39
//notes on HIH800 operation 
//write = 0
//read = 1
//ACK = 0
//NACK = 1



void request_measurement();
void store_data(int data[], int n); //n should always be 4
void(* resetFunc) (void) = 0; //used to restart the arduino if something weird happens

float convert_humidity(int data[], int n);
float convert_temperature(int data[], int n);
float record_pressure(float r_sensor);

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Serial.begin(9600);
}

void loop() {

SOFT_RESTART:
  request_measurement(); //Request new data every time the loop starts
  delay(DELAYBETWEENREADINGS); // wait for readings to occur, data sheet says will be 36.65 ms, though increase to not be spammed to all hell
  store_data(r_data, 4);

  if (count >= 9) {
    count = 0;
    for(int i = 0; i < 10; i++) {
      temperature = temperature + t_temp[i];
      humidity = humidity + t_humi[i];
      pressure = pressure + t_pres[i];
    }

    temperature = temperature/10;
    humidity = humidity/10;
    pressure = pressure/10;

    //Print the values
    Serial.print("Data:\n");

    //turn the address of the float into a pointer, and then write all the bytes
    Serial.write((byte *)&temperature,sizeof(temperature));
    Serial.print("\n");
    Serial.write((byte *)&humidity,sizeof(humidity));
    Serial.print("\n");
    Serial.write((byte *)&pressure,sizeof(pressure));
    Serial.print("\n");
  }

  //Check the status of the data: 00 valid, 01 stale, 1X reset as somethings wrong
  th_stat = r_data[0] >> 6;
  if(th_stat == 0x01) {
    //Serial.println ("Stale data, resending request.");
    goto SOFT_RESTART;
  }

  else if(th_stat == 0x00) {  //valid data to read so convert to usable
    t_temp[count] = convert_temperature(r_data, 4);
    t_humi[count] = convert_humidity(r_data, 4);
    t_pres[count] = record_pressure(analogRead(A3));
    /*//If it's activating that if statement, you can display the values early
    Serial.println("TEST STUFF");
    Serial.print("Humidity: " + String(humidity) + " %\n");
    Serial.print("Temperature: " + String(temperature) + " C\n");
    Serial.print("Pressure: " + String(pressure) + " kPa\n\n\n");
    //End of pre-display*/
    if((temperature > 120.0) || (temperature < - 40.0) || (humidity > 100.0) || (humidity < 0.0)) {
      //Serial.println ("Values were invalid or n was greater than 4, requesting new data");
      goto SOFT_RESTART;
    }

    count++;
  }
  else {
    //Serial.println("I'm scared and resetting.");
    resetFunc();
  }
}

//----------------------------------------------
//START OF FUNCTIONS, I'll add decriptions later
//----------------------------------------------


void request_measurement() {
  Wire.beginTransmission(SENSORADDRESS);
  Wire.write(byte(0x00)); // sends slave address:000 0000 and to write(0)
  Wire.endTransmission();
  delay(40); // wait for readings to occur, data sheet says will be 36.65 ms
}

//n should always be 4
void store_data(int data[], int n) {
//read first byte  
  Wire.requestFrom(SENSORADDRESS,n);
  int i = 0;//used to store received data
  while(Wire.available()) {
    if(i < n)
      r_data[i] = Wire.read();
    else
      break; //someting weird happend break the loop, prob do a new request
    
    /*// For Testing Purposes
    Serial.print("\nArray location written to and value: ");
    Serial.println(i);
    Serial.println(r_data[i], BIN); //note, does not print leading 0's
    // End of Testing Purposes */

    i++;
  }
}

float convert_humidity(int data[], int n) {
  unsigned int r_humi = -2;
  if(n <= 4) {
    r_humi = r_data[0] << 10;
    r_humi = r_humi >> 2;
    r_humi = r_humi | r_data[1];
  }
  /*// To display humidity in binary preconversion if needed
  Serial.print("Received Humidity preconversion: ");
  Serial.println(r_humi, BIN);
  // End of display humidity */
  return (r_humi / TH_CONSTANT )* 100;
}

float convert_temperature(int data[], int n) {
  unsigned int r_temp = 0xFFFF; //it's just a max number that will never be achieved normally
  if(n <= 4) {
    r_temp = r_data[2] << 8;
    r_temp = r_temp | r_data[3];
    r_temp = r_temp >> 2;
  }
  /*// To display temp in binary preconversion if needed
  Serial.print("Received Temperature preconversion: ");
  Serial.println(r_temp, BIN);
  // End of display temp */
  return ((r_temp / TH_CONSTANT) * 165.0) - 40;
}

float record_pressure(float r_sensor) {
  float v_out = r_sensor * (5.0 / 1023.0);

  //Serial.print("Output Voltage: " + String(v_out) + " V\n");
  return ((v_out / 5.0) - B_PRESSURECONST) / A_PRESSURECONST;
}

