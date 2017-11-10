# MQTT bridge for TM1638

This is an MQTT bridge for TM1638 that runs on Raspberry Pi.

![TM1638 Led&Key](asset/tm1638.jpg?raw=true | width=400)
![Raspberry Pi](asset/raspberry.png?raw=true)
![MQTT](asset/mqtt.png?raw=true)

A TM1638 board usually presents a row of 8 leds, a row of 8 buttons, and a row of 8 7-segments digits display.

The purpose of this bridge is to:
- turn MQTT messages into Leds/Display actions: listen to MQTT topics and display text on the display, or power on/off the leds according to the messages received
- turn Buttons events into MQTT messages: it will publish messages on an MQTT topic when the buttons are either pressed or released.
