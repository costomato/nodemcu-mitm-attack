# NodeMCU ESP8266 Vulnerability Showcase

This project showcases a vulnerability in NodeMCU ESP8266 that allows man-in-the-middle (MITM) attacks to be easily possible on the IoT device and steal the data that is being sent from NodeMCU to the server. The project demonstrates how an attacker can intercept the data being sent from NodeMCU to a server using ARP spoofing and how the data can be stolen as it is sent over the network in plaintext.

## Introduction

The NodeMCU ESP8266 is a popular Internet of Things (IoT) device that uses built-in Wi-Fi to connect to the internet. It can be programmed using Lua or Arduino IDE, and is widely used in many IoT projects. However, it is vulnerable to Man-in-the-Middle (MITM) attacks, where an attacker can intercept and steal the data being sent from NodeMCU to the server. This project demonstrates how to perform a MITM attack on NodeMCU and how to prevent it using Elliptic Curve Cryptography.

## Project Overview

This project involves two phases. In the first phase, we connect a BMP280 sensor to NodeMCU and collect temperature, barometric pressure, and altitude data from the sensor. NodeMCU then sends this data to ThingSpeak, a cloud-based IoT platform, using its built-in Wi-Fi module. In the second phase, we demonstrate how to perform a MITM attack on NodeMCU using a MacBook as the attacker machine. Finally, we implement Elliptic Curve Cryptography to secure the data being sent from NodeMCU to the server.

## Tools Required

- NodeMCU ESP8266
- BMP280 Sensor
- MacBook or any other computer with Wireshark and Mitmproxy installed
- Arduino IDE or Lua to program NodeMCU
- ThingSpeak account

### Phase 1: Collecting Data from Sensor and Sending to ThingSpeak

1. Connect the BMP280 sensor to NodeMCU. The BMP280 has SDA and SCL pins that should be connected to D2 and D1 pins on NodeMCU, respectively. The VCC and GND pins should be connected to the 3V3 and G pins on NodeMCU, respectively.

2. Open the Arduino IDE or Lua editor and write a code to collect data from the sensor and send it to ThingSpeak using the Wi-Fi module of NodeMCU. The code should also display the collected data on the Serial Monitor. The code can be found in the file `bmp_to_thingspeak.ino` in this repository. Make sure that you have installed the required libraries like Adafruit_BMP280, ESP8266WiFi, and ThingSpeak.

3. Upload the code to NodeMCU using the Arduino IDE or Lua editor.

4. Create a ThingSpeak account and create a channel to receive the data from NodeMCU. Note down the Channel ID and API Key.

5. Power on NodeMCU and verify that it is sending data to ThingSpeak by checking the Serial Monitor and the ThingSpeak channel.

### Phase 2: Performing MITM Attack on NodeMCU

1. Install Wireshark on the MacBook or any other computer that will be used as the attacker machine.

2. Connect the MacBook and NodeMCU to the same Wi-Fi network.

`Next, we will perform a Man-in-the-Middle (MITM) attack to intercept the traffic between NodeMCU and the server. For this purpose, we will use mitmproxy, a popular open-source tool for intercepting, modifying, and replaying HTTP/HTTPS traffic.`

3. Download mitmproxy:

mitmproxy can be installed on macOS using the Homebrew package manager. Open the terminal and type the following command to install mitmproxy:

```zsh
brew install mitmproxy
```

4. List devices on the network:

Type the following command to list all the devices connected to the same network:
```zsh
arp -a
```

5. Enable IP forwarding:

To enable IP forwarding, type the following command:

```zsh
sudo sysctl -w net.inet.ip.forwarding=1
```

6. Perform ARP spoofing:

Use the following command to perform ARP spoofing:

```zsh
arpspoof -i en0 -r -t <default gateway> <victim device IP address>
```

Here, -i specifies the interface (in this case, it is Wi-Fi), -r makes the attack bidirectional, -t specifies the target (NodeMCU), and <default gateway> is the IP address of the router, and <victim device IP address> is the IP address of NodeMCU. Note that the order of <default gateway> and <victim device IP address> can be reversed.

7. Enable port forwarding:

To capture the traffic between NodeMCU and the server, we need to forward the port through which the data is being sent to mitmproxy. Use the following command to do so:

```zsh
echo "rdr pass inet proto tcp from any to any port <port on which data is being sent> -> 127.0.0.1 port <port on which mitmproxy is running>" | sudo pfctl -ef -
```

In this case, the requests are sent via HTTP, so we need to forward port 80 to the desired port (8080 in this case, as we will be running mitmproxy on port 8080).

For Linux, you can use iptables to forward the port.

8. Run mitmproxy:

Use the following command to start mitmproxy:

```zsh
mitmproxy --mode transparent --showhost -p <port on which mitmproxy is running> -k
```

Here, --mode transparent specifies the mode of attack (transparent means the victim won't be aware of the attack), --showhost shows the host name of the server, -p specifies the port on which mitmproxy is running (8080 in this case), and -k disables TLS verification (since we are intercepting HTTPS traffic).

Wait for data from NodeMCU:

Now, you can see the data that is being sent from NodeMCU to the server. Note that the data is not encrypted, so sensitive data can be easily stolen.

9. Revert port forwarding rule (optional):

Once you have finished intercepting the traffic, use the following command to revert the port forwarding rule:

```zsh
sudo pfctl -f /etc/pf.conf
```

For Linux, use the appropriate iptables command to revert the port forwarding rule.

Finally, to ensure the security of the data being sent from NodeMCU, it is recommended using elliptic curve cryptography instead of RSA, as RSA requires a lot of processing power and is vulnerable to timing attacks.

### Phase 3: Adding Security to the Project, i.e Implementing Elliptic Curve Cryptography

Problem: The data being sent from nodemcu was not encrypted. Sensitive data can be easily stolen.
Solution: RSA cannot be used as it requires more computational power. Hence we use elliptic curve cryptography as it is suitable for low-powered devices.

We can add security to the project by following these steps:

> **Note:** The complete implementation of ECC-based security—featuring Elliptic Curve Diffie-Hellman (ECDH) key exchange and AES encryption—has been moved to separate repositories for better modularity and clarity.

* 🔐 **ECC Client (ESP32-based sensor node):**
  [iot-security-ecc-client](https://github.com/costomato/iot-security-ecc-client)

* 🛡️ **ECC Server (decryption + ThingSpeak integration):**
  [iot-security-ecc-server](https://github.com/costomato/iot-security-ecc-server)

These repositories demonstrate how to protect IoT communication using secure cryptographic protocols, ensuring confidentiality and integrity even in hostile network environments.

---

## Conclusion

In this project, we showcased a vulnerability in the nodemcu esp8266 that allows a man-in-the-middle attack to be easily possible on the IoT device and steal the data that is being sent from nodemcu to the server. We demonstrated how to perform a man-in-the-middle attack on the nodemcu and steal the data being sent from nodemcu to the server. We also provided a solution to this vulnerability by adding security to the project using elliptic curve cryptography.