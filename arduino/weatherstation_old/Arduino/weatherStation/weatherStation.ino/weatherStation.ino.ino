#include <math.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"
#include "TimerOne.h" // Timer Interrupt set to 2 second for read sensors 
#include <SPI.h>
#include <RH_RF69.h>
#include <RHReliableDatagram.h>
#include "Adafruit_Si7021.h"

#define WindSensorPin (0) // The pin location of the anemometer sensor 
#define Offset 0;
#define RF69_FREQ 433.0

// Where to send packets to!
#define DEST_ADDRESS 1
// change addresses for each client board, any number :)
#define MY_ADDRESS 2

#if defined (__AVR_ATmega32U4__) // Feather 32u4 w/Radio
#define RFM69_CS      8
#define RFM69_INT     7
#define RFM69_RST     4
#define LED           13
#endif

volatile uint16_t Luminosity;
volatile unsigned long Rotations; // cup rotation counter used in interrupt routine
volatile unsigned long ContactBounceTime; // Timer to avoid contact bounce in interrupt routine
float WindSpeed = 0; // speed miles per hour

int SamplesPerSecond = 10;
unsigned long Timestamp;

int VaneValue;// raw analog value from wind vane
int Direction;// translated 0 - 360 direction
int CalDirection;// converted value with offset applied
int LastValue;
float Temperature;

int WindSpeedSampleCounter = 0;

String values;
bool serialDebug = false;

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);
// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);
// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram rf69_manager(rf69, MY_ADDRESS);
Adafruit_Si7021 si7021 = Adafruit_Si7021();

void setup() {
  if (serialDebug) {
    Serial.begin(115200);
    while (!Serial);
  }
  setupWindSpeed();
  setupWindDirection();
  setupLightSensor();
  setupCommunication();
  setupTemperatureSensor();

}

void loop() {
  loopWindDirection();
  loopLightSensor();
  
  values = String(Luminosity) + String(";") + String(VaneValue) + String(";") + String(WindSpeed) + String(";") + String(Temperature);
  if (serialDebug) {
    Serial.println(values);
  }
  
  if ((millis() - Timestamp) > 2000) {  
    if (!serialDebug) {
      loopWindSpeed();
    }
    loopTemperatureSensor();
    Timestamp = millis();
  }
  sendData();
}

void setupWindSpeed() {
  Rotations = 0;
  pinMode(WindSensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(WindSensorPin), isr_rotation, FALLING);
}

void setupWindDirection() {
  LastValue = 1;
}

void setupLightSensor() {
  Luminosity = 0;
  // You can change the gain on the fly, to adapt to brighter/dimmer light situations
  tsl.setGain(TSL2591_GAIN_LOW);    // 1x gain (bright light)
  //tsl.setGain(TSL2591_GAIN_MED);      // 25x gain
  //tsl.setGain(TSL2591_GAIN_HIGH);   // 428x gain

  // Changing the integration time gives you a longer time over which to sense light
  // longer timelines are slower, but are good in very low light situtations!
  tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // shortest integration time (bright light)
  //tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
  //tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  // longest integration time (dim light)
}

void setupCommunication() {
  pinMode(LED, OUTPUT);
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);

  // manual reset
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);

  if (!rf69_manager.init()) {
    Serial.println("RFM69 radio init failed");
    while (1);
  }
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  // No encryption
  if (!rf69.setFrequency(RF69_FREQ)) {
    Serial.println("setFrequency failed");
  }

  // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
  // ishighpowermodule flag set like this:
  rf69.setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

  // The encryption key has to be the same as the one in the server
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
                  };
  rf69.setEncryptionKey(key);

  pinMode(LED, OUTPUT);
}

void setupTemperatureSensor() {
  Temperature = 0.0;
}
void loopWindSpeed() {
    // convert to mp/h using the formula V=P(2.25/T)
    // V = P(2.25/2.5) = P * 0.9
    WindSpeed = Rotations * 0.9; //mp/h
    WindSpeed = WindSpeed * 1.60934; //km/h
    Rotations = 0; // Reset count for next sample
}

void loopWindDirection() {
  VaneValue = analogRead(A4);
  Direction = map(VaneValue, 0, 1023, 0, 360);
  CalDirection = Direction + Offset;

  if (CalDirection > 360)
    CalDirection = CalDirection - 360;

  if (CalDirection < 0)
    CalDirection = CalDirection + 360;

  // Only update the display if change greater than 2 degrees.
  if (abs(CalDirection - LastValue) > 5)
  {
    //getHeading(CalDirection);
    LastValue = CalDirection;
  }
}

void loopLightSensor() {
  Luminosity = simpleRead();
}

void loopTemperatureSensor() {
    Temperature = si7021.readTemperature();
}

// Dont put this on the stack:
uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
uint8_t data[] = "  OK";

void sendData() {
  int valuesLength = values.length() + 1;
  char radiopacket[valuesLength];
  values.toCharArray(radiopacket, valuesLength);

  if (serialDebug) {
    Serial.print("Sending "); Serial.println(radiopacket);
  }

  // Send a message to the DESTINATION!
  if (rf69_manager.sendtoWait((uint8_t *)radiopacket, strlen(radiopacket), DEST_ADDRESS)) {
    // Now wait for a reply from the server
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (rf69_manager.recvfromAckTimeout(buf, &len, 500, &from)) {
      buf[len] = 0; // zero out remaining string
      Blink(LED, 40, 3); //blink LED 3 times, 40ms between blinks
    } 
  } 
}

void Blink(byte PIN, byte DELAY_MS, byte loops) {
  for (byte i = 0; i < loops; i++)  {
    digitalWrite(PIN, HIGH);
    delay(DELAY_MS);
    digitalWrite(PIN, LOW);
    delay(DELAY_MS);
  }
}

// This is the function that the interrupt calls to increment the rotation count
void isr_rotation () {
  if ((millis() - ContactBounceTime) > 15 ) { // debounce the switch contact.
    Rotations++;
    ContactBounceTime = millis();
  }
}

uint16_t simpleRead(void) {
  // Simple data read example. Just read the infrared, fullspecrtrum diode
  // or 'visible' (difference between the two) channels.
  // This can take 100-600 milliseconds! Uncomment whichever of the following you want to read
  uint16_t x = tsl.getLuminosity(TSL2591_VISIBLE);
  //uint16_t x = tsl.getLuminosity(TSL2591_FULLSPECTRUM);
  //uint16_t x = tsl.getLuminosity(TSL2591_INFRARED);

  return x;
}

