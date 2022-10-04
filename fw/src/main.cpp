#include <Arduino.h>
#if defined USE_SOFTWARE_SERIAL
#include <SoftwareSerial.h>
#endif

#include <EEPROM.h>

#define FAN_OUT PB1
#define ONE_WIRE_BUS PB3
#define RX -1  // *** D3, Pin 2
#define TX PB4 // *** D4, Pin 3
#define BTN_IN PB2

#define USE_DS18B20_AVR

#define DEG                                                                    \
  "\xa7"                                                                       \
  "C"

#if defined USE_DS18B20_AVR
#include <bitop.h>
#include <ds18b20/ds18b20.h>
#endif

#if defined USE_SOFTWARE_SERIAL
SoftwareSerial _Serial(RX, TX);
#endif // USE_SOFTWARE_SERIAL

int16_t nTempTarget = 240;

bool bOn = false;
void setup() {
  // put your setup code here, to run once:
  delay(100);

#if defined USE_SOFTWARE_SERIAL
  _Serial.begin(57600);
  _Serial.println("Initializing...");
#endif // USE_SOFTWARE_SERIAL

  pinMode(FAN_OUT, OUTPUT);
  digitalWrite(FAN_OUT, !bOn);

  pinMode(BTN_IN, INPUT_PULLUP);

#if defined USE_DS18B20_AVR
  // set precision
  ds18b20wsp(&PORTB, &DDRB, &PINB, BIT(ONE_WIRE_BUS), NULL, 0, 0,
             DS18B20_RES12);
  ds18b20convert(&PORTB, &DDRB, &PINB, BIT(ONE_WIRE_BUS), NULL);
#endif

  EEPROM.begin();
  nTempTarget = EEPROM.get(0, nTempTarget);
  _Serial.print("nTT=");
  _Serial.println(nTempTarget);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(100);

  // static int8_t temp = 0;
  static int16_t temp16 = 0;
  bool bLog = false;
#if defined USE_DS18B20_AVR
  int16_t ntemp;
  static int16_t ntempprev = -1;
  if (DS18B20_ERROR_OK !=
      ds18b20read(&PORTB, &DDRB, &PINB, BIT(ONE_WIRE_BUS), NULL, &ntemp))
    ntemp = 1600;
  ds18b20convert(&PORTB, &DDRB, &PINB, BIT(ONE_WIRE_BUS), NULL);

  if (ntempprev != ntemp) {
    ntempprev = ntemp;
    float temperatur = ntemp / 16.0;
    // temp = temperatur;
    // char ctemp = (temperatur - temp) * 100;
    // _Serial.printf("%+0.2hd.%02d" DEG "\n", temp, ctemp);
    temp16 = ntemp / 1.6;
    // _Serial.print("temp16=");
    // _Serial.println(temp16);
    bLog = true;
  }
#endif

  if (bOn == false && temp16 >= (nTempTarget + 10)) {
    bOn = true;
    digitalWrite(FAN_OUT, !bOn);
#if defined USE_SOFTWARE_SERIAL
    // _Serial.println("FAN ON");
    bLog = true;
#endif // USE_SOFTWARE_SERIAL
  } else if (bOn == true && temp16 <= (nTempTarget - 10)) {
    bOn = false;
    digitalWrite(FAN_OUT, !bOn);
#if defined USE_SOFTWARE_SERIAL
    // _Serial.println("FAN OFF");
    bLog = true;
#endif // USE_SOFTWARE_SERIAL
  }

  static bool bBtn = false;

  if (bBtn == false && !digitalRead(BTN_IN)) {
    bBtn = true;
    // _Serial.println("BTN ON");
    bLog = true;
  } else if (bBtn == true && digitalRead(BTN_IN)) {
    bBtn = false;
    // bLog = true;_Serial.println("BTN OFF");
    nTempTarget = temp16;
    EEPROM.put(0, nTempTarget);
    _Serial.printf("nTT:=");
    _Serial.println(nTempTarget);
  }

  if (bLog) {
    _Serial.print("Tmp=");
    _Serial.print(temp16);
    _Serial.print(", [");
    _Serial.print(nTempTarget - 10);
    _Serial.print(":");
    _Serial.print(nTempTarget + 10);
    _Serial.printf("][B=%c][F=%c]\n", (bBtn ? 'X' : '0'), (bOn ? 'X' : '0'));
  }
}