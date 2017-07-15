/*
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
// char ssid[] = "workshop";
// char wifiPassword[] = "workshop";
char ssid[] = "Kercem2";
char wifiPassword[] = "E0E3106433F4";

const char* mqtt_server = "192.168.0.36";
char msg[100];
const char *topicToPublish = "Chillis2"; //mqtt topic for moisture
const char *pumpTopic = "Pump_2"; //mqtt topic for pump status
char *pumpStatus = "";


WiFiClient espClient;
PubSubClient client(espClient);

// ======================================================================
// PUMP VARIABLES - INTERVAL AND DURATION
//number of mS between pump running 86400000 is 1 day
const unsigned long timeBetweenWatering = 60 * 60 * 1000;
unsigned long pumpRunDuration = 15000;
// ======================================================================

unsigned long currentMillis = timeBetweenWatering;
unsigned long previousMillis = 0;
unsigned long pumpStartMillis;

#define pump1Pin D1
#define pump2Pin D2
#define sensorVoltagePin D7

//Prototpye function definitions
void publish(const char *topic, int data1);
void publishPump(const char *topic, char *pumpStatus);
int getMoisture();
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);


void setup(){
Serial.begin(9600);
Serial.println("Starting.....");
pinMode(pump1Pin, OUTPUT);
pinMode(pump2Pin, OUTPUT);
pinMode(sensorVoltagePin, OUTPUT);
pinMode(BUILTIN_LED, OUTPUT);
//turn off both pumps
digitalWrite(pump1Pin,LOW);
digitalWrite(pump2Pin,LOW);
digitalWrite(sensorVoltagePin, LOW);

// Connect to WiFi
WiFi.begin(ssid, wifiPassword);
  while (WiFi.status() != WL_CONNECTED) {
    // Serial.print(".");
    delay(200);
  }
Serial.print("Connected, IP = ");
Serial.println(WiFi.localIP());

client.setServer(mqtt_server, 1883);
client.setCallback(callback);

}//end setup

void loop() {
currentMillis = millis();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
//check if it is time to add water
if (currentMillis - previousMillis >= timeBetweenWatering){
	// first check if there is water in the tray
  if (getMoisture() > 0){
    //run pump1 for the set period
    Serial.print("Pump1 started\t");
    pumpStatus = "Started pump 1";
    publishPump(pumpTopic, pumpStatus);
    digitalWrite(pump1Pin, HIGH);
    delay(pumpRunDuration);
    //turn off pump1
    digitalWrite(pump1Pin, LOW);
    pumpStatus = "Stopped pump 1";
    publishPump(pumpTopic, pumpStatus);
    Serial.println("Pump1 stopped");
    //run pump2 for the set period
    pumpStartMillis = millis();
    Serial.print("Pump2 started\t");
    pumpStatus = "Started pump 2";
    publishPump(pumpTopic, pumpStatus);
    digitalWrite(pump2Pin, HIGH);
    delay(pumpRunDuration);
    //turn off pump2
    digitalWrite(pump2Pin, LOW);
    pumpStatus = "Stopped pump 2";
    publishPump(pumpTopic, pumpStatus);
    Serial.println("Pump2 stopped");
  }//end of adding water block if moisure reading is low enough
  previousMillis = currentMillis;
}//end of block if it is time to water
}//end of loop

// ======================================================
void publishPump(const char *topic, char *pumpStatus){
  if (!client.connected()) {
        reconnect();
  }
  //construct the JSON string to send
  snprintf (msg, 100, "{\"Pump\": \"%s\"}", pumpStatus);
  // send the message
  client.publish(topic, msg);
  } // end publishPump

void publish(const char *topic, int data1) {
  if (!client.connected()) {
      reconnect();
  }
  //construct the JSON string to send
  snprintf (msg, 100, "{\"sensor_ID\": \"Chillis\",\"Moisture\": %d}", data1);
  // send the message
  client.publish(topic, msg);
} //end of publish

//=========================================================

int getMoisture() {
  //apply voltage to the moisture sensor
  digitalWrite(sensorVoltagePin, HIGH);
  delay(1000);
  // takes 10 readings separated by 500mS and averages them
  int cumulativeLevel = 0;
  for (int i; i<10; i++){
    cumulativeLevel = cumulativeLevel + analogRead(A0);
    delay(100);
  }
  //turn off voltage to moisture sensor
  digitalWrite(sensorVoltagePin, LOW);
  int averageLevel = cumulativeLevel / 10;
  //send the values to the mqtt broker
  publish(topicToPublish, averageLevel);
  return averageLevel;
} //end of getSensorReadings

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    // Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("Chilli_Waterer2")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      // client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("startpump");
    } 
    else {
      // Serial.print("failed, rc=");
      // Serial.print(client.state());
      // Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }// end if
    delay(1000);
  }// end while
}// end reconnect

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
    pumpStatus = "Started pump 1";
    publishPump(pumpTopic, pumpStatus);
    digitalWrite(pump1Pin, HIGH);
    delay(pumpRunDuration);
    //turn off pump1
    digitalWrite(pump1Pin, LOW);
    pumpStatus = "Stopped pump 1";
    publishPump(pumpTopic, pumpStatus);
    digitalWrite(BUILTIN_LED, HIGH);
  }

  if ((char)payload[0] == '2') {
    digitalWrite(BUILTIN_LED, LOW);  
    pumpStatus = "Started pump 2";
    publishPump(pumpTopic, pumpStatus);
    digitalWrite(pump2Pin, HIGH);
    delay(pumpRunDuration);
    //turn off pump1
    digitalWrite(pump2Pin, LOW);
    pumpStatus = "Stopped pump 2";
    publishPump(pumpTopic, pumpStatus);
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by ma
  }
}

