# ESP32 TOTP

An ESP32-based TOTP storing the private key in flash storage and getting time
synchronization information from the internet using NTP. This is based on my
other TOTP setup using Arduino, found here:
[lxsavage/arduino-totp](https://github.com/lxsavage/arduino-totp).

## Setup

> [!NOTE]
> This project was built on PlatformIO, and was tested using an Expressif ESP32
> dev board. While other compatible configurations should work, they are
> currently untested due to me not having other hardware.

The setup of this system is as follows:

1. Wire the [hardware connections](#hardware-connections) as described below to
   the ESP32
2. Upload the program to the ESP32 and follow the instructions for writing the
   secret/WiFi credentials to the board

### Hardware Connections

> [!NOTE]
> If following along from
> [lxsavage/arduino-totp](https://github.com/lxsavage/arduino-totp), these
> pinouts are different due to moving pins around to compensate for reserved
> pins during upload on the ESP32.

```plaintext
LCD 1602 (for display)

VSS -> GND
VDD -> 5V
V0  -> POT middle
RS  -> GPIO13
RW  -> GND
E   -> GPIO14
D4  -> GPIO25
D5  -> GPIO26
D6  -> GPIO18
D7  -> GPIO19
A   -> 220Ω resistor -> 5V
K   -> GND
```

```plaintext
POTENTIOMETER (for adjusting LCD contrast)

left -> GND
middle -> LCD V0
right -> 5V
```

```plaintext
BTN (for reset)

side 1 -> GPIO21
side 2 -> 10kΩ resistor -> 3.3V
```

### Loading Credentials

In order to load credentials, do the following:

1. Hold the BTN down while pressing the board ENABLE/restart button
2. As soon as the display reads "LOAD MODE", let go of the BTN
3. Send the requested information on the display via a serial connection set at
   115200 baud
4. Once everything has been set, the board will restart on its own

> [!NOTE]
> You can skip writing a new secret by waiting 2 seconds, then pressing BTN
> again. You can skip setting new WiFi credentials by pressing ENABLE/restart
> on the board after putting in a new secret. Note that you have to set the
> SSID/PPK together for the network and cannot change these individually.
