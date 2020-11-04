
#include <Wire.h>
#include <Adafruit_MLX90614.h>

#include <SoftwareSerial.h>
#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
SoftwareSerial mySoftwareSerial(3,2); // RX, TX

DFRobotDFPlayerMini myDFPlayer;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();


void setup() {
  Serial.begin(9600);
  mlx.begin();  
  mySoftwareSerial.begin(9600);
  myDFPlayer.begin(mySoftwareSerial);   //Use softwareSerial to communicate with mp3.
  myDFPlayer.setTimeOut(500); //Set serial communictaion time out 500ms
  //----Set volume----
  myDFPlayer.volume(5);  //Set volume value (0~30).
  myDFPlayer.volumeUp(); //Volume Up
  myDFPlayer.volumeDown(); //Volume Down
  myDFPlayer.EQ(DFPLAYER_EQ_ROCK);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
  myDFPlayer.play(2);  //Play the first mp3
  //myDFPlayer.randomAll(); //Random play all the mp3.
}

void loop() {
  Serial.print("Ambient = "); Serial.print(mlx.readAmbientTempC()); 
  Serial.print("*C\tObject = "); Serial.print(mlx.readObjectTempC()); Serial.println("*C");
  //Serial.print("Ambient = "); Serial.print(mlx.readAmbientTempF()); 
  //Serial.print("*F\tObject = "); Serial.print(mlx.readObjectTempF()); Serial.println("*F");
  //boolean play_state = digitalRead(12);
  //if(play_state == LOW){
    //mp3_play();
  //}
  Serial.println();
  delay(500);
}
