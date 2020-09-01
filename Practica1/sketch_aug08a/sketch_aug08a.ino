//SCL -> A5
//SDA -> A4

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver servoController = Adafruit_PWMServoDriver(0x40); 

const uint8_t frequency = 50;    // Frecuencia PWM de 50Hz o T=20ms
const uint8_t ServoMinTicks = 102; // ancho de pulso en ticks para pocicion 0°
const uint8_t ServoMaxTicks = 512; // ancho de pulso en ticks para la pocicion 180°

void setup()
{
  servoController.begin();
  servoController.setPWMFreq(frequency );
}

void loop()
{
  for (uint16_t duty = ServoMinTicks; duty < ServoMaxTicks; duty++)
  {
    for (uint8_t n = 0; n<16; n++)
    {
      servoController.setPWM(n, 0, duty);
    }
  }
  delay(1000);

  for (uint16_t duty = ServoMaxTicks; duty > ServoMinTicks; duty++)
  {
    for (uint8_t n = 0; n<16; n++)
    {
      servoController.setPWM(n, 0, duty);
    }
  }
  delay(1000);
}
