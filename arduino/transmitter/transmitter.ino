#include <math.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"
#include "TimerOne.h" // Timer Interrupt set to 2 second for read sensors 
#include <SPI.h>
#include <RH_RF69.h>
#include <RHReliableDatagram.h>
#include "Adafruit_Si7021.h"

// TRANSMITTER


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
bool serialDebug = true;

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);
// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);
// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram rf69_manager(rf69, MY_ADDRESS);
Adafruit_Si7021 si7021 = Adafruit_Si7021();

void setup() {
  if (serialDebug) {
    Serial.begin(115200);
  }
  setupCommunication();
}

void loop() {  
  values = "Peter ist doof";
  sendData();
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

// Dont put this on the stack:
uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
uint8_t data[] = "  OK";

void sendData() {
  int valuesLength = values.length() + 1;
  char radiopacket[valuesLength];
  values.toCharArray(radiopacket, valuesLength);

  if (serialDebug) {
    Serial.print("Sending "); 
    Serial.println(radiopacket);
    Serial.println("Destination:");
    Serial.println(DEST_ADDRESS);
  }

  // Send a message to the DESTINATION!
  if (rf69_manager.sendtoWait((uint8_t *)radiopacket, strlen(radiopacket), DEST_ADDRESS)) {
    // Now wait for a reply from the server
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (rf69_manager.recvfromAckTimeout(buf, &len, 500, &from)) {
      if (serialDebug) {
        Serial.println("Received:");
        Serial.println((char*)buf);
      }
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
