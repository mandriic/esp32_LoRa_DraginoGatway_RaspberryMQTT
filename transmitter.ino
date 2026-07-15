#define RH_ESP32 //FIRST ESP32!

#include <SPI.h>
#include <RH_RF95.h>

// Ra-02 to ESP32 (depends your scheme)
#define RFM95_CS    5
#define RFM95_RST   14
#define RFM95_INT   2

RH_RF95 rf95(RFM95_CS, RFM95_INT);
float frequency = 433.0;
int packetCounter = 0;

void setup() {
  Serial.begin(115200);
  delay(100);
  
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  if (!rf95.init()) {
    Serial.println("LoRa init failed!");
    while (1);
  }
  
  if (!rf95.setFrequency(frequency)) {
    Serial.println("Set frequency failed!");
    while (1);
  }
  
  rf95.setTxPower(13);
  Serial.println("ESP32 LoRa Transmitter Ready!");
}

void loop() {
  Serial.println("Sending to Dragino...");
  
  String message = "Hello World #" + String(packetCounter++);
  uint8_t data[message.length() + 1];
  message.getBytes(data, sizeof(data));
  
  rf95.send(data, sizeof(data));
  rf95.waitPacketSent();
  
  // Очікуємо відповідь (ACK) від Dragino
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
  
  if (rf95.waitAvailableTimeout(3000)) { 
    if (rf95.recv(buf, &len)) {
      Serial.print("Got reply: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);
    }
  } else {
    Serial.println("No reply, is Dragino running?");
  }
  
  delay(5000);
}
