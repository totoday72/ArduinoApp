
#include <Wire.h>
#include <Adafruit_MLX90614.h>

#include <SoftwareSerial.h>
#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
SoftwareSerial mySoftwareSerial(3, 2); // RX, TX

//*******************TECLADO *****************
#include "Adafruit_Keypad.h"

const byte ROWS = 4; // rows
const byte COLS = 4; // columns
//define the symbols on the buttons of the keypads
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {36, 34, 32, 30}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {28, 26, 24, 22};  //22 AZUL MARCADO CON 1 connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Adafruit_Keypad teclado = Adafruit_Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS);

//*****************TECLADO**************************



DFRobotDFPlayerMini mp3;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
//sensor ultrasonico
#define triggerUltrasonico 6  //pin trigger del ultrasonico
#define echoUltrasonico 5     //pin echo del ultrasonico

void setup() {
  teclado.begin();
  pinMode(triggerUltrasonico, OUTPUT);
  pinMode(echoUltrasonico, INPUT);
  Serial.begin(9600);
  pinMode(4, INPUT);
  mlx.begin();
  mySoftwareSerial.begin(9600);
  mp3.begin(mySoftwareSerial);   //Use softwareSerial to communicate with mp3.
  mp3.setTimeOut(500); //Set serial communictaion time out 500ms
  //----Set volume----
  mp3.volume(10);  //Set volume value (0~30).
  mp3.volumeUp(); //Volume Up
  mp3.volumeDown(); //Volume Down
  mp3.EQ(DFPLAYER_EQ_ROCK);
  mp3.outputDevice(DFPLAYER_DEVICE_SD);
  //myDFPlayer.play(18);  //Play the first mp3
  //myDFPlayer.randomAll(); //Random play all the mp3.
  //myDFPlayer.advertise(3);
}
int tiemponumeros = 900;
const int tempambiente = 2;
const int tempcorporal = 1;

void loop() {

  float distancia = medirDistancia();
  if (distancia < 15) {
    String Usuario = ingresarUsuario();
    Serial.println("Su usuario es:" + Usuario);
    delay(1500);
    decirTemperatura(tempcorporal); //temperatura corporal 1 y !=1 temperatura ambiente
  }
  //delay(1000);

}


//***************** INGRESAR USUARIO ****************************
String ingresarUsuario() {
  mp3.play(28);
  char caracter = 'x';
  String cadena = "";
  boolean seguro = false;
  while (!seguro) {
    caracter = 'x';
    cadena = "";
    while (caracter != '*') {
      teclado.tick();
      Serial.println("Usuario es:" + cadena);
      if (teclado.available()) {
        keypadEvent e = teclado.read();
        if (e.bit.EVENT == KEY_JUST_PRESSED) {
          caracter = (char)e.bit.KEY;
          if (caracter == '#') {
            if (cadena.length() > 0) {
              cadena = cadena.substring(0, cadena.length() - 1);
            }
          } else if (caracter != '*' ) {
            if (caracter != '#') {
              cadena += caracter;
            }
          }
          Serial.println(" pressed");
        } else if (e.bit.EVENT == KEY_JUST_RELEASED) {
          sonidotecla(caracter);
        }
      }
    }
    caracter = 'x';
    mp3.play(35);
    delay(tiemponumeros);
    for (int i = 0; i < cadena.length(); i++) {
      sonidotecla(cadena.charAt(i));
    }
    mp3.play(21);
    delay(tiemponumeros);
    while (caracter != '*') {
      teclado.tick();
      Serial.println("Usuario es:" + cadena);
      if (teclado.available()) {
        keypadEvent e = teclado.read();
        if (e.bit.EVENT == KEY_JUST_PRESSED) {
          caracter = (char)e.bit.KEY;
          if (caracter == '*') {
            seguro = true;
          } else if (caracter == '#') {
            caracter = 'x';
            cadena = "";
            mp3.play(28);
            break;
          }
          Serial.println(" pressed");
        } else if (e.bit.EVENT == KEY_JUST_RELEASED) {
          sonidotecla(caracter);
        }
      }
    }
  }
  return cadena;
}

//***************** INGRESAR USUARIO ****************************


//***************** DISTANCIA ****************************
//***************** DISTANCIA ****************************
float medirDistancia() {
  long duracion;
  float distancia = 0;
  digitalWrite(triggerUltrasonico, LOW);
  delayMicroseconds(5);
  digitalWrite(triggerUltrasonico, HIGH);
  //delayMicroseconds(1000);
  delayMicroseconds(10);
  digitalWrite(triggerUltrasonico, LOW);
  duracion = pulseIn(echoUltrasonico, HIGH);
  distancia = duracion * 0.01768115942 ;
  Serial.print ("Distancia en Promedio: ");
  Serial.println(distancia);
  return distancia;
}

//***************** DISTANCIA ****************************
//***************** DISTANCIA ****************************

//***************** SONIDO ****************************
//***************** SONIDO ****************************

void sonidotecla(char tecla) {
  switch (tecla) {
    case '1':
      mp3.play(40);
      break;
    case '2':
      mp3.play(17);
      break;
    case '3':
      mp3.play(39);
      break;
    case '4':
      mp3.play(12);
      break;
    case '5':
      mp3.play(9);
      break;
    case '6':
      mp3.play(31);
      break;
    case '7':
      mp3.play(32);
      break;
    case '8':
      mp3.play(26);
      break;
    case '9':
      mp3.play(24);
      break;
    case '0':
      mp3.play(7);
      break;
    case '*':
      mp3.play(3);
      break;
    case '#':
      mp3.play(25);
      break;
    case 'A':
      mp3.play(2);
      break;
    case 'B':
      mp3.play(4);
      break;
    case 'C':
      mp3.play(5);
      break;
    case 'D':
      mp3.play(13);
      break;
  }
  delay(tiemponumeros);

}

void decirTemperatura(int modo) {
  int tempObjeto = 0;
  if (modo == 1) {
    mp3.play(33);
    delay(2000);
    tempObjeto = int(mlx.readObjectTempC());
  } else {
    mp3.play(36);
    delay(2000);
    tempObjeto = int(mlx.readAmbientTempC());
  }


  if (tempObjeto <= 15) {
    reproducirSonidoNumero(tempObjeto, tempObjeto);
  } else if (tempObjeto < 20) {
    reproducirSonidoNumero(tempObjeto - 10, tempObjeto);
  } else if (tempObjeto < 30) {
    reproducirSonidoNumero(tempObjeto - 20, tempObjeto);
  } else if (tempObjeto < 40) {
    reproducirSonidoNumero(tempObjeto - 30, tempObjeto);
  } else if (tempObjeto < 50) {
    reproducirSonidoNumero(tempObjeto - 40, tempObjeto);
  } else if (tempObjeto < 60) {
    reproducirSonidoNumero(tempObjeto - 50, tempObjeto);
  } else {
    mp3.play(14);
  }
  delay(1500);
  Serial.println(analogRead(4));
}

void reproducirSonidoNumero(int sonido, int numero) {
  if (sonido == 0) {
    if (numero == 20) {
      mp3.play(45);
    } else if (numero == 30) {
      mp3.play(44);
    } else if (numero == 40) {
      mp3.play(43);
    } else if (numero == 50) {
      mp3.play(42);
    }
  } else {
    if (numero < 20) {
      mp3.play(14);
      delay(tiemponumeros);
    } else if (numero < 30) {
      mp3.play(41);
      delay(tiemponumeros);
    } else if (numero < 40) {
      mp3.play(38);
      delay(tiemponumeros);
    } else if (numero < 50) {
      mp3.play(11);
      delay(tiemponumeros);
    } else if (numero < 60) {
      mp3.play(10);
      delay(tiemponumeros);
    }
    switch (sonido) {
      case 0:
        mp3.play(7);
        break;
      case 1:
        mp3.play(40);
        break;
      case 2:
        mp3.play(17);
        break;
      case 3:
        mp3.play(39);
        break;
      case 4:
        mp3.play(12);
        break;
      case 5:
        mp3.play(9);
        break;
      case 6:
        mp3.play(31);
        break;
      case 7:
        mp3.play(32);
        break;
      case 8:
        mp3.play(26);
        break;
      case 9:
        mp3.play(24);
        break;
      case 10:
        mp3.play(15);
        break;
      case 11:
        mp3.play(27);
        break;
      case 12:
        mp3.play(16);
        break;
      case 13:
        mp3.play(37);
        break;
      case 14:
        mp3.play(6);
        break;
      case 15:
        mp3.play(30);
        break;
    }
  }
  Serial.println(analogRead(4));
  delay(tiemponumeros - 100);
  mp3.play(22);
  Serial.println(analogRead(4));
}

//***************** SONIDO ****************************
//***************** SONIDO ****************************
