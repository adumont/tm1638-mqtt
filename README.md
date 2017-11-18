# MQTT bridge for TM1638

This is an MQTT bridge for TM1638 that runs on Raspberry Pi.

![TM1638 Led&Key](assets/tm1638.jpg?raw=true | width=400)
![Raspberry Pi](assets/raspberrypi.png?raw=true)
![MQTT](assets/mqtt.png?raw=true)

A TM1638 board usually presents a row of 8 leds, a row of 8 buttons, and a row of 8 7-segments digits display.

The purpose of this bridge is to:
- turn MQTT messages into Leds/Display actions: listen to MQTT topics and display text on the display, or power on/off the leds according to the messages received
- turn Buttons events into MQTT messages: it will publish messages on an MQTT topic when the buttons are pressed.

Example:

- You can read the button topic via Node-Red and toggle a smartplug or a smartbulb.

# How to build

## Build dependencies

To build tm1638-mqtt we first need to build 3 libraries:
- libbcm2835.so
- libtm1638.so
- paho-mqtt3c

### Build libbcm2835.so

- Download `bcm2835-1.52.tar.gz` (from http://www.airspayce.com/mikem/bcm2835/)
- Untar in `/home/pi`. Go to `/home/pi/bcm2835-1.52/`, and run `./configure`, then `make`.
- Then, go to `/home/pi/bcm2835-1.52/src`, and run:

```
gcc -DHAVE_CONFIG_H -I. -I.. -g -O2 -MT bcm2835.o -MD -MP -MF .deps/bcm2835.Tpo -o libbcm2835.so bcm2835.c -fPIC -shared
```

This will create the `libbcm2835.so` shared library. Copy it to `/usr/local/lib`.

### Build libtm1638.so

You will find the TM1638 library source code and some building instructions on [Martin’s Atelier](http://www.mjoldfield.com/atelier/2012/08/pi-tm1638.html).

```
$ cd
$ wget https://github.com/downloads/mjoldfield/pi-tm1638/pi-tm1638-1.0.tar.gz
$ tar xzvf pi-tm1638-1.0.tar.gz
$ cd pi-tm1638-1.0
$ ./configure
$ make
```

Then build the `libtm1638.so` shared library and copy it to `/usr/local/lib`:

```
$ cd src
$ gcc -std=c99 -o libtm1638.so tm1638.c -fPIC -shared -lbcm2835
$ sudo cp libtm1638.so /usr/local/lib/
```

### Build libpaho-mqtt3c.so

Clone the [Eclipse Paho MQTT C client](https://github.com/eclipse/paho.mqtt.c) repository and build it as per the instructions in its README. This will build and install the library.

## Build the TM1638-MQTT bridge

- Clone this repository

- Edit the GPIO PINs to which you have connected your TM1638 to the Raspbery Pi.


In my case this is how I have connected the TM1638 to the Pi as shown below:

| TM1638 | RPi |
|--------|-----|
|STB|GPIO17|
|CLK|GPIO27|
|DIO|GPIO22|

![TM1638 and RPi](assets/TM1638-RPi3_bb.png?raw=true | width=50%)

You can locate the PIN numbers in the code by looking searching for `tm1638_alloc`. The line  reads:

```
   t = tm1638_alloc(17, 27, 22);
```

If you connect the TM1638 to different pins, edit that line.

- If needed, edit the constants at the beggining of `tm1638mqtt.c`:

| Constant        | Default value           | Description  |
| ------------- |:-------------:| -----|
| `ADDRESS`     | `127.0.0.1:1883` | The IP:port of the MQTT broker |
| `CLIENTID`     | `TM1638ClientSub` | ClientId of this client |
| `TOPIC_LEDS`     | `8leds` | Messages received on this topic indicate the leds to light up |
| `TOPIC_TEXT`     | `7segs` | Messages received on this topic will be written on the eight 7-segments display |
| `TOPIC_BTTN`     | `buttons` | A message will be written in this topic when a new button is pressed. |

- Run `make`

# How to use

```
$ nohup tm1638mqtt &
```

`tm1638mqtt` will connect to the MQTT broker (defined in `ADDRESS`) and suscribe to the two topics defined in `TOPIC_LEDS` and `TOPIC_TEXT`. It will also publish messages to `TOPIC_BTTN`.

| Topic        | Format | Comments  |
| ------------ |:-------------:| -----|
| `TOPIC_LEDS` | 0-255 integer | The 8 bits will indicate which leds should be lighten up |
| `TOPIC_TEXT` | plain text | Text will be shown on display |
| `TOPIC_BTTN` | 0-7 digit | The digit indicates the corresponding button has been pressed |

# Possible enhancements

- The logic can easily be modified to indicate which combination of buttons is pressed for example (when pressing multiple buttons simultaneously) or when a button is released...
- Pass the constants as options/parameters to the process via the shell, instead of hardcoded (topics, PINs)
- Code can be modified if you need a different MQTT topics hierachy/structure
