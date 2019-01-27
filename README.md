# Iridium 9603N Solar Beacon

A 48g solar powered Iridium 9603N + GNSS Beacon (Tracker)

Suitable for high altitude ballooning, asset tracking and many other remote monitoring applications.

![Iridium_9603N_Solar_Beacon_1](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Iridium_9603N_Solar_Beacon_1.JPG)

The beacon can be powered by two [PowerFilm Solar MPT3.6-150 solar panels](https://www.powerfilmsolar.com/custom-solutions/electronic-component-solar-panels/electronic-component-solar-panels-product-page/mpt3-6-150).
It can also be powered via USB becoming an extremely small Iridium Beacon Base which you can use to track other beacons from anywhere.

![Iridium_9603N_Solar_Beacon_5](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Iridium_9603N_Solar_Beacon_5.JPG)

The Iridium 9603N and u-blox MAX-M8Q GNSS share a single antenna. Antenna switching is performed by a Skyworks AS179-92LF RF Switch.

**See [LEARN.md](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/LEARN.md) for more details.**

**See [ASSEMBLY.md](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/ASSEMBLY.md) for details on how to assemble the PCB.**

See [Iridium_9603N_Solar_Beacon.pdf](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/Iridium_9603N_Solar_Beacon.pdf) for the schematic, layout and Bill Of Materials.

The [Eagle](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/tree/master/Eagle) directory contains the schematic and pcb design files.

The [Arduino](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/tree/master/Arduino) directory contains the Arduino code.

The [OpenSCAD](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/tree/master/OpenSCAD) directory contains the .stl and .scad files for the 3D-printed cover.

Please refer to the [Iridium_9603_Beacon Repo](https://github.com/PaulZC/Iridium_9603_Beacon/blob/master/RockBLOCK.md) for details on how to track your beacon
via the RockBLOCK Gateway with or without an Internet connection. If you are configuring the Solar Beacon as a Base, please make sure you use the version of
the Base code included in this Repo as the solar beacon pin definitions are slightly different and the code needs to ensure the 9603N and MAX-M8Q are not
powered up simultaneously when sharing the antenna.

This project is distributed under a Creative Commons Attribution + Share-alike (BY-SA) licence.
Please refer to section 5 of the licence for the "Disclaimer of Warranties and Limitation of Liability".

Enjoy!

**_Paul_**