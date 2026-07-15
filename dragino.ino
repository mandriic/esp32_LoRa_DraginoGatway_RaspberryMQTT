#include <SPI.h>
#include <RH_RF95.h>

// Dragino LG01-P
#define LORA_SS    10
#define LORA_RST   9
#define LORA_INT   2 // DIO0 connect to D2

RH_RF95 rf95(LORA_SS, LORA_INT);
int led = A2; // LED Dragino
float frequency = 433.0; // Your's default freq 

void setup() {
  pinMode(led, OUTPUT);     
  Serial.begin(115200); // connect with Linux (Atheros)
  while (!Serial); 
  
  Serial.println("--- DRAGINO LORA RECEIVER START ---");
  
  pinMode(LORA_RST, OUTPUT);
  digitalWrite(LORA_RST, HIGH);
  digitalWrite(LORA_RST, LOW);
  delay(10);
  digitalWrite(LORA_RST, HIGH);
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
  Serial.print("Listening on: "); Serial.print(frequency); Serial.println(" MHz");
}

void loop() {
  if (rf95.available()) {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    
    if (rf95.recv(buf, &len)) {
      digitalWrite(led, HIGH);
      
      // Print in Serial clean text, which read Linux-script
      Serial.print("RX: ");
      Serial.print((char*)buf);
      Serial.print(" | RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);
      
      // Sending (ACK) to ESP32
      uint8_t data[] = "ACK from Dragino";
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      
      digitalWrite(led, LOW);
    }
  }
}
