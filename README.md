# esp32_LoRa_DraginoGatway_RaspberryMQTT
This project describes the creation of an autonomous, lightweight and stable LoRa gateway based on the **Dragino LG01-P** router (433 MHz), which receives data from sensors on **ESP32** and broadcasts it to a local MQTT broker on **Raspberry Pi** (or any other server).

During the setup process, we completely abandoned the factory "heavy" and unstable Bridge protocol from Arduino Yun, switched the hardware to pure data exchange via UART and solved problems with routing in the local network.


To work with the LoRa module Ra-02 (SX1278), the RadioHead library is used.

Solving the problem of compiling for ESP32:
When compiling RadioHead for the ESP32 architecture, the fatal error: util/atomic.h: No such file or directory often occurs (since this library is purely AVR).

Solution: Before connecting the libraries in the sketch, you need to force the architecture macro #define RH_ESP32.

Step 2. **Configuring Dragino LG01-P (Receiver)**
The Dragino gateway consists of two parts. We need to configure both.

1. Firmware of the ATMega328p microcontroller
By default, Dragino has a sketch loaded that uses the Bridge.h library. It constantly spams the run-bridge command and binary data to the port, which prevents direct reading of the port. We replace it with a pure LoRa receiver.

Connect Dragino to the PC via the built-in USB port, select the Arduino Uno board in the Arduino IDE and load this sketch:
-dragino.ino

Configuring the Linux part (OpenWrt)
Log in to Dragino via SSH (ssh root@10.130.1.1).
Step A. Disable the system console on the serial port
By default, OpenWrt uses the /dev/ttyATH0 port as a terminal for entering Linux commands. The system perceives any data from the ATMega as commands and tries to execute them.
Open the /etc/inittab file

<code>#ttyATH0::askconsole:/bin/ash --login</code>

Save the changes (Esc, :wq, Enter) and reboot Dragino. Now the port is completely free for our data.

Step 3. Routing and merging into one network
In our scenario:
Raspberry Pi is in the home Wi-Fi/LAN network (192.168.1.XXX).
Dragino is connected to the PC via cable in its own subnet 10.130.1.X (Dragino IP: 10.130.1.1).
In order for Dragino to be able to reach Raspberry Pi without complex routing, we configure an additional IP address (alias) from the Dragino subnet for Raspberry Pi.
Temporary alias on Raspberry Pi (for testing):
Determine the name of the network interface (for example, eth0 or wlan0 via ip link show) and execute on Raspberry:


<code>sudo ip addr add 10.130.1.XXX/24 dev eth0 label eth0:0</code>


(To permanently save the alias on the Raspberry Pi, use the NetworkManager configurator (nmcli) or /etc/dhcpcd.conf depending on your OS version).


Step 4. **Setting up an MQTT broker on Raspberry Pi**
By default, the modern Mosquitto broker (v2.0+) blocks external connections without authorization. We need to allow Dragino to connect anonymously.

<code>sudo nano /etc/mosquitto/mosquitto.conf</code>

And add this:

<code>listener 1883 0.0.0.0
allow_anonymous true</code>

Save and restart service:

<code>sudo systemctl restart mosquitto.service</code>

Step 5. Script-repeater on Dragino (UART -> MQTT)
Since the stripped-down Dragino firmware does not have the stty utility and the pyserial library for Python, we use the built-in utility stack: microcom (for reading UART at 115200 baud) and mosquitto_pub (for sending data).

<code>vi /root/bridge.sh</code>

copy file bridge.sh or create your own

Script autorun settings:
To have the bridge automatically start when the router starts, add its launch to the <code>/etc/rc.local</code> file before the exit 0 line:
<code>/root/bridge.sh > /dev/null 2>&1 &</code>

Step 6. Testing and verifying the result
Make sure the bridge is running on Dragino.

On Raspberry Pi, subscribe to the topic:
<code>mosquitto_sub -h localhost -t "lora/data" -v</code>

Turn on the ESP32. A clean stream of data will start to appear in the Raspberry Pi terminal:
<code>
lora/data RX: Hello World #771 | RSSI: -30
lora/data RX: Hello World #772 | RSSI: -30
</code>
