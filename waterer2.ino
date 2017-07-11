/*
TEST VERSION
Designed for a Wemos D1
Pumps water to TWO separate plants are pre-determined intervals.
Connect pumps via FET switch to D1 and D2
Published pump start and stop messages to MQTT broker
Added a moisture detector
Robin Harris
4th July 2017
*/

#include <Wire.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// char ssid[] = "C4Di_Management";
// char wifiPassword[] = "c4d1manag3m3nt4747";
char ssid[] = "workshop";
char wifiPassword[] = "workshop";

const char* mqtt_server = "192.168.0.36";
char msg[100];
const char *topicToPublish = "Home/chillis"; //mqtt topic for moisture
const char *pumpTopic = "Pump"; //mqtt topic for pump status
char *pumpStatus = "";


WiFiClient espClient;
PubSubClient client(espClient);

// ======================================================================
// PUMP VARIABLES - INTERVAL AND DURATION
//number of mS between pump running 86400000 is 1 day
//for testing use a short value e.g.60000 = 1 minute (Pump runs for 15s)
const unsigned long timeBetweenWatering = 30 * 1000;
unsigned long pumpRunDuration = 5000;
// ======================================================================

unsigned long currentMillis;
unsigned long previousMillis =0;
unsigned long pumpStartMillis;

#define pump1Pin D1
#define pump2Pin D2

//Prototpye function definitions
void publish(const char *topic, int data1);
void publishPump(const char *topic, char *pumpStatus);
int getMoisture();

void setup(){
Serial.begin(9600);
pinMode(pump1Pin, OUTPUT);
pinMode(pump2Pin, OUTPUT);
//turn off both pumps
digitalWrite(pump1Pin,LOW);
digitalWrite(pump2Pin,LOW);

// Connect to WiFi
// WiFi.begin(ssid, wifiPassword);
//   while (WiFi.status() != WL_CONNECTED) {
//     // Serial.print(".");
//     delay(200);
//   }

client.setServer(mqtt_server, 1883);

}//end setup

void loop() {
currentMillis = millis();
//check if it is time to add water
if (currentMillis - previousMillis >= timeBetweenWatering){
	// first check if there is water in the tray
  if (getMoisture() < 500){
    //run pump1 for the set period
    //Serial.print("Pump1 started\t");
    pumpStatus = "Started pump 1";
    publishPump(pumpTopic, pumpStatus);
    digitalWrite(pump1Pin, HIGH);
    delay(pumpRunDuration);
    //turn off pump1
    digitalWrite(pump1Pin, LOW);
    pumpStatus = "Stopped pump 1";
    publishPump(pumpTopic, pumpStatus);
    //Serial.println("Pump1 stopped");
    //run pump2 for the set period
    pumpStartMillis = millis();
    //Serial.print("Pump2 started\t");
    pumpStatus = "Started pump 2";
    publishPump(pumpTopic, pumpStatus);
    digitalWrite(pump2Pin, HIGH);
    delay(pumpRunDuration);
    //turn off pump2
    digitalWrite(pump2Pin, LOW);
    pumpStatus = "Stopped pump 2";
    publishPump(pumpTopic, pumpStatus);
    //Serial.println("Pump2 stopped");
  }//end of adding water block if moisure reading is low
  previousMillis = currentMillis;
}//end of block if it is time to water

}//end of loop

// ======================================================
void publishPump(const char *topic, char *pumpStatus){
  // while (!client.connected()) {
  //       client.connect("Chilli_Waterer");
  //       // Serial.print(".");
  //       delay(1000); //wait then retry until connected
  //     }
  //construct the JSON string to send
  snprintf (msg, 100, "{\"Pump\": \"%s\"}", pumpStatus);
  Serial.println(msg);
  // send the message
  // client.publish(topic, msg);
  }

void publish(const char *topic, int data1) {
    // connect to the mqtt broker
    // while (!client.connected()) {
    // 	client.connect("Chilli_Waterer");
    // 	// Serial.print(".");
    // 	delay(1000); //wait then retry until connected
    // }
  //construct the JSON string to send
  snprintf (msg, 100, "{\"sensor_ID\": \"Chillis\",\"Moisture\": %d}", data1);
  // send the message
  Serial.println(msg);
  // client.publish(topic, msg);
} //end of publish

//=========================================================

int getMoisture() {
  // takes 10 readings separated by 500mS and averages them
  int cumulativeLevel = 0;
  for (int i; i<10; i++){
    cumulativeLevel = cumulativeLevel + random(0,1023);
    delay(100);
  }
  int averageLevel = cumulativeLevel / 10;
  Serial.println(averageLevel);
  //send the values to the mqtt broker
  // publish(topicToPublish, averageLevel);
  return averageLevel;
} //end of getSensorReadings
