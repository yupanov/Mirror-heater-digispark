#include <GyverNTC.h>
GyverNTC therm(1, 10000, 4300, 25, 9860); // pin(Digispark P2 = 1), R thermostor, B thermistor, base temp, R resistor

uint16_t countdownTimer = 0;

// Constants
const uint8_t setTemp = 60;   // Celcius degrees
const uint16_t cycle = 1000;  // Refresh data cycle in milliseconds
const uint16_t period = 1000; // PWM period in milliseconds
const int16_t timeLeft = 600;  // Countdown time in seconds

// Pins
const uint8_t btnPin = 0;
const uint8_t ledPin = 1;
const uint8_t relayPin = 3;

void setup()
{
  pinMode(btnPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH); // relay OFF
}

void loop()
{
  btnHandle();
  static uint32_t cicleTimer = millis();
  static uint16_t pwmDutyCycle = 0;

  if (countdownTimer > 0)
  {
    if (millis() - cicleTimer >= cycle)
    {
      cicleTimer = millis();
      pwmDutyCycle = calculatePwmDutyCycle();
      countdownTimer -= cycle / 1000;
      digitalWrite(ledPin, HIGH);
    }
    relayPWM(pwmDutyCycle, period);
  }
  else
  {
    digitalWrite(relayPin, HIGH); // relay OFF
    digitalWrite(ledPin, LOW);
  }
}

uint16_t calculatePwmDutyCycle() {
  uint16_t pwmDutyCycle;
  int8_t temp = therm.getTempAverage();
  int8_t diffTemp = setTemp - temp;
  if (diffTemp < 0) diffTemp = 0;
  pwmDutyCycle = diffTemp * 10;
  if (pwmDutyCycle > 100) pwmDutyCycle = 100;
  return pwmDutyCycle;
}

void btnHandle()
{
  static boolean btnState;
  static boolean lastBtnState = 0;
  static uint32_t debounceBtnTimer;

  boolean reading = digitalRead(btnPin);
  if (reading != lastBtnState)
  {
    debounceBtnTimer = millis();
  }
  if ((millis() - debounceBtnTimer) > 50 && reading != btnState)
  {
    btnState = reading;
    if (btnState == LOW) { // The button action itself: reset the timer or off
      if (countdownTimer == 0) countdownTimer = timeLeft;
      else countdownTimer = 0;
    }
  }
  lastBtnState = reading;
}

void relayPWM(uint16_t pwmDutyCycle, uint16_t period)
{ // 0-100
  static uint32_t pwmTimer = millis();
  uint16_t k = period / 100;

  if (millis() - pwmTimer < pwmDutyCycle * k)
  { //during  the pwmDutyCycle
    digitalWrite(relayPin, LOW); // relay ON
  }
  else if (millis() - pwmTimer < period)
  { // after the pwmDutyCycle during the rest of the cycle
    digitalWrite(relayPin, HIGH); // relay OFF
  }
  else
    pwmTimer = millis();
}
