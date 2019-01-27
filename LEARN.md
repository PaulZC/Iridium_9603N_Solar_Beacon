# Iridium 9603N Solar Beacon

A 48g solar powered Iridium 9603N + GNSS Beacon (Tracker)

Suitable for high altitude ballooning, asset tracking and many other remote monitoring applications.

![Iridium_9603N_Solar_Beacon_1](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Iridium_9603N_Solar_Beacon_1.JPG)

![Iridium_9603N_Solar_Beacon_2](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Iridium_9603N_Solar_Beacon_2.JPG)

![Iridium_9603N_Solar_Beacon_3](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Iridium_9603N_Solar_Beacon_3.JPG)

## Background

The Iridium 9603N Solar Beacon is a miniature version of the [Iridium_9603_Beacon](https://github.com/PaulZC/Iridium_9603_Beacon).
It can be powered by two [PowerFilm Solar MPT3.6-150 solar panels](https://www.powerfilmsolar.com/custom-solutions/electronic-component-solar-panels/electronic-component-solar-panels-product-page/mpt3-6-150)
but can also be powered via USB becoming an extremely small Iridium Beacon Base which you can use to track other beacons without an Internet connection.

![Iridium_9603N_Solar_Beacon_5](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Iridium_9603N_Solar_Beacon_5.JPG)

## The Design

See [Iridium_9603N_Solar_Beacon.pdf](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/Iridium_9603N_Solar_Beacon.pdf) for the schematic,
layout and Bill Of Materials.

The [Eagle](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/tree/master/Eagle) directory contains the schematic and pcb design files.

The key components of the Iridium 9603N Solar Beacon are:

### Iridium 9603N Module
![Assembly_15.JPG](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Assembly_15.JPG)

Available from:
- https://www.rock7.com/shop-product-detail?productId=50

Other UK and International distributors can be found at:
- https://www.iridium.com/where-to-buy/?pid=27083

Make sure you purchase the 9603N and not the older 9603. The 9603N will run from 5V ± 0.5V which is important as the super capacitor charger is set to produce 5.3V; the older 9603 is only rated to 5V ± 0.2V.

### SMA Iridium + GNSS Antenna
![Learn_1.JPG](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Learn_1.JPG)

For this version of the beacon, I recommend using the Maxtena M1600HCT-P-SMA which is tuned for the Iridium, GPS and GLONASS frequency bands.

### Atmel ATSAMD21G18 Processor
![Learn_2.JPG](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Learn_2.JPG)

As used on the Adafruit Feather M0:
- https://www.adafruit.com/products/2772

Available from e.g. Farnell / Element14 (2460544)

### Linear Technology LTC3225EDDB SuperCapacitor Charger
![Learn_3.JPG](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Learn_3.JPG)

- https://www.analog.com/en/products/ltc3225.html

Available as a bare chip from e.g. Farnell / Element14 (1715231)

### u-blox MAX-M8Q GNSS
![Learn_4.JPG](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Learn_4.JPG)

- https://www.u-blox.com/en/product/max-m8-series

### MCP111T-240 Reset Supervisor
![Learn_5.JPG](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Learn_5.JPG)

The SAMD21G18 has a built-in power-on reset and brown-out detector circuit, but it doesn't work properly if the supply voltage rises very slowly.
The SAMD21G18 datasheet isn’t much help here. Section 8.2.4.1 specifies:
- _Minimum Rise Rate_

   _The integrated power-on reset (POR) circuitry monitoring the VDDANA power supply requires a minimum rise rate._

But then the supply characteristics section (37.4) specifies a _maximum_ supply rise rate of 0.1V/µs, not a minimum.

Tests I've carried out show that the processor will reset correctly if the power supply ramps up at 0.3 V/s or more, but fails to reset correctly at 0.2 V/s or less.
As the solar panel voltage will ramp up very slowly at sunrise, I've included a separate reset supervisor.
The Microchip MCP111-240 has an open drain output which holds the processor in reset until the supply rises above 2.4V, ensuring a clean start.

### SPX3819-3.3 Voltage Regulator
![Learn_6.JPG](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Learn_6.JPG)

The SPX3819-3.3 Voltage Regulator regulates the output from the two solar panels, or the USB port, providing 3.3V for the processor and GNSS.

The LTC3225EDDB SuperCapacitor Charger draws its power directly from the solar panels or USB without going through the regulator.

MBR120 diodes protect the solar panels and the USB port from each other.

### LT1634BCMS8-1.25 1.25V Voltage Reference
![Learn_7.JPG](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Learn_7.JPG)

The SAMD21G18 processor is powered from a low drop-out 3.3V regulator. The solar / USB voltage is measured via an analog pin connected to the mid point
of two 100K resistors configured as a voltage divider. The voltage measured by the analog pin is relative to the regulator voltage. When the power voltage drops below approximately
3.7V, the regulator voltage starts to collapse. The analog voltage appears to never drop below approximately 3.4V even when the actual voltage is lower than this.

By adding a 1.25V voltage reference and connecting it to a second analog pin, its constant voltage will appear to increase as the regulator voltage starts to collapse.
This voltage increase can be used to correct the power voltage measurement.

### Skyworks AS179-92LF RF Switch

The Iridium 9603N and MAX-M8Q GNSS receiver share the same antenna. A Skyworks AS179-92LF RF switch is used to switch the antenna connection from one to the other according to which is powered up.
Most importantly, the RF switch disconnects and protects the GNSS receiver when the Iridium 9603N is transmitting.

![Learn_9.JPG](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Learn_9.JPG)

Switching is performed by applying either EXT_PWR or 3V3SW to the AS179's V1 and V2 pins.
EXT_PWR is the 5.3V power rail for the 9603N (switched via Q2 and Q3, enabled by pulling MISO/D22 high).
3V3SW is the 3.3V power rail for the MAX-M8Q (switched via Q1, enabled by pulling D11 low).
When modifying the Arduino code, take great care to make sure EXT_PWR and 3V3SW are not enabled at the same time.
**BADS THINGS WILL PROBABLY HAPPEN IF YOU DO ENABLE BOTH SIMULTANEOUSLY!**

### Two PowerFilm Solar MPT3.6-150 solar panels
![Iridium_9603N_Solar_Beacon_1.JPG](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Iridium_9603N_Solar_Beacon_1.JPG)

- https://www.powerfilmsolar.com/custom-solutions/electronic-component-solar-panels/electronic-component-solar-panels-product-page/mpt3-6-150

Available (in the UK) from e.g.:
- http://www.selectsolar.co.uk/prod/264/powerfilm-mpt36150-100ma-36v-mini-solar-panel

### Iridum 9603N Power Switching

The datasheet for the 9603N contains a note which says:

_When a transceiver has been turned off, Product Developers should not reapply power on a unit until more than 2 seconds has elapsed after power has reached 0V. Additionally, if a unit does not respond to AT commands, power off the module, wait for 2 seconds and then power it back on._

_When a 9603 is powered off the power on reset circuit requires 2 seconds for voltages to decay. If the 2 second wait time is not adhered to the reset circuit may not operate and the modem could be placed in a non-operational state. The state is not permanent and can be rectified by the above procedure._

I have seen a 9603N go into this non-operational state only twice. Unfortunately, with the previous versions of the beacon design,
the only way to completely remove power to the 9603N was to manually discharge the super capacitors. Obviously it would be disastrous if this happened
during a flight. So, a P-MOSFET (Q3) is used as a switch to disconnect the power to the 9603N when required.

![Learn_8.JPG](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Learn_8.JPG)

### IO Pins
![Learn_10.JPG](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Learn_10.JPG)

**SWCLK** and **SWDIO** are used during [programming of the SAMD bootloader](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/LEARN.md#how-do-i-install-the-atsamd21g18-bootloader)

**3V3** is the 3.3V power rail from the SPX3819-3.3 voltage regulator.

## Why do you need the Super Capacitors?
The Iridium 9603 module draws an average current of 145mA and a peak current of 1.3A when transmitting its short data bursts. That’s too much for the solar panels to provide.
The LTC3225 super capacitor charger draws a lower current from the panels to slowly charge two 2.7V capacitors, connected in series, to 5.3V. The capacitors then deliver the 1.3A to the module when it sends the data burst.

## Why is the Super Capacitor Charger charge current set to 60mA?
The datasheet for the 9603N quotes: an average idle current of 34mA; and an average receive current of 39mA.

For solar operation, we need to charge the capacitors at a higher current than 39mA, but keep the total current draw within what the solar panels can deliver.
(The LTC3225 has an efficiency of approximately 50% and hence draws approximately _twice_ the chosen charge current.)
The 10F capacitors provide the majority of the higher current draw during the transmit cycle.

For USB operation the super capacitor charge current can be set to 150mA and smaller (1F) super capacitors are adequate.

## Can I leave the USB connected during testing?
Yes. Leaving the USB connected is useful as you can monitor the Serial messages produced by the code in the Arduino IDE Serial Monitor.
If you use a standard USB cable then the beacon will draw power from USB. To test the beacon running on solar power, you will need to break the USB 5V power connection.
You can do this with a home-made power-break cable.

Take a short male to female USB extension cable; carefully strip the outer sheath from cable somewhere near the middle;
prise apart the screen connection to reveal the four USB wires (red (5V); black (GND); green and white (data)); cut and insulate the ends of the red 5V wire leaving the black, green and white wires and the screen connection intact:

![USB_Power_Break.JPG](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/USB_Power_Break.JPG)

## Arduino Code
The [Arduino](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/tree/master/Arduino) directory contains the Arduino code.

The code is based extensively on Mikal Hart’s IridiumSBD Beacon example:
- https://github.com/mikalhart/IridiumSBD/tree/master/examples/Beacon

The main loop is structured around a large switch / case statement which:
- Initialises the serial ports; checks the solar panel voltage
- Powers up the GPS; checks the solar panel voltage
- Waits until the GPS establishes a fix; checks the solar panel voltage
- Powers down the GPS and powers up the LTC3225EDDB supercapacitor charger; checks the solar panel voltage
- Waits for the supercapacitors to charge; checks the solar panel voltage
- Queues the Iridium message transmission; checks the solar panel voltage
- Powers everything down and puts the processor to sleep until the next alarm interrupt

If the solar panel voltage falls below a useful level at any time, the code jumps to the sleep case and waits for the next alarm interrupt.

The BEACON_INTERVAL is stored in non-volatile (flash) memory internal to the SAMD21 processor and can be updated via a Mobile Terminated (MT) SBD message.

## How do I install the ATSAMD21G18 bootloader?
Get yourself a Segger J-Link programmer and connect it according to [Atmel_SAMD21_Programming_Cable.pdf](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/Atmel_SAMD21_Programming_Cable.pdf).

![Atmel_SAMD21_Programming_Cable](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Atmel_SAMD21_Programming_Cable.JPG)

![Assembly_12](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Assembly_12.JPG)

Ignore the RST connection.

Connect the 5V-Supply output from the J-Link to the + pad to power the board while you configure it (it doesn’t need external power for this bit).

Follow Lady Ada’s excellent instructions:
- https://learn.adafruit.com/proper-step-debugging-atsamd21-arduino-zero-m0/restoring-bootloader

If you are using Atmel's Studio:

![Programming_1](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Programming_1.JPG)

Select Tools \ Device Programming

![Programming_2](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Programming_2.JPG)

Select J-Link in the Tool pull-down menu
Select ATSAMD21G18A in the Device pull-down menu
Select SWD in the Interface pull-down menu
Then click Apply

![Programming_2a](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Programming_2a.JPG)

Click the J-Link icon to apply target power to the beacon

![Programming_3](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Programming_3.JPG)

Select the Target Power tab and set Current State to Power On. This will apply 5V power from the J-Link
to the beacon VBUS

![Programming_4](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Programming_4.JPG)

Close the J-Link window. In the Device Programming window click 'Read' to read the Device Signature and Target Voltage

![Programming_5](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Programming_5.JPG)

Click 'Memories', click the '...' icon to select the bootloader Flash file
Click Program to erase, program and verify the device

![Programming_6](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Programming_6.JPG)

Finally, select the J-Link icon again, select the Target Power tab and set Current State to Power Off
Close Device Programming and Atmel Studio and you are ready to configure the Beacon with the Arduino code

## How do I upload the Arduino code?
The 9603 Beacon is based on the Adafruit Feather M0 (Adalogger):
- https://www.adafruit.com/products/2796
- https://www.adafruit.com/products/2772

You can follow Lady Ada’s excellent instructions:
- https://cdn-learn.adafruit.com/downloads/pdf/adafruit-feather-m0-adalogger.pdf

## What other libraries do I need?
The main one is Mikal Hart’s Iridium SBD library (V2.0) written for the Rock7 RockBLOCK:
- http://arduiniana.org/libraries/iridiumsbd/
- https://github.com/mikalhart/IridiumSBD

The code uses Cristian Maglie's FlashStorage library to store and retrieve the BEACON_INTERVAL setting:
- https://github.com/cmaglie/FlashStorage

You will also need:
- https://github.com/mikalhart/TinyGPS
- http://arduiniana.org/libraries/pstring/
- https://github.com/arduino-libraries/RTCZero

## What data will I get back from the beacon?

The Arduino code included in this repository will send the following (separated by commas):
- GPS Time and Date (year, month, day, hour, minute, second)
- GPS Latitude (degrees)
- GPS Longitude (degrees)
- GPS Altitude (m)
- GPS Speed (m/s)
- GPS Heading (degrees)
- GPS HDOP (m)
- GPS Satellites
- Solar voltage (V)
- Iteration count

E.g.:

   _20170729144631,55.866573,-2.428458,103,0.1,0,3.0,5,0,0.0,4.98,0_

You can opt to receive the data as an email attachment from the Iridium system. The email itself contains extra useful information:
- Message sequence numbers (so you can identify if any messages have been missed)
- The time and date the message session was processed by the Iridium system
- The status of the message session (was it successful or was the data corrupt)
- The size of the message in bytes
- The approximate latitude and longitude the message was sent from
- The approximate error radius of the transmitter’s location

E.g.:

   _From:	sbdservice@sbd.iridium.com_  
   _Sent:	20 August 2016 16:25_  
   _To:_  
   _Subject:	SBD Msg From Unit: 30043406174_  
   _Attachments:	30043406174-000029.sbd_  
  
   _MOMSN: 29_  
   _MTMSN: 0_  
   _Time of Session (UTC): Sat Aug 20 15:24:57 2016 Session Status: 00 - Transfer OK Message Size (bytes): 61_  
  
   _Unit Location: Lat = 55.87465 Long = -2.37135 CEPradius = 4_

You can adapt the code to send whatever data you like, up to a maximum of 340 bytes. The message is sent as plain text, but you could encrypt it if required.

You can opt to receive the data via HTTP instead of email. Your service provider will provide further details.

## Does the Solar Beacon work in exactly the same way as the Iridium 9603 Beacon?

Yes, with the exception that the solar beacon does not have an MPL3115A2 pressure and temperature sensor. The messages produced by the solar beacon have the same
format as messages from its big brother; the pressure and temperature data is simply set to zero.

You can track the solar beacon from another solar beacon acting as a Base in exactly the same way as you can with the Iridium Beacon. Please refer to the
main [Iridium_9603_Beacon Repo](https://github.com/PaulZC/Iridium_9603_Beacon/blob/master/RockBLOCK.md) for full details. Please make sure you use the version of
the Base code included in this Repo as the solar beacon pin definitions are slightly different and the code needs to ensure the 9603N and MAX-M8Q are not
powered up simultaneously when sharing the antenna.

## Acknowledgements

This project wouldn’t have been possible without the open source designs and code kindly provided by:
- Adafruit:

   The Adafruit SAMD Board library  
   The design for the Feather M0 Adalogger  
   For more details, check out the product page at:
   - https://www.adafruit.com/product/2772  

   Adafruit invests time and resources providing this open source design, please support Adafruit and open-source hardware by purchasing products from Adafruit!  
   Designed by Adafruit Industries.  
   Creative Commons Attribution, Share-Alike license

   Sercom examples:
   - https://learn.adafruit.com/using-atsamd21-sercom-to-add-more-spi-i2c-serial-ports/creating-a-new-serial

- Mikal Hart:

   The Iridium SBD library (distributed under the terms of the GNU LGPL license)  
   TinyGPS  
   PString

- Arduino:

   The Arduino IDE  
   Arduino SAMD Board library  
   RTCZero library

- Cave Moa:

   The SimpleSleepUSB example:
   - https://github.com/cavemoa/Feather-M0-Adalogger/tree/master/SimpleSleepUSB

- MartinL:

   Sercom examples:
   - https://forum.arduino.cc/index.php?topic=341054.msg2443086#msg2443086

- Cristian Maglie:

   FlashStorage library:
   - https://github.com/cmaglie/FlashStorage

## Licence

This project is distributed under a Creative Commons Share-alike 4.0 licence.
Please refer to section 5 of the licence for the "Disclaimer of Warranties and Limitation of Liability".
  

Enjoy!

**_Paul_**