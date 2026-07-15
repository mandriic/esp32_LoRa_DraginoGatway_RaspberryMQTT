#!/bin/sh

# Raspberry Pi 
RASP_IP="10.130.1.XXX"
TOPIC="lora/data"

echo "Starting bridge: /dev/ttyATH0 -> MQTT ($RASP_IP)"

# Читаємо порт за допомогою microcom та перенаправляємо рядки в MQTT
microcom -s 115200 /dev/ttyATH0 | while read -r line; do
    if [ -n "$line" ]; then
        mosquitto_pub -h "$RASP_IP" -t "$TOPIC" -m "$line"
    fi
done
