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
#endif


// Change to 434.0 or other frequency, must match RX's freq!
#define RF69_FREQ 433.0

// who am i? (server address)
#define MY_ADDRESS     1


#if defined (__AVR_ATmega32U4__) // Feather 32u4 w/Radio
  #define RFM69_CS      8
  #define RFM69_INT     7
  #define RFM69_RST     4
  #define LED           13
#endif
// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram rf69_manager(rf69, MY_ADDRESS);

// SerialCommand Handler. Hört in loop auf Befehle die über serielle Schnittstelle empfangen werden
SerialCommand sCmd;

void setup()
{
  Serial.begin(115200);
  //while (!Serial) { delay(1); } // wait until serial console is open, remove if not tethered to computer

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
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  rf69.setEncryptionKey(key);
  
  pinMode(LED, OUTPUT);
  sCmd.addCommand("FLUSH", flushHandler);
  //sCmd.addCommand("RESET", resetHandler);
}



void flushHandler() {
  Serial.flush();
}

// Dont put this on the stack:
uint8_t data[] = "And hello back to you";
// Dont put this on the stack:
uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];

void loop() {
  if (rf69_manager.available())
  {
    // Wait for a message addressed to us from the client
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (rf69_manager.recvfromAck(buf, &len, &from)) {
      buf[len] = 0; // zero out remaining string

      Serial.println((char*)buf);
      Blink(LED, 40, 3); //blink LED 3 times, 40ms between blinks

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
}



/*
 * Visuelles Feedback über Board-LED
 */
void Blink(byte pin, byte delay_ms, byte loops) {
  for (byte i = 0; i < loops; i++)  {
    digitalWrite(pin, HIGH);
    delay(delay_ms);
    digitalWrite(pin, LOW);
    delay(delay_ms);
  }
}
