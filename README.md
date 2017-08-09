# MQTT bridge for TM1638

This is a MQTT bridge for TM1638 to run on Raspberry Pi.

A TM1638 board usually presents a row of 8 leds, a row of 8 buttons, and a row of 8 7-segments digits display.

The purpose of this bridge is to:
- turn MQTT messages into Leds/Display actions: listen to MQTT topics and display text on the display, or power on/off the leds according to the messages received
- turn Buttons events into MQTT messages: it will publish messages on an MQTT topic when the buttons are either pressed or released.
