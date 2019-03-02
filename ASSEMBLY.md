# Iridium 9603N Solar Beacon Assembly Instructions

Here are the assembly instructions for the Iridium 9603N Solar Beacon:

### Blank PCB

Start by having the blank PCBs manufactured. If you are based in the UK or Europe, I can recommend
[Multi-CB](https://www.multi-circuit-boards.eu/en/index.html) who can manufacture PCBs in 1-8 working days and
can process the Eagle .brd file direct - there's no need to generate Gerber files.

My recommended options are:
- Layers: 2 layers
- Format: single pieces
- Surface finish: chemical gold (ENIG)
- Material: FR4, 1.55mm
- Cu layers: 35um
- Solder stop: both sides, green
- Marking print: both sides, white

![Assembly_1](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Assembly_1.JPG)

### Add solder paste

Multi-CB can also provide you with a solder paste (SMD) stencil for the PCB. My recommended options are:
- SMD stencil for: top layer
- Make the Y dimension 20mm longer than the PCB itself to allow you to fix it down with tape
- Type: custom
- Pad reduction: yes, 10%
- Thickness: 100um
- Existing fiducials: lasered through
- Text type: half lasered
- Double-sided brushing: yes

I secure the blank PCB onto a flat work surface by locating it between two engineer's squares. I use a sheet of toughened glass
as the work surface as it is both very flat and easy to clean.

Use the two round fiducials to line up the stencil with the PCB. Secure the stencil with tape.

![Assembly_2](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Assembly_2.JPG)

Apply solder paste close to the component cut-outs and then scrape the paste over the stencil using a knife blade
or a similar straight edge. Take appropriate safety precautions when working with solder paste - particularly if you are using
tin-lead solder instead of lead-free.

![Assembly_3](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Assembly_3.JPG)

![Assembly_4](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Assembly_4.JPG)

![Assembly_5](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Assembly_5.JPG)

### Position the surface mount components

Position the components onto the blobs of solder paste using tweezers. A magnifier lamp or a USB microscope will
help you place the components in the correct position. J1 - the 20-way Samtec connector - is probably the trickiest
component to position. Take extra time to make sure the legs are centered accurately on the pads.
You may find the [SMT_Component_Placer](https://github.com/PaulZC/SMT_Component_Placer) useful.

![Assembly_7](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Assembly_7.JPG)

![Assembly_6](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Assembly_6.JPG)

### Reflow the surface mount components

Use a reflow oven to heat the circuit board to the correct temperatures to melt the solder. A reflow oven doesn't need to be
expensive. The one shown below was assembled from:

- Quest 9L 800W mini-oven
- Inkbird PID temperature controller and 40A solid state relay
- Type K thermocouple

Several people have published good reflow oven construction guides, including [this one](http://www.die4laser.com/toaster/index.html).

![Assembly_8](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Assembly_8.JPG)

Use the correct temperature profile for your solder paste, but you won't go far wrong with 160C for four minutes, followed by
210C for one minute, followed by a cool-down with the door open. Use a flashlight to check that the solder has melted across
the whole PCB at 210C. Hold the temperature at 210C a little longer if some of the solder paste still appears 'gray' instead of 'silver'.

![Assembly_9](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Assembly_9.JPG)

### Check for shorts

Carefully examine all the solder joints and correct any shorts you find.

The 'trick' to removing a short is to add more solder or solder paste and then to use
copper solder braid or wick to remove all the solder in one go.

![Shorts_1](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Shorts_1.JPG)

![Shorts_2](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Shorts_2.JPG)

![Shorts_3](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Shorts_3.JPG)

Solder any remaining surface mount components by hand and then use a cotton swab dipped in Iso-Propyl Alcohol
(IPA / Propanol / rubbing alcohol) to remove any flux residue.

### Press-in nuts

The Iridium 9603N module is held in place by two 2-56 screws. The threaded 2-56 press-in nuts now need to be pressed into the rear of the
PCB. These are McMaster part number 95117A411. The nuts are best pressed into place using an arbor press.

### Add the non-surface mount components

The non-surface mount components (super capacitors, SMA conector) can now be soldered in by hand.

![Assembly_10](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Assembly_10.JPG)

![Assembly_11](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Assembly_11.JPG)

### Install the bootloader

The SAMD21G18A processor now needs to be configured with a bootloader using a J-Link programmer or similar. See
[How do I install the ATSAMD21G18 Bootloader](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/LEARN.md#how-do-i-install-the-atsamd21g18-bootloader)
for further details.

![Assembly_12](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Assembly_12.JPG)

### Test the PCB

Before connecting the Iridium 9603N, it is a good idea to test the completed PCB. The [Arduino](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/tree/master/Arduino)
directory contains a sketch called Iridium_9603N_Solar_Beacon_Test which will test all of the components on the PCB for you. Messages are displayed
on the Arduino IDE Serial Monitor as each test is passed.

### Install the Iridium 9603N module

Take appropriate ESD precautions throughout and especially when handling the 9603N module.

Connect the 9603N to the beacon PCB using a HIROSE (HRS) u.FL-u.FL cable or similar. The cable needs to be 50 Ohm and approximately 25mm long.

![Assembly_13](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Assembly_13.JPG)

Carefully fold the 9603N module over onto the PCB, insert the 20-way connector into the Samtec socket, then secure the module using:
- two 4.5mm OD x 6mm spacers (McMaster 94669A100)
- two 2-56 x 7/16" screws (McMaster 92185A081)

![Assembly_15](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Assembly_15.JPG)

Screw the antenna onto the SMA connector

![Assembly_16](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Assembly_16.JPG)

### Retest the PCB

Now that the 9603N has been installed, retest the completed PCB using the [Arduino test code](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/tree/master/Arduino).

### Secure J1 with Epoxy

I recommend securing J1 to the PCB with a small amount of epoxy to make it more robust. 3M Scotch-Weld 2216 is an ideal choice as it remains slightly flexible when cured.
You will need to temporarily remove the 9603N while you apply the epoxy. Apply the mixed epoxy around the body and legs of J1 using a toothpick,
taking great care to keep the epoxy clear of the connector opening and pins. Make sure the epoxy is fully cured before re-installing the 9603N.

### Lacquer the PCB

I recommend giving the PCB a coat of lacquer, especially if you are intending to use it to track a balloon flight.
You will need to temporarily remove the 9603N while you apply the lacquer. Cover all of the surface mount components with
[Acrylic Protective Lacquer (conformal coating)](https://uk.rs-online.com/web/p/conformal-coatings/3217324/) except: J1, SW1, CON1, CON2, IO pads.
Keep the top of the MAX-M8Q clear of lacquer too as its package is only just thin enough to fit under the 9603N.

![Assembly_14](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Assembly_14.JPG)

Once the lacquer is dry, re-attach the 9603N and your board is ready for use.

### Assemble and attach the solar panels

The [Eagle](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/tree/master/Eagle) directory contains the schematic and pcb design file for
the Solar Panel Support.

Solder two MBR120 diodes onto the PCB pads. In this photo the diode cathodes point down.

![Solar_Panel_1](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Solar_Panel_1.JPG)

Cut six pieces of double sided foam tape and stick them onto the PCB fingers. I recommend using good quality acrylic adhesive tape,
e.g. 3M VHB Tape 4941P.

![Solar_Panel_2](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Solar_Panel_2.JPG)

Stick the two solar panels to the foam tape, paying close attention to the panel orientation. In this photo both panels are oriented so the minus
connections are at the bottom.

![Solar_Panel_3](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Solar_Panel_3.JPG)

Take four header pins and bend one end of each through 90 degrees. Solder these in position, applying solder to both sides of the PCB for added
strength.

![Solar_Panel_4](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Solar_Panel_4.JPG)

Solder the pins to the panels; apply heat and flow solder onto each pin until the top plastic coat on the panel melts and the solder flows onto the
electrode beneath. Take care not to apply excessive heat or you may damage the panel.

![Solar_Panel_5](https://github.com/PaulZC/Iridium_9603N_Solar_Beacon/blob/master/img/Solar_Panel_5.JPG)

Pass the beacon SMA connector through the hole in the center of the solar panel support. Use two more 90 degree header pins to link the two, soldering
the pins to both beacon and solar panel PCBs.

### The Small Print

This project is distributed under a Creative Commons Attribution + Share-alike (BY-SA) licence.
Please refer to section 5 of the licence for the "Disclaimer of Warranties and Limitation of Liability".

Enjoy!

**_Paul_**