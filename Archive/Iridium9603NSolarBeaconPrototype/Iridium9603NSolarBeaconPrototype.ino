// ########################################
// # Iridium 9603N Solar Beacon Prototype #
// ########################################

// A version of the Iridium 9603N Beacon Prototype powered by a PowerFilm MPT3.6-150 solar panel

// With grateful thanks to Mikal Hart:
// Based on Mikal's IridiumSBD Beacon example: https://github.com/mikalhart/IridiumSBD
// Requires Mikal's TinyGPS library: https://github.com/mikalhart/TinyGPS
// and PString: http://arduiniana.org/libraries/pstring/

// With grateful thanks to:
// Adafruit: https://learn.adafruit.com/using-atsamd21-sercom-to-add-more-spi-i2c-serial-ports/creating-a-new-serial
// MartinL: https://forum.arduino.cc/index.php?topic=341054.msg2443086#msg2443086

// Hacked by Paul to run on the Adafruit Feather M0 (Adalogger)
// https://www.adafruit.com/products/2796
// GPS data provided by Adafruit Ultimate GPS Breakout
// https://www.adafruit.com/products/746
// Don't fit the CR1220 back-up battery. VBAT can be provided by the Adalogger 3V rail.
// You can use the Ultimate GPS FeatherWing (https://www.adafruit.com/products/3133) but:
//   You will need to invert the GPS Enable (GPS_EN)
//   You won't be able to power down the MPL3115A2
//   You can still use the Adalogger 3V rail instead of the CR1220 battery, but 
//   you will need to solder a wire to the BAT pad on the bottom of the FeatherWing
// Pressure (altitude) and temperature provided by Sparkfun MPL3115A2 Breakout
// https://www.sparkfun.com/products/11084
// The Adafruit MPL3115A2 Breakout will work too
// https://www.adafruit.com/products/1893
// Requires Adafruit's MPL3115A2 library
// https://github.com/adafruit/Adafruit_MPL3115A2_Library

// Uses RTCZero to provide sleep functionality (on the M0)
// https://github.com/arduino-libraries/RTCZero

// With grateful thanks to CaveMoa for his SimpleSleepUSB example
// https://github.com/cavemoa/Feather-M0-Adalogger
// https://github.com/cavemoa/Feather-M0-Adalogger/tree/master/SimpleSleepUSB
// Note: you will need to close and re-open your serial monitor each time the M0 wakes up

// With thanks to David A. Mellis and Tom Igoe for the smoothing tutorial
// http://www.arduino.cc/en/Tutorial/Smoothing

// Iridium 9603N is interfaced to Adalogger using Serial2
// D6 (Port A Pin 20) = Enable (Sleep) : Connect to 9603 ON/OFF Pin 5
// D10 (Port A Pin 18) = Serial2 TX : Connect to 9603 Pin 6
// D12 (Port A Pin 19) = Serial2 RX : Connect to 9603 Pin 7
// A3 / D17 (Port A Pin 4) = Network Available : Connect to 9603 Pin 19
// Iridium 9603 is powered from Linear Technology LTC3225 SuperCapacitor Charger
// (fitted with 2 x 10F 2.7V caps e.g. Bussmann HV1030-2R7106-R)
// to provide the 1.3A peak current when the 9603 is transmitting.
// Charging 10F capacitors to 5.3V at 60mA could take ~7 minutes!
// (~6.5 mins to PGOOD, ~7 mins to full charge)
// 5.3V is OK as the 9603N has an extended supply voltage range of +5 V +/- 0.5 V
// http://www.linear.com/product/LTC3225
// http://www.linear.com/product/LTC3225#demoboards
// D5 (Port A Pin 15) = LTC3225 ~Shutdown
// A1 / D15 (Port B Pin 8) = LTC3225 PGOOD
// Connect LTC3225 VIN to Adalogger VBUS / USB
// Connect LTC3225 GND to Adalogger GND
// Connect LTC3225 VOUT to Iridium 9603 EXT_PWR Pin 1 + 2
// Connect LTC3225 GND to Iridium 9603 EXT_GND Pin 3 + 4
// Connect 9603 RTS Pin 13 to SIG_GND Pin 18 (9603 won't communicate without this)
// Connect 9603 DTR Pin 14 to SIG_GND Pin 15 (9603 won't communicate without this)

// Ultimate GPS is interfaced to Adalogger using Serial1
// D1 (Port A Pin 10) = Serial1 TX : Connect to GPS RX
// D0 (Port A Pin 11) = Serial1 RX : Connect to GPS TX
// D11 (Port A Pin 16) = GPS ENable : Connect to GPS EN(ABLE)
// Connect Ultimate GPS VIN to Adalogger VBUS / USB
// Connect Ultimate GPS GND to Adalogger GND
// Connect Ultimate GPS VBAT to Adalogger 3V (no need for a CR1220 backup battery)

// Adalogger has a built-in SD card slot
// D22 (Port A Pin 12) = MISO : Connected to SD Card
// D23 (Port B Pin 10) = MOSI : Connected to SD Card
// D24 (Port B Pin 11) = SCK : Connected to SD Card

// MPL3115A2 Pressure (Altitude) and Temperature Sensor
// D20 (Port A Pin 22) = SDA : Connect to MPL3115A2 SDA
// D21 (Port A Pin 23) = SCL : Connect to MPL3115A2 SCL
// Connect MPL3115A2 VCC/Vin to Ultimate GPS 3.3Vout (so GPS_EN can disable both GPS and MPL3115A2 to save power)
// Connect MPL3115A2 GND to Adalogger GND

// More Adalogger pins
// D13 (Port A Pin 17) = Red LED
// D9 (Port A Pin 7) = AIN 7 : Battery Voltage / 2
// D8 (Port A Pin 6) = Green LED
// D16 / A2 (Port B Pin 9) : Reserve for (e.g.) DS3231 RTC Featherwing Interrupt

// If you are using the LTC3225 demo circuit (1220B):
// Set VOUT_SELECT to 5.3V (Connect VSEL to VIN)
// Set ICHRG to 60mA (Connect PROG to GND via 30k)
// Remove the Run/SHDN link (the Adalogger controls the SHDN pin)
// Remove the 0.25F low profile capacitor from the rear of the PCB
// Fit two (e.g.) Bussmann HV1030-2R7106-R 10F 2.7V caps in OPT C3 and OPT C4
// There is no thermal relief on the OPT C4 GND pin and it can be difficult to solder
// You might have to link the cap leg to the low profile cap negative pad instead

// I recommend removing the MCP73831 LiPo Charger from the Adalogger to prevent possible damage to
// the solar panel. (If you power the board from USB, the LiPo Charger will attempt to charge the solar panel!)
// You can remove the charger by cutting through the chip legs with a scalpel blade.

// Connect PowerFilm MPT3.6-150 Solar Panel +ve to Adalogger VBAT (not GPS VBAT!)
// Connect PowerFilm MPT3.6-150 Solar Panel -ve to Adalogger GND

// Connect a reset supervisor to ensure a clean start when the sun rises:
// Connect Microchip MCP111-195 VSS to Adalogger GND
// Connect Microchip MCP111-195 VDD to Adalogger 3V
// Connect Microchip MCP111-195 VOUT to Adalogger RST

#include <IridiumSBD.h>
#include <TinyGPS.h> // NMEA parsing: http://arduiniana.org
#include <PString.h> // String buffer formatting: http://arduiniana.org

#include <RTCZero.h> // M0 Real Time Clock
RTCZero rtc; // Create an rtc object
int BEACON_INTERVAL = 30; // Define how often messages are sent in MINUTES (suggested values: 30,60,120,180,240) (max 1440)

// MPL3115A2
#include <Wire.h>
#include <Adafruit_MPL3115A2.h>
Adafruit_MPL3115A2 baro = Adafruit_MPL3115A2();

// Serial2 pin and pad definitions (in Arduino files Variant.h & Variant.cpp)
#define PIN_SERIAL2_RX       (34ul)               // Pin description number for PIO_SERCOM on D12
#define PIN_SERIAL2_TX       (36ul)               // Pin description number for PIO_SERCOM on D10
#define PAD_SERIAL2_TX       (UART_TX_PAD_2)      // SERCOM pad 2 (SC1PAD2)
#define PAD_SERIAL2_RX       (SERCOM_RX_PAD_3)    // SERCOM pad 3 (SC1PAD3)
// Instantiate the Serial2 class
Uart Serial2(&sercom1, PIN_SERIAL2_RX, PIN_SERIAL2_TX, PAD_SERIAL2_RX, PAD_SERIAL2_TX);
HardwareSerial &ssIridium(Serial2);

#define ssGPS Serial1 // Use Adalogger (Cortex M0) Serial1 to interface to the Ultimate GPS

static const int IridiumSleepPin = 6; // Iridium Sleep connected to D6
IridiumSBD isbd(ssIridium, IridiumSleepPin); // This should disable the 9603
TinyGPS tinygps;
static const int ledPin = 13; // Red LED on pin D13
long iterationCounter = 0; // Increment each time a transmission is attempted

static const int greenLED = 8; // Green LED on pin D8
static const int networkAvailable = 17; // 9602 Network Available on pin D17
static const int LTC3225shutdown = 5; // LTC3225 ~Shutdown on pin D5
static const int LTC3225PGOOD = 15; // LTC3225 PGOOD on pin A1 / D15
static const int GPS_EN = 11; // Ultimate GPS Enable on pin D11
#define GPS_ON HIGH // HIGH for the Ultimate GPS Breakout, LOW for the Ultimate GPS FeatherWing
#define GPS_OFF LOW // LOW for the Ultimate GPS Breakout, HIGH for the Ultimate GPS FeatherWing
#define VAP A7 // Solar panel voltage analog pin (panel voltage divided by 2)

// Loop Steps
#define init          0
#define start_GPS     1
#define read_GPS      2
#define read_pressure 3
#define start_LTC3225 4
#define wait_LTC3225  5
#define start_9603    6
#define zzz           7
#define wake          8

// Variables used by Loop
int year;
byte month, day, hour, minute, second, hundredths;
unsigned long dateFix, locationFix;
float latitude, longitude;
long altitude;
bool fixFound = false;
bool charsSeen = false;
int loop_step = init;
float vbat = 5.3;
float pascals, tempC;
int PGOOD;
unsigned long tnow;

// Storage for the average voltage during Iridium callbacks
const int numReadings = 25;   // number of samples
int readings[numReadings];    // the readings from the analog input
int readIndex = 0;            // the index of the current reading
long int total = 0;           // the running total
int latest_reading = 0;       // the latest reading
int average_reading = 0;      // the average reading

// IridiumSBD Callback
bool ISBDCallback()
{
  // Check solar panel voltage now we are drawing current for the 9603
  // If panel voltage is low, stop Iridium send
  // Average voltage over numReadings to smooth out any short dips

   // subtract the last reading:
   total = total - readings[readIndex];
   // read from the sensor:
   latest_reading = analogRead(VAP);
   readings[readIndex] = latest_reading;
   // add the reading to the total:
   total = total + latest_reading;
   // advance to the next position in the array:
   readIndex = readIndex + 1;
   // if we're at the end of the array...wrap around to the beginning:
   if (readIndex >= numReadings) readIndex = 0;
   // calculate the average:
   average_reading = total / numReadings; // Seems to work OK with integer maths - but total does need to be long int
   vbat = float(average_reading) * (2.0 * 3.3 / 1023.0);
  
  if (vbat < 3.0) {
    Serial.print("***!!! LOW VOLTAGE (ISBDCallback) ");
    Serial.print(vbat,2);
    Serial.println("V !!!***");
    return false; // Returning false causes IridiumSBD to terminate
  }
  else {     
    return true;
  }
}

// Interrupt handler for SERCOM1 (essential for Serial2 comms)
void SERCOM1_Handler()
{
  Serial2.IrqHandler();
}

// RTC alarm interrupt
void alarmMatch()
{
  int rtc_mins = rtc.getMinutes(); // Read the RTC minutes
  int rtc_hours = rtc.getHours(); // Read the RTC hours
  if (BEACON_INTERVAL > 1440) BEACON_INTERVAL = 1440; // Limit BEACON_INTERVAL to one day
  rtc_mins = rtc_mins + BEACON_INTERVAL; // Add the BEACON_INTERVAL to the RTC minutes
  while (rtc_mins >= 60) { // If there has been an hour roll over
    rtc_mins = rtc_mins - 60; // Subtract 60 minutes
    rtc_hours = rtc_hours + 1; // Add an hour
  }
  rtc_hours = rtc_hours % 24; // Check for a day roll over
  rtc.setAlarmMinutes(rtc_mins); // Set next alarm time (minutes)
  rtc.setAlarmHours(rtc_hours); // Set next alarm time (hours)
}

// Read solar panel voltage
void get_vbat() {
  vbat = analogRead(VAP) * (2.0 * 3.3 / 1023.0);
}

void setup()
{
  rtc.begin(); // Start the RTC
  rtc.setAlarmSeconds(rtc.getSeconds()); // Initialise RTC Alarm Seconds
  alarmMatch(); // Set next alarm time
  rtc.enableAlarm(rtc.MATCH_HHMMSS); // Alarm Match on hours, minutes and seconds
  rtc.attachInterrupt(alarmMatch); // Attach alarm interrupt
  
  pinMode(ledPin, INPUT); // Adalogger Red LED - turn off to save power
  pinMode(greenLED, INPUT); // Adalogger Green LED - turn off to save power
  
  pinMode(LTC3225shutdown, OUTPUT); // LTC3225 supercapacitor charger shutdown pin
  digitalWrite(LTC3225shutdown, LOW); // Disable the LTC3225 supercapacitor charger
  pinMode(LTC3225PGOOD, INPUT); // Define an input for the LTC3225 Power Good signal
  
  pinMode(GPS_EN, OUTPUT); // Adafruit Ultimate GPS enable
  digitalWrite(GPS_EN, GPS_OFF); // Disable the GPS (and MPL3115A2)
  
  pinMode(IridiumSleepPin, OUTPUT); // The call to IridiumSBD should have done this - but just in case
  digitalWrite(IridiumSleepPin, LOW); // Disable the Iridium 9603
  pinMode(networkAvailable, INPUT); // Define an input for the Iridium 9603 Network Available signal

  iterationCounter = 0; // Make sure iterationCounter is set to zero (indicating a reset)
  loop_step = init; // Make sure loop_step is set to init

  // Initialise voltage sample buffer to 5.3V
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 822; // 5.3V * 1023 / (2 * 3.3)
  }
  total = numReadings * 822;
  vbat = 5.3;
}

void loop()
{
  unsigned long loopStartTime = millis();

  switch(loop_step) {

    case init:

      // Start the serial console
      Serial.begin(115200);
      delay(20000); // Wait 20 secs - allow time for user to open serial monitor
    
      // Send welcome message
      Serial.println("Iridium 9603N Solar Beacon");
      
      // Setup the IridiumSBD
      isbd.attachConsole(Serial);
      isbd.attachDiags(Serial);
      isbd.setPowerProfile(1);
      isbd.useMSSTMWorkaround(false);

      // Check solar panel voltage
      // If panel voltage is low, go to sleep
      get_vbat();
      if (vbat < 3.0) {
        Serial.print("***!!! LOW VOLTAGE (init) ");
        Serial.print(vbat,2);
        Serial.println(" !!!***");
        loop_step = zzz;
      }
      else {
        loop_step = start_GPS;
      }
      
      break;
      
    case start_GPS:
      // Power up the GPS and MPL3115A2
      Serial.println("Powering up the GPS and MPL3115A2...");
      digitalWrite(GPS_EN, GPS_ON); // Enable the GPS and MPL3115A2

      delay(2000); // Allow time for both to start
    
      // Check solar panel voltage now we are drawing current for the GPS
      // If panel voltage is low, go to sleep
      get_vbat();
      if (vbat < 3.0) {
        Serial.print("***!!! LOW VOLTAGE (start_GPS) ");
        Serial.print(vbat,2);
        Serial.println("V !!!***");
        loop_step = zzz;
      }
      else {
        loop_step = read_GPS;
      }
      
      break;

    case read_GPS:
      // Start the GPS serial port
      ssGPS.begin(9600);

      delay(1000); // Allow time for the port to open

      // Configure Ultimate GPS
      Serial.println("Configuring GPS...");
      ssGPS.println("$PMTK220,1000*1F"); // Set NMEA Update Rate to 1Hz
      delay(100);
      ssGPS.println("$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28"); // Set NMEA Output to GGA and RMC
      delay(100);
      ssGPS.println("$PGCMD,33,0*6D"); // Disable Antenna Updates
      delay(1100); // Delay for > 1 second
    
      // Reset TinyGPS and begin listening to the GPS
      Serial.println("Beginning to listen for GPS traffic...");
      tinygps = TinyGPS();
      
      // Look for GPS signal for up to 5 minutes
      for (tnow = millis(); !fixFound && millis() - tnow < 5UL * 60UL * 1000UL;)
      {
        if (ssGPS.available())
        {
          charsSeen = true;
          if (tinygps.encode(ssGPS.read()))
          {
            tinygps.f_get_position(&latitude, &longitude, &locationFix);
            tinygps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &dateFix);
            altitude = tinygps.altitude();
            fixFound = locationFix != TinyGPS::GPS_INVALID_FIX_TIME && 
                       dateFix != TinyGPS::GPS_INVALID_FIX_TIME && 
                       altitude != TinyGPS::GPS_INVALID_ALTITUDE &&
                       year != 2000;
          }
        }

        // if we haven't seen any GPS data in 10 seconds, then stop waiting
        if (!charsSeen && millis() - tnow > 10000) {
          break;
        }

        // Check solar panel voltage now we are drawing current for the GPS
        // If panel voltage is low, stop looking for GPS and go to sleep
        get_vbat();
        if (vbat < 3.0) {
          break;
        }        
      }

      Serial.println(charsSeen ? fixFound ? F("A GPS fix was found!") : F("No GPS fix was found.") : F("Wiring error: No GPS data seen."));

      if (vbat < 3.0) {
        Serial.print("***!!! LOW VOLTAGE (read_GPS) ");
        Serial.print(vbat,2);
        Serial.println("V !!!***");
        loop_step = zzz;
      }
      else if (!charsSeen) {
        Serial.println("***!!! No GPS data received !!!***");
        loop_step = zzz;
      }
      else {
        loop_step = read_pressure;
      }
      
      break;

    case read_pressure:
      // Start the MPL3115A2 (does Wire.begin())
      if (baro.begin()) {
        pascals = baro.getPressure();
        if (pascals > 110000) pascals = 0.0; // Correct wrap-around if pressure drops too low
        tempC = baro.getTemperature();
      }
      else {
        Serial.println("***!!! Error initialising MPL3115A2 !!!***");
        pascals = 0.0;
        tempC = 0.0;
      }

       // Power down the GPS and MPL3115A2
      Serial.println("Powering down the GPS and MPL3115A2...");
      digitalWrite(GPS_EN, GPS_OFF); // Disable the GPS and MPL3115A2

      loop_step = start_LTC3225;

      break;

    case start_LTC3225:
      // Power up the LTC3225EDDB super capacitor charger
      Serial.println("Powering up the LTC3225EDDB...");
      digitalWrite(LTC3225shutdown, HIGH); // Enable the LTC3225EDDB supercapacitor charger
      delay(1000); // Let PGOOD stabilise
      
      // Allow 10 mins for LTC3225 to achieve PGOOD
      PGOOD = digitalRead(LTC3225PGOOD);
      for (tnow = millis(); !PGOOD && millis() - tnow < 10UL * 60UL * 1000UL;)
      {
        // Check solar panel voltage now we are drawing current for the LTC3225
        // If panel voltage is low, stop LTC3225 and go to sleep
        get_vbat();
        if (vbat < 3.0) {
          break;
        }

        PGOOD = digitalRead(LTC3225PGOOD);

        Serial.println("Waiting for PGOOD to go HIGH...");
        delay(1000);
      }

      // If voltage is low or supercapacitors did not charge then go to sleep
      if (vbat < 3.0) {
        Serial.print("***!!! LOW VOLTAGE (start_LTC3225) ");
        Serial.print(vbat,2);
        Serial.println("V !!!***");
        loop_step = zzz;
      }
      else if (PGOOD == LOW) {
        Serial.println("***!!! LTC3225 !PGOOD (start_LTC3225) !!!***");
        loop_step = zzz;
      }
      // Otherwise start up the Iridium 9603
      else {
        loop_step = wait_LTC3225;
      }
      
      break;

    case wait_LTC3225:
      // Allow extra time for the super capacitors to charge
      Serial.println("Giving the LTC3225EDDB extra time...");
      
      // Allow 1 min for extra charging
      PGOOD = digitalRead(LTC3225PGOOD);
      for (tnow = millis(); PGOOD && millis() - tnow < 1UL * 60UL * 1000UL;)
      {
        // Check solar panel voltage now we are drawing current for the LTC3225
        // If panel voltage is low, stop LTC3225 and go to sleep
        get_vbat();
        if (vbat < 3.0) {
          break;
        }

        PGOOD = digitalRead(LTC3225PGOOD);

        Serial.println("Making sure capacitors are charged...");
        delay(1000);
      }

      // If voltage is low or supercapacitors did not charge then go to sleep
      if (vbat < 3.0) {
        Serial.print("***!!! LOW VOLTAGE (wait_LTC3225) ");
        Serial.print(vbat,2);
        Serial.println("V !!!***");
        loop_step = zzz;
      }
      else if (PGOOD == LOW) {
        Serial.println("***!!! LTC3225 !PGOOD (wait_LTC3225) !!!***");
        loop_step = zzz;
      }
      // Otherwise start up the Iridium 9603
      else {
        loop_step = start_9603;
      }
      
      break;

    case start_9603:
      // Start talking to the 9603 and power it up
      Serial.println("Beginning to talk to the 9603...");

      ssIridium.begin(19200);
      delay(1000);

      if (isbd.begin() == ISBD_SUCCESS) // isbd.begin powers up the 9603
      {
        char outBuffer[80]; // Always try to keep message short
    
        if (fixFound)
        {
          sprintf(outBuffer, "%d%02d%02d%02d%02d%02d,", year, month, day, hour, minute, second);
          int len = strlen(outBuffer);
          PString str(outBuffer + len, sizeof(outBuffer) - len);
          str.print(latitude, 6);
          str.print(",");
          str.print(longitude, 6);
          str.print(",");
          str.print(altitude / 100);
          str.print(",");
          str.print(tinygps.f_speed_knots(), 1);
          str.print(",");
          str.print(tinygps.course() / 100);
          str.print(",");
          str.print(pascals, 0);
          str.print(",");
          str.print(tempC, 1);
          str.print(",");
          str.print(vbat, 2);
          str.print(",");
          str.print(float(iterationCounter), 0);
        }
    
        else
        {
          // No GPS fix found!
          sprintf(outBuffer, "19700101000000,0.0,0.0,0,0.0,0,");
          int len = strlen(outBuffer);
          PString str(outBuffer + len, sizeof(outBuffer) - len);
          str.print(pascals, 0);
          str.print(",");
          str.print(tempC, 1);
          str.print(",");
          str.print(vbat, 2);
          str.print(",");
          str.print(float(iterationCounter), 0);
        }
    
        Serial.print("Transmitting message '");
        Serial.print(outBuffer);
        Serial.println("'");
        isbd.sendSBDText(outBuffer);
        ++iterationCounter; // Increment iterationCounter
      }
      
      loop_step = zzz;

      break;

    case zzz:
      // Get ready for sleep
      Serial.println("Going to sleep until next alarm time...");
      isbd.sleep(); // Put 9603 to sleep
      delay(1000);
      ssIridium.end(); // Close GPS and Iridium serial ports
      ssGPS.end();
      delay(1000); // Wait for serial ports to clear
  
      // Disable both GPS and Iridium supercapacitor charger
      Wire.end(); // Stop I2C comms
      digitalWrite(GPS_EN, GPS_OFF); // Disable the GPS (and MPL3115A2)
      digitalWrite(LTC3225shutdown, LOW); // Disable the LTC3225 supercapacitor charger
  
      // Close and detach the serial console (as per CaveMoa's SimpleSleepUSB)
      Serial.end(); // Close the serial console
      USBDevice.detach(); // Safely detach the USB prior to sleeping
    
      // Sleep until next alarm match
      rtc.standbyMode();
  
      // Wake up!
      loop_step = wake;
  
      break;

    case wake:
      // Attach and reopen the serial console
      USBDevice.attach(); // Re-attach the USB
      delay(1000);  // Delay added to make serial more reliable

      // Now loop back to init
      loop_step = init;

      break;
  }
}


