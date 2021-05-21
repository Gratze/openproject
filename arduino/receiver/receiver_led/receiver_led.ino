// A basic everyday NeoPixel strip test program.

// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting NeoPixels on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.
// - When using a 3.3V microcontroller with a 5V-powered NeoPixel strip,
//   a LOGIC-LEVEL CONVERTER on the data line is STRONGLY RECOMMENDED.
// (Skipping these may work OK on your workbench but can fail in the field)

#include <Arduino.h>
#include <NeoPatterns.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
#define LED_PIN          13
#define LED_COUNT        15

// onComplete callback functions
void OnePattern(NeoPatterns *aLedsPtr);

// The NeoPatterns instances
//NeoPatterns bar16 = NeoPatterns(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800, &OnePattern);

//NeoPixels
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);


#include <SPI.h>
#include <RH_RF69.h>
#include <RHReliableDatagram.h>
#include <RHReliableDatagram.h>
#include <SerialCommand.h>

// RECEIVER
// Pinbelegung des Boards definieren (ATmega32U4)

#if defined (__AVR_ATmega32U4__)
#define RFM69_CS      8     // Chip Select (Start=low / Stop=high SPI-Transaktion - Start/Stop des Funkmoduls)
#define RFM69_INT     7     // IRQ (interrupt request) - Unterbrechungsanforderung an Prozessor (Warten des Programmes auf Senden und Empfangen des Funkmoduls)
#define RFM69_RST     4     // Reset des Funkmoduls
#define LED           13    // LED Leuchte
#define HEART_VALVE   12    // Herzklappe
#endif


// Change to 434.0 or other frequency, must match RX's freq!
#define RF69_FREQ 433.0

// who am i? (server address)
#define MY_ADDRESS     1

#define TRANSMITTER_ADDRESS 2


#if defined (__AVR_ATmega32U4__) // Feather 32u4 w/Radio
  #define RFM69_CS      8
  #define RFM69_INT     7
  #define RFM69_RST     4
  #define LED           13
  #define HEART_VALVE   12
#endif
// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram rf69_manager(rf69, MY_ADDRESS);

// SerialCommand Handler. Hört in loop auf Befehle die über serielle Schnittstelle empfangen werden
SerialCommand sCmd;

bool led_on = false;
bool heart_is_open = false;

void setup()
{

  Serial.begin(115200);
  //while (!Serial) { delay(1); } // wait until serial console is open, remove if not tethered to computer

  pinMode(LED, OUTPUT);     
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);

  // heart valve
  pinMode(HEART_VALVE, INPUT);
  heart_is_open = digitalRead(HEART_VALVE);

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
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  rf69.setEncryptionKey(key);
  
  // pinMode(LED, OUTPUT);
  sCmd.addCommand("FLUSH", flushHandler);
  //sCmd.addCommand("RESET", resetHandler);

   // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
    #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
    #endif
    // END of Trinket-specific code.

  //bar16.begin(); // This sets the pin. 
  strip.begin();
  strip.show();
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  delay(1000);
  digitalWrite(10, LOW);
}



void flushHandler() {
  Serial.flush();
}

// Dont put this on the stack:
uint8_t data[] = "And hello back to you";
// Dont put this on the stack:
uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];

unsigned long lastUpdate = 0;
int vibrationCounter = 0;

void loop() {
  
  if (rf69_manager.available())
  {
    Serial.println("received");
    // Wait for a message addressed to us from the client
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (rf69_manager.recvfromAck(buf, &len, &from)) {
        char* command = (char*) buf;
        char cmd = command[0];
      
          switch (cmd) {
            case '0': 
              Serial.println("Lampe AUS");
              led_on = false;
              //bar16.Heartbeat(COLOR32(0, 0, 0), 0, 0);
              
              break;
            case '1': 
              Serial.println("Lampe AN");
              led_on = true;
              //bar16.Heartbeat(COLOR32(255, 0, 0), 29, 0);
              break;
            default:
              Serial.println("Command not supported.");
              break;
          }
        
      buf[len] = 0; // zero out remaining string

      Serial.println(cmd);
      // Blink(LED, 40, 3); //blink LED 3 times, 40ms between blinks

      // Send a reply back to the originator client
      if (!rf69_manager.sendtoWait(data, sizeof(data), from))
        Serial.println("Sending failed (no ack)");
    }
  }
  if (Serial) {
    if (Serial.available() > 0) {
      sCmd.readSerial();
    }
  }

   if (!led_on) {
      //bar16.update();
      digitalWrite(10, LOW);
      strip.clear();
      strip.show();
    }

   if(led_on){
      if(vibrationCounter == 0){
        lastUpdate = millis(); 
        analogWrite(10, 200);
        for(int i = 0; i < 60; i++){
          for(int j = 7; j < 14; j++){
            //Blau
            strip.setPixelColor(j, 0, 0, i);
          } 
        }
        strip.show();
        vibrationCounter++;
      }
      
      if((millis() - lastUpdate) > 100 && vibrationCounter == 1){
        digitalWrite(10, LOW);
        for(int i = 60; i >= 0; i--){
          for(int j = 7; j < 14; j++){
            strip.setPixelColor(j, 0, 0, i);
          } 
        }
        strip.show();
        vibrationCounter++;
      }
      
      if((millis() - lastUpdate) > 400 && vibrationCounter == 2){
        analogWrite(10, 250);
        for(int i = 0; i < 150; i++){
          for(int j = 0; j < 7; j++){
            //Rot
            strip.setPixelColor(j, i, 0, 0);
          } 
        }
        strip.show();
        vibrationCounter++;
      }
      
      if((millis() - lastUpdate) > 550 && vibrationCounter == 3){
        digitalWrite(10, LOW);
        for(int i = 150; i >= 0; i--){
          for(int j = 0; j < 7; j++){
            strip.setPixelColor(j, i, 0, 0);
          } 
        }
        strip.show();
        vibrationCounter++;
      }

      if((millis() - lastUpdate) > 1000 && vibrationCounter == 4){
        vibrationCounter = 0;
      }
   }

   // Check if heart was opened or closed
   bool new_heart_is_open = digitalRead(HEART_VALVE);

   if (heart_is_open != new_heart_is_open) {
     // state has changed
     Serial.println(heart_is_open);
     Serial.println(new_heart_is_open);
     heart_is_open = new_heart_is_open;
     if (new_heart_is_open) {
        Serial.println("Heart was opened");
     } else {
        Serial.println("Heart was closed");
     }
     if (rf69_manager.available()) {
        uint8_t data_opened[] = "HEART OPENED";
        uint8_t data_closed[] = "HEART CLOSED";
        if (new_heart_is_open) {
          if (!rf69_manager.sendtoWait(data_opened, sizeof(data_opened), TRANSMITTER_ADDRESS)) {
            Serial.println("Sending failed: Received no ack.");
          }
        } else {
          if (!rf69_manager.sendtoWait(data_closed, sizeof(data_closed), TRANSMITTER_ADDRESS)) {
            Serial.println("Sending failed: Received no ack.");
          }
        }
      } else {
        Serial.println("Sending failed: rf69 manager not available.");
      }
   } 
}

/*
 * Visuelles Feedback über Board-LED
 */
void Blink(byte pin, byte delay_ms, byte loops) {
  for (byte i = 0; i < loops; i++)  {
    // digitalWrite(pin, HIGH);
    // delay(delay_ms);
    // digitalWrite(pin, LOW);
    // delay(delay_ms);
  }
}

void OnePattern(NeoPatterns *aLedsPtr){
  
}
