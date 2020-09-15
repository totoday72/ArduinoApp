
//-------------------------------------------------------------------------
//------------------------ Settings-----------------------------------
//-------------------------------------------------------------------------
char ssid[] = "sdfsadfasdfasdfasdfasdfasdfasdfa"; //  Change this to your network SSID (name).
char pass[] = "KKBGVVJK";    // Change this your network password

//------ MQTT broker settings and topics
const char* broker = "mqtt.thingspeak.com"; //Server for mqtt app data
char mqttUserName[] = "Totoday72";       // Can be any name.  
char mqttPass[] = "EV0EFZGZ6PL29HBH";   // Change this your MQTT API Key from Account > MyProfile.  

//-- published settings
char publish_writeAPIKey[] = "P6CW3EZ9W7HS2MHA";// Change to your channel Write API Key.
long publishChannelID = 1129609;       // numero de canal donde se va a guardar los datos        
String publishTopic ="channels/" + String( publishChannelID ) + "/publish/"+String(publish_writeAPIKey);        

//-- subscribed settings Virtuino command 1   
long suscribeChannelID_1 = 1117297; // numero de canal donde se van a leeer los datos que se guardaran con la app movil
String subscribeTopicFor_Command_1="channels/"+String(suscribeChannelID_1)+"/subscribe/fields/field1";   

//-- subscribed settings Virtuino command 2   
//long suscribeChannelID_2 = 123456;
//String subscribeTopicFor_Command_2="channels/"+String(suscribeChannelID_2)+"/subscribe/fields/field1";   // motor

const unsigned long postingInterval = 20L * 1000L; // Post data every 20 seconds.

//------------------------ Variables-----------------------------------
//-------------------------------------------------------------------------
//#include <ESP8266WiFi.h>  // for ESP8266 enable this line and disable the next
//#include <WiFi.h>           // for ESP32 enable this line and disable the previous
#include "WiFiEsp.h" // for ESP8266 with arduino Nano or Mega
#include <MQTTClient.h>
#include "SoftwareSerial.h"

WiFiEspClient net;
MQTTClient client;
SoftwareSerial Serial1(11, 12); // pins RX, TX from ESP
#define ESP_BAUDRATE  9600
unsigned long lastUploadedTime = 0;

//========================================= connect
void connect() {
  Serial.print("\nconnecting to wifi.");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(2000);
 }
  //--- create a random MQTT client id
  char clientID[] ="ESP8266_0000000000";  // For random generation of client ID.
  for (int i = 8; i <18 ; i++) clientID[i]=  char(48+random(10));
    
  Serial.print("\nconnecting to broker...");
  while (!client.connect(clientID,mqttUserName,mqttPass)) {
    Serial.print(".");
    delay(2000);
  }
  Serial.println("\nMQTT broker nconnected!");
  
  client.subscribe(subscribeTopicFor_Command_1);
  //client.subscribe(subscribeTopicFor_Command_2);
  
}

//========================================= messageReceived
void messageReceived(String &topic, String &payload) {
 Serial.println("incoming: " + topic + " - " + payload);
 
  //-- check for Virtuino Command 1
  if (topic==subscribeTopicFor_Command_1){
        Serial.println("Command 1 = "+payload);
        int v = payload.toInt();
        if (v>0) digitalWrite(LED_BUILTIN,HIGH);
        else digitalWrite(LED_BUILTIN,LOW);
   }

  /*//-- check for Virtuino Command 1
  if (topic==subscribeTopicFor_Command_2){
        Serial.println("Command 2 = "+payload);
   }
   */
  
}

//========================================= setup
//=========================================
void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN,OUTPUT); 
  setEspBaudRate(ESP_BAUDRATE);
  Serial1.print("AT+RST\r\n");

  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo native USB port only
  }

  Serial.print("Searching for ESP8266..."); 
  // initialize ESP module
  WiFi.init(&Serial1);
  Serial.print("Searching for ESP8266..."); 
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }
  Serial.println("found it!");
  WiFi.begin(ssid, pass); // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported by Arduino. You need to set the IP address directly.
  client.begin(broker, net);
  client.onMessage(messageReceived);
  connect();
}


//========================================= loop
//=========================================
void loop() {
  client.loop();
  delay(10);  // <- fixes some issues with WiFi stability
  if (!client.connected()){
    connect();
    return;
  }
 
  if (millis() - lastUploadedTime > postingInterval) { // The uploading interval must be > 15 seconds 
    int sensorValue_1 = random(100); // replace with your sensor value
    int sensorValue_2=random(100);  // replace with your sensor value
    //int sensorValue_3=random(100);  // if you want to use three sensors enable this line
    
    String dataText = String("field1=" + String(sensorValue_1)+ "&field2=" + String(sensorValue_2));
    //String dataText = String("field1=" + String(sensorValue_1)+ "&field2=" + String(sensorValue_2)+"&field3=" + String(sensorValue_3)); // example for publish tree sensors
    boolean result = client.publish(publishTopic, dataText);
    
    if (result) Serial.println("Data has been published succesfully"); 
    else Serial.println("Unable to publish data");
    
    lastUploadedTime = millis();
  }
 

}


void setEspBaudRate(unsigned long baudrate){
  long rates[6] = {115200,74880,57600,38400,19200,9600};

  Serial.print("Setting ESP8266 baudrate to ");
  Serial.print(baudrate);
  Serial.println("...");

  for(int i = 0; i < 6; i++){
    Serial1.begin(rates[i]);
    delay(100);
    Serial1.print("AT+UART_DEF=");
    Serial1.print(baudrate);
    Serial1.print(",8,1,0,0\r\n");
    delay(100);  
  }
    
  Serial1.begin(baudrate);
}
