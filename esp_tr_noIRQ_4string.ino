#include <SPI.h>
#include "printf.h"
#include <RF24.h>

#define SCK_PIN 12
#define MISO_PIN 13
#define MOSI_PIN 11
#define CSN_PIN 10
#define CE_PIN 9

// --- define data symbel ---
struct NRFsymbel {
  uint8_t group_id;
  uint8_t part_id;
  uint8_t total;
  uint8_t type;
  uint8_t other;
  uint8_t data[27];
};

// --- define sensory buffer ---
struct FrameBuffer {
  bool parts_received[8];       
  uint8_t total_parts;
  uint8_t type;
  uint8_t data[27 * 8];         // at most 8 data history 
};

//----- group_id as index -----
FrameBuffer group_buffers[256];  

// --- nrf radio setting ---
SPIClass mySPI(FSPI);
RF24 radio(CE_PIN, CSN_PIN);
const uint8_t addr[6] = "ABCDE";// tx&rx address
const uint8_t payloadSize = 32;// maxium transmit bytes 
bool role = false; // RX by default

void setup() {
  //--- open serial print ---
  Serial.begin(115200);
  delay(1000);
  Serial.println("Setup start...");

  //--- nrf set up---
  mySPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CSN_PIN);
  if (!radio.begin(&mySPI)) {
    Serial.println("NRF24 init failed");
    while (1);
  }
  if (radio.isChipConnected()) {
    Serial.println("NRF24 connected to SPI");
  } else {
    Serial.println("NRF24 is NOT connected to SPI");
    while (1);
  }

  //--- nrf radio setting ---
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_1MBPS); // rate setting
  radio.setChannel(110); // channel setting
  radio.setCRCLength(RF24_CRC_DISABLED);
  radio.setAutoAck(false); // close ACK
  radio.setPayloadSize(payloadSize);
  radio.openWritingPipe(addr);
  radio.openReadingPipe(0, addr);

  if (role) { // tx & rx mode switch 
    radio.stopListening();
    Serial.println("Set as TX");
  } else {
    radio.startListening();
    Serial.println("Set as RX");
  }
  printf_begin();
  radio.printDetails();
  Serial.println("Setup complete.");
}

void loop() {
  static uint32_t counter = 0;

  if (role) {
    // TX Mode 
    char message[] = "Hello from ESP32!"; // string test check 
    uint8_t buf[payloadSize];
    
    // clean buffer
    memset(buf, 0, payloadSize);
    // copy message to buffer 
    memcpy(buf, message, strlen(message));

    bool report = radio.write(&buf, sizeof(buf));
    if (report) {
      Serial.print("TX OK: ");
      Serial.println(message);
    } else {
      Serial.println("TX failed");
    }

    delay(1000);
  } else {
    // RX polling mode
    if (radio.available()) {

    //--- data read ---
    uint8_t rawBuf[32];
    radio.read(rawBuf, 32);

    //--- data copy ---
    NRFsymbel packet;
    memcpy(&packet, rawBuf, sizeof(packet));
    FrameBuffer &fb = group_buffers[packet.group_id];
    fb.total_parts = packet.total;
    fb.type = packet.type;
    memcpy(&fb.data[packet.part_id * 27], packet.data, 27);
    fb.parts_received[packet.part_id] = true;

    // --- check whether copy complete ---
    bool complete = true;
    for (int i = 0; i < fb.total_parts; i++) {
      if (!fb.parts_received[i]) {
        complete = false;
        break;
      }
    }

    if (complete) {
    uint8_t *payload = fb.data;

    //--- print distance data ---
    uint8_t f = payload[1];
    uint8_t l = payload[2];
    uint8_t r = payload[3];
    Serial.printf("\n Frame %d:\n", packet.group_id);
    Serial.printf("Distance - Front: %dcm, Left: %dcm, Right: %dcm\n", f, l, r);

    //--- print barcode data ---
    uint8_t barcode = payload[0];
    if (barcode != 0xFF) {
      Serial.print("Barcode - ");
    for (int i = 7; i >= 0; i--) {
      Serial.print((barcode >> i) & 0x01);
      }
      Serial.println();
      } else {
      Serial.println("No barcode");
      }

      // buffer clean 
      memset(&fb, 0, sizeof(FrameBuffer));
      }
    }
  }
}
