// ###################################
// # Iridium 9603N Solar Beacon Base #
// ###################################

// Written for the Iridium 9603N Solar Beacon
// This code acts as a 'base station'. When connected to a laptop, allows messages from other Iridium Beacons to be received via RockBLOCK

// Message delivery is via Rock7's RockBLOCK:
// http://www.rock7mobile.com/products-rockblock-9603
// http://www.rock7mobile.com/downloads/RockBLOCK-9603-Developers-Guide.pdf (see last page)

// Assumptions:
// Power will be provided by USB. No battery voltage monitoring is necessary.
// No sleep functionality is required. Base beacon is always on.
// Code will be accessed via the Arduino IDE Serial Monitor - or - matching Python code.

// GNSS / Iridium antenna switching is via a Skyworks AS179-92LF RF Switch
// Switching is performed by applying either EXT_PWR or 3V3SW to the AS179's V1 and V2 pins
// EXT_PWR is the 5.3V power rail for the 9603N (switched via Q2 and Q3, enabled by pulling MISO/D22 high)
// 3V3SW is the 3.3V power rail for the MAX-M8Q (switched via Q1, enabled by pulling D11 low)
// Take great care to make sure EXT_PWR and 3V3SW are not enabled at the same time!
// BADS THINGS WILL PROBABLY HAPPEN IF YOU DO ENABLE BOTH SIMULTANEOUSLY!

// With grateful thanks to Mikal Hart:
// Based on Mikal's IridiumSBD Beacon example: https://github.com/mikalhart/IridiumSBD
// Requires Mikal's TinyGPS library: https://github.com/mikalhart/TinyGPS
// and PString: http://arduiniana.org/libraries/pstring/

// With grateful thanks to:
// Adafruit: https://learn.adafruit.com/using-atsamd21-sercom-to-add-more-spi-i2c-serial-ports/creating-a-new-serial
// MartinL: https://forum.arduino.cc/index.php?topic=341054.msg2443086#msg2443086

// The Iridium_9603_Beacon PCB is based extensively on the Adafruit Feather M0 (Adalogger)
// https://www.adafruit.com/products/2796

// GPS data provided by u-blox MAX-M8Q
// https://www.u-blox.com/en/product/max-m8-series

// Iridium 9603N is interfaced to M0 using Serial2
// D6 (Port A Pin 20) = Enable (Sleep) : Connect to 9603 ON/OFF Pin 5
// D10 (Port A Pin 18) = Serial2 TX : Connect to 9603 Pin 6
// D12 (Port A Pin 19) = Serial2 RX : Connect to 9603 Pin 7
// D21 (Port A Pin 23) = Ring Indicator: Connected to 9603 Pin 12

// Power to the 9603N is switched by a P-channel MOSFET
// The MOSFET gate is pulled high by a 10K resistor. The gate is pulled low by a 2N2222 NPN transistor
// Power is enabled by pulling the base of the transistor high
// The transistor base is connected to MISO / D22

// Iridium 9603 is powered from Linear Technology LTC3225 SuperCapacitor Charger
// (fitted with 2 x 10F 2.7V caps e.g. Bussmann HV1030-2R7106-R)
// to provide the 1.3A peak current when the 9603 is transmitting.
// Charging 10F capacitors to 5.3V at 60mA could take ~7 minutes!
// (~6.5 mins to PGOOD, ~7 mins to full charge)
// 5.3V is OK as the 9603N has an extended supply voltage range of +5 V +/- 0.5 V
// http://www.linear.com/product/LTC3225
// D5 (Port A Pin 15) = LTC3225 ~Shutdown
// A1 / D15 (Port B Pin 8) = LTC3225 PGOOD

// MAX-M8Q GNSS is interfaced to M0 using Serial1
// D1 (Port A Pin 10) = Serial1 TX : Connect to GPS RX
// D0 (Port A Pin 11) = Serial1 RX : Connect to GPS TX
// D11 (Port A Pin 16) = GPS ENable : Connect to GPS EN(ABLE)

// D13 (Port A Pin 17) = WB2812B NeoPixel + single Red LED
// D9 (Port A Pin 7) = AIN 7 : Bus Voltage / 2
// D14 (Port A Pin 2) = AIN 0 : 1.25V precision voltage reference

// As the supply voltage drops, reported VBUS on A7 drops to ~3.4V and then flatlines
// as the 3.3V rail starts to collapse. The 1.25V reference on A0 allows lower voltages to be measured
// as the signal on A0 will appear to _rise_ as the 3.3V rail collapses.

// Red LED on D13 shows when the SAMD is in bootloader mode (LED will fade up/down)

#include <IridiumSBD.h> // Requires V2: https://github.com/mikalhart/IridiumSBD
#include <TinyGPS.h> // NMEA parsing: http://arduiniana.org
#include <PString.h> // String buffer formatting: http://arduiniana.org

// Serial2 pin and pad definitions (in Arduino files Variant.h & Variant.cpp)
#define PIN_SERIAL2_RX       (34ul)               // Pin description number for PIO_SERCOM on D12
#define PIN_SERIAL2_TX       (36ul)               // Pin description number for PIO_SERCOM on D10
#define PAD_SERIAL2_TX       (UART_TX_PAD_2)      // SERCOM pad 2 (SC1PAD2)
#define PAD_SERIAL2_RX       (SERCOM_RX_PAD_3)    // SERCOM pad 3 (SC1PAD3)
// Instantiate the Serial2 class
Uart Serial2(&sercom1, PIN_SERIAL2_RX, PIN_SERIAL2_TX, PAD_SERIAL2_RX, PAD_SERIAL2_TX);
HardwareSerial &ssIridium(Serial2);

#define ssGPS Serial1 // Use M0 Serial1 to interface to the MAX-M8Q

// Leave the "#define GALILEO" uncommented to use: GPS + Galileo + GLONASS + SBAS
// Comment the "#define GALILEO" out to use the default u-blox M8 GNSS: GPS + SBAS + QZSS + GLONASS
#define GALILEO

// Set Nav Mode to Portable
static const uint8_t setNavPortable[] = {
  0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0x01, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 
  0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// Set Nav Mode to Pedestrian
static const uint8_t setNavPedestrian[] = {
  0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0x01, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 
  0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// Set Nav Mode to Automotive
static const uint8_t setNavAutomotive[] = {
  0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0x01, 0x00, 0x04, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 
  0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// Set Nav Mode to Sea
static const uint8_t setNavSea[] = {
  0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0x01, 0x00, 0x05, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 
  0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// Set Nav Mode to Airborne <1G
static const uint8_t setNavAir[] = {
  0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0x01, 0x00, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 
  0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static const int len_setNav = 42;

// Set NMEA Config
// Set trackFilt to 1 to ensure course (COG) is always output
// Set Main Talker ID to 'GP' to avoid having to modify TinyGPS
static const uint8_t setNMEA[] = {
  0xb5, 0x62, 0x06, 0x17, 0x14, 0x00, 0x20, 0x40, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const int len_setNMEA = 26;

// Set GNSS Config to GPS + Galileo + GLONASS + SBAS (Causes the M8 to restart!)
static const uint8_t setGNSS[] = {
  0xb5, 0x62, 0x06, 0x3e, 0x3c, 0x00,
  0x00, 0x20, 0x20, 0x07,
  0x00, 0x08, 0x10, 0x00, 0x01, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x03, 0x00, 0x01, 0x00, 0x01, 0x01,
  0x02, 0x04, 0x08, 0x00, 0x01, 0x00, 0x01, 0x01,
  0x03, 0x08, 0x10, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x04, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x03,
  0x05, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x05,
  0x06, 0x08, 0x0e, 0x00, 0x01, 0x00, 0x01, 0x01 };
static const int len_setGNSS = 66;

// Clear Stored Configuration
// Clears msgConf, navConf, rxmConf
static const uint8_t clearConf[] = {
  0xb5, 0x62, 0x06, 0x09, 0x0C, 0x00,
  0x1A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
// Save Configuration
// Saves msgConf, navConf, rxmConf
static const uint8_t saveConf[] = {
  0xb5, 0x62, 0x06, 0x09, 0x0C, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
// Load Configuration
// Loads msgConf, navConf, rxmConf
static const uint8_t loadConf[] = {
  0xb5, 0x62, 0x06, 0x09, 0x0C, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00 };
static const int len_Conf = 18;

static const int LTC3225shutdown = 5; // LTC3225 ~Shutdown on pin D5
static const int LTC3225PGOOD = 15; // LTC3225 PGOOD on pin A1 / D15
static const int Enable_9603N = 22; // 9603N Enable (enables EXT_PWR via P-MOSFET)
static const int RingIndicator = 21; // Iridium Ring Indicator connected to D21
static const int IridiumSleepPin = 6; // Iridium Sleep connected to D6
IridiumSBD isbd(ssIridium, IridiumSleepPin); // This should disable the 9603
TinyGPS tinygps;
static const int ledPin = 13; // WB2812B + Red LED on pin D13
long iterationCounter = 0; // Increment each time a transmission is attempted

static const int GPS_EN = 11; // GPS Enable on pin D11
#define GPS_ON LOW
#define GPS_OFF HIGH
#define VAP A7 // Bus voltage analog pin (bus voltage divided by 2)
#define VREF A0 // 1.25V precision voltage reference
#define VBUS_NORM 3.3 // Normal bus voltage for battery voltage calculations
#define VREF_NORM 1.25 // Normal reference voltage for battery voltage calculations
#define VBAT_LOW 3.05 // Minimum voltage for LTC3225

// Loop Steps
#define init          0
#define start_LTC3225 1
#define menu_choice   2
#define read_GPS      3
#define read_pressure 4
#define start_9603    5
#define flush_queue   6
#define read_battery  7
#define power_down    8
#define send_message  9

// Variables used by Loop
int year;
byte month, day, hour, minute, second, hundredths;
unsigned long dateFix, locationFix;
float latitude, longitude;
long altitude;
float speed;
short satellites;
long course;
long hdop;
bool fixFound = false;
bool charsSeen = false;
int loop_step = init;
float vbat = 5.0;
float vref = VREF_NORM;
float vrail = VBUS_NORM;
int PGOOD;
unsigned long tnow;

// IridiumSBD Callbacks
bool ISBDCallback()
{
  // 'Flash' the LED
  if ((millis() / 1000) % 2 == 1) {
    digitalWrite(ledPin, HIGH);
  }
  else {
    digitalWrite(ledPin, LOW);
  }
  return true;
}
// V2 console and diagnostic callbacks (replacing attachConsole and attachDiags)
// Comment the next two lines to disable diagnostic messages
//void ISBDConsoleCallback(IridiumSBD *device, char c) { Serial.write(c); }
//void ISBDDiagsCallback(IridiumSBD *device, char c) { Serial.write(c); }

// Interrupt handler for SERCOM1 (essential for Serial2 comms)
void SERCOM1_Handler()
{
  Serial2.IrqHandler();
}

// Read 'battery' voltage
void get_vbat() {
  // Measure the reference voltage and calculate the rail voltage
  vref = analogRead(VREF) * (VBUS_NORM / 1023.0);
  vrail = VREF_NORM * VBUS_NORM / vref;

  vbat = analogRead(VAP) * (2.0 * vrail / 1023.0); // Read 'battery' voltage from resistor divider, correcting for vrail
}

// Send message in u-blox UBX format
// Calculates and appends the two checksum bytes
// Doesn't add the 0xb5 and 0x62 sync chars (these need to be included at the start of the message)
void sendUBX(const uint8_t *message, const int len) {
  int csum1 = 0; // Checksum bytes
  int csum2 = 0;
  for (int i=0; i<len; i++) { // For each byte in the message
    ssGPS.write(message[i]); // Write the byte
    if (i >= 2) { // Don't include the sync chars in the checksum
      csum1 = csum1 + message[i]; // Update the checksum bytes
      csum2 = csum2 + csum1;
    }
  }
  csum1 = csum1 & 0xff; // Limit checksums to 8-bits
  csum2 = csum2 & 0xff;
  ssGPS.write((uint8_t)csum1); // Send the checksum bytes
  ssGPS.write((uint8_t)csum2);
}

// Flash the LED quickly
void flash_LED(unsigned long duration) {
  for (tnow = millis(); millis() - tnow < duration * 1000UL;)
  {
    if ((millis() / 100) % 2 == 1) {
      digitalWrite(ledPin, HIGH);
    }
    else {
      digitalWrite(ledPin, LOW);
    }
  }
}

void setup()
{
  pinMode(LTC3225shutdown, OUTPUT); // LTC3225 supercapacitor charger shutdown pin
  digitalWrite(LTC3225shutdown, HIGH); // Enable the LTC3225 supercapacitor charger
  pinMode(LTC3225PGOOD, INPUT); // Define an input for the LTC3225 Power Good signal
  
  pinMode(Enable_9603N, OUTPUT); // 9603N enable via P-FET and NPN transistor
  digitalWrite(Enable_9603N, LOW); // Disable the 9603N
  
  pinMode(GPS_EN, OUTPUT); // GPS enable
  digitalWrite(GPS_EN, GPS_ON); // Enable the GPS
  
  pinMode(IridiumSleepPin, OUTPUT); // The call to IridiumSBD should have done this - but just in case
  digitalWrite(IridiumSleepPin, LOW); // Disable the Iridium 9603
  pinMode(RingIndicator, INPUT_PULLUP); // Define an input for the Iridium 9603 Ring Indicator signal

  pinMode(ledPin, OUTPUT); // LED
  digitalWrite(ledPin, LOW); // Disable the LED
  
  loop_step = init; // Make sure loop_step is set to init
}

void loop()
{
  unsigned long loopStartTime = millis();
  int choice;
  int err;

  switch(loop_step) {

    case init:
      {
      // Start the serial console
      Serial.begin(115200);
      //delay(5000); // Wait 5 secs - allow time for user to open serial monitor
    
      // Send welcome message
      Serial.println("Iridium 9603N Solar Beacon Base:");
      
      // Setup the IridiumSBD
      // (attachConsole and attachDiags methods have been replaced with ISBDConsoleCallback and ISBDDiagsCallback)
      isbd.setPowerProfile(IridiumSBD::USB_POWER_PROFILE); // Change power profile to "low current"
      isbd.useMSSTMWorkaround(false); // Redundant?
      isbd.adjustSendReceiveTimeout(60); // Default is 300 seconds

      // Start the GPS serial port
      ssGPS.begin(9600);

      delay(1000); // Allow time for the port to open

      // Configure GPS
      Serial.println("Configuring GNSS");

      // Initialise the M8 by clearing the stored configuration and then loading it
      sendUBX(clearConf, len_Conf); // Clear stored configuration
      delay(100);
      sendUBX(loadConf, len_Conf); // Load configuration
      delay(2100);

      // Disable all messages except GGA and RMC
      ssGPS.println("$PUBX,40,GLL,0,0,0,0*5C"); // Disable GLL
      delay(1100);
      ssGPS.println("$PUBX,40,ZDA,0,0,0,0*44"); // Disable ZDA
      delay(1100);
      ssGPS.println("$PUBX,40,VTG,0,0,0,0*5E"); // Disable VTG
      delay(1100);
      ssGPS.println("$PUBX,40,GSV,0,0,0,0*59"); // Disable GSV
      delay(1100);
      ssGPS.println("$PUBX,40,GSA,0,0,0,0*4E"); // Disable GSA
      delay(1100);
      
      sendUBX(setNavPortable, len_setNav); // Set Portable Navigation Mode
      //sendUBX(setNavPedestrian, len_setNav); // Set Pedestrian Navigation Mode
      //sendUBX(setNavAutomotive, len_setNav); // Set Automotive Navigation Mode
      //sendUBX(setNavSea, len_setNav); // Set Sea Navigation Mode
      //sendUBX(setNavAir, len_setNav); // Set Airborne <1G Navigation Mode
      delay(1100);

      sendUBX(setNMEA, len_setNMEA); // Set NMEA: to always output COG; and set main talker to GP (instead of GN)
      delay(1100);

#ifdef GALILEO
      sendUBX(setGNSS, len_setGNSS); // Set GNSS - causes M8 to restart!
      delay(3000); // Wait an extra time for GNSS to restart
#endif

      sendUBX(saveConf, len_Conf); // Save configuration
      delay(1100);

      digitalWrite(GPS_EN, GPS_OFF); // Disable the GPS

      while(ssGPS.available()){ssGPS.read();} // Flush RX buffer so we don't confuse TinyGPS with UBX acknowledgements

      // Start TinyGPS
      tinygps = TinyGPS();

      // Prepare Iridium serial port
      ssIridium.begin(19200);

      loop_step = start_LTC3225;
      }
      break;
      
    case start_LTC3225:
      {
      // Power up the LTC3225EDDB super capacitor charger
      Serial.println("Powering up the LTC3225EDDB...");
      Serial.println("Waiting for PGOOD to go HIGH...");
      
      // Allow 10 mins for LTC3225 to achieve PGOOD
      PGOOD = digitalRead(LTC3225PGOOD);
      for (tnow = millis(); !PGOOD && millis() - tnow < 10UL * 60UL * 1000UL;)
      {
        // 'Flash' the LED
        if ((millis() / 1000) % 2 == 1) {
          digitalWrite(ledPin, HIGH);
        }
        else {
          digitalWrite(ledPin, LOW);
        }

        PGOOD = digitalRead(LTC3225PGOOD);
      }
      if (PGOOD == LOW) {
        Serial.println("ERROR: LTC3225 PGOOD failed to go high");
        flash_LED(5); // Flash LED quickly for 5 seconds
        digitalWrite(ledPin, LOW);
        while (true) ; // Do nothing more...
      }

      // Allow extra time for the super capacitors to charge
      Serial.println("PGOOD has gone HIGH");
      Serial.println("Allowing extra time to make sure capacitors are charged...");
      
      // Allow 20 secs for extra charging
      PGOOD = digitalRead(LTC3225PGOOD);
      for (tnow = millis(); PGOOD && millis() - tnow < 1UL * 20UL * 1000UL;)
      {
        // 'Flash' the LED
        if ((millis() / 1000) % 2 == 1) {
          digitalWrite(ledPin, HIGH);
        }
        else {
          digitalWrite(ledPin, LOW);
        }

        PGOOD = digitalRead(LTC3225PGOOD);
      }

      // Show menu
      Serial.println();
      Serial.println("Menu:");
      Serial.println("=====");
      Serial.println();
      Serial.println("1: Read Battery");
      Serial.println("2: Read GNSS");
      Serial.println("3: (N/A)");
      Serial.println("4: Check for an Iridium Message");
      Serial.println("5: Flush MT queue (RockBLOCK only)");
      Serial.println("6: Power down");
      Serial.println("7: Send an Iridium Message");
      Serial.println();
      Serial.println("If you are using Serial Monitor to send commands: set the line ending to Carriage Return");
      Serial.println();
      Serial.println("For option 7: send 7 followed by CR; then the message followed by CR");
      Serial.println();

      loop_step = menu_choice;
      }
      break;

    case menu_choice:
      {
      digitalWrite(ledPin, HIGH);

      // Wait for the arrival of a one (or two digit) int menu choice followed by a CR
      int choice = 0;
      char receivedChars[3];
      while(Serial.available()==0) ; // Wait for first character
      receivedChars[0] = Serial.read(); // Read the first character
      if (isDigit(receivedChars[0])) { // Check if first character is a number
        while(Serial.available()==0) ; // Wait for second character
        receivedChars[1] = Serial.read(); // Read the second character
        if (isDigit(receivedChars[1])) { // Check if second character is a number
          while(Serial.available()==0) ; // Wait for third character
          receivedChars[2] = Serial.read(); // Read the third character
          if (receivedChars[2] == '\r') { // If the third character is CR
            receivedChars[2] = 0; // NULL-terminate the number
            choice = atoi(receivedChars); // Convert to int
          }
        }
        else if (receivedChars[1] == '\r') { // Second character was not a number so check if it is a CR
            receivedChars[1] = 0; // Second character was a CR so NULL-terminate the number
            choice = atoi(receivedChars); // Convert to int
        }
      }

      if (choice == 1) loop_step = read_battery;
      else if (choice == 2) loop_step = read_GPS;
      else if (choice == 3) loop_step = read_pressure;
      else if (choice == 4) loop_step = start_9603;
      else if (choice == 5) loop_step = flush_queue;
      else if (choice == 6) loop_step = power_down;
      else if (choice == 7) loop_step = send_message;
      else Serial.println("ERROR: invalid menu choice"); // Comment this line out to ignore invalid choices or extra CR LF
      }
      break;

    case read_GPS:
      {
      digitalWrite(GPS_EN, GPS_ON); // Enable the GPS using the saved configuration
      delay(2100); // Wait
      while(ssGPS.available()){ssGPS.read();} // Flush RX buffer so we get a fresh fix
      fixFound = false; // Clear fixFound so we get a fresh fix
      charsSeen = false; // Clear charsSeen

      // Look for GPS signal for up to 60 seconds
      for (tnow = millis(); !fixFound && millis() - tnow < 1UL * 60UL * 1000UL;)
      {
        // 'Flash' the LED
        if ((millis() / 500) % 2 == 1) {
          digitalWrite(ledPin, HIGH);
        }
        else {
          digitalWrite(ledPin, LOW);
        }

        if (ssGPS.available())
        {
          charsSeen = true;
          if (tinygps.encode(ssGPS.read()))
          {
            tinygps.f_get_position(&latitude, &longitude, &locationFix);
            tinygps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &dateFix);
            altitude = tinygps.altitude(); // Altitude in cm (long)
            speed = tinygps.f_speed_mps(); // Get speed - checks that we have received an RMC message
            satellites = tinygps.satellites(); // Get number of satellites
            course = tinygps.course(); // Get course over ground
            hdop = tinygps.hdop(); // Get horizontal dilution of precision
            fixFound = locationFix != TinyGPS::GPS_INVALID_FIX_TIME && 
                       dateFix != TinyGPS::GPS_INVALID_FIX_TIME && 
                       altitude != TinyGPS::GPS_INVALID_ALTITUDE &&
                       speed != TinyGPS::GPS_INVALID_F_SPEED &&
                       satellites != TinyGPS::GPS_INVALID_SATELLITES &&
                       course != TinyGPS::GPS_INVALID_ANGLE &&
                       hdop != TinyGPS::GPS_INVALID_HDOP &&
                       year != 2000;
          }
        }

        // if we haven't seen any GPS data in 10 seconds, then stop waiting
        if (!charsSeen && millis() - tnow > 10000) {
          break;
        }
      }

      //Serial.println(charsSeen ? fixFound ? F("A GPS fix was found!") : F("No GPS fix was found.") : F("Wiring error: No GPS data seen."));
      char outBuffer[100];

      if (charsSeen and fixFound) {
        sprintf(outBuffer, "%d%02d%02d%02d%02d%02d,", year, month, day, hour, minute, second);
        int len = strlen(outBuffer);
        PString str(outBuffer + len, sizeof(outBuffer) - len);
        str.print(latitude, 6);
        str.print(",");
        str.print(longitude, 6);
        str.print(",");
        str.print(altitude / 100); // Convert altitude from cm to m
        str.print(",");
        str.print(speed, 1); // Speed in metres per second
        str.print(",");
        str.print(course / 100); // Convert from 1/100 degree to degrees
        str.print(",");
        str.print((((float)hdop) / 100),1); // Convert from 1/100 m to m
        str.print(",");
        str.print(satellites);
        Serial.println(outBuffer);
      }
      else {
        // No GPS fix found!
        Serial.println("ERROR: no GNSS fix");
      }

      digitalWrite(GPS_EN, GPS_OFF); // Disable the GPS
      
      loop_step = menu_choice;
      }
      break;

    case read_pressure:
      {
      Serial.println("0,0.0");

      loop_step = menu_choice;
      }
      break;

    case read_battery:
      {
      get_vbat();
      Serial.println(vbat,2);

      loop_step = menu_choice;
      }
      break;

    case start_9603:
      {
      digitalWrite(ledPin, HIGH);
        
      digitalWrite(Enable_9603N, HIGH); // Enable the 9603N
      delay(2000);

      err = isbd.begin(); // isbd.begin powers up the 9603
      if (err != ISBD_SUCCESS) {
        Serial.print("ERROR: isbd.begin failed with error ");
        Serial.println(err);
        flash_LED(5); // Flash LED quickly for 5 seconds
        digitalWrite(ledPin, LOW);
        while (true) ; // Do nothing more...          
      }

      uint8_t sbdBuffer[200];
      size_t bufferSize = sizeof(sbdBuffer);

      err = isbd.sendReceiveSBDText(NULL, sbdBuffer, bufferSize);
      if (err == ISBD_SUCCESS) {
        if (bufferSize > 0) {
          for (int i=0; i<bufferSize; ++i) {
            Serial.write(sbdBuffer[i]);
          }
          Serial.print(",");
        }
        Serial.println(isbd.getWaitingMessageCount());
      }
      else {
        flash_LED(2); // Flash LED quickly for 2 seconds
        Serial.print("ERROR: sendReceiveSBDText failed with error ");
        Serial.println(err);
      }

      isbd.sleep(); // Put 9603 to sleep
      delay(1000);
      
      digitalWrite(IridiumSleepPin, LOW); // Disable the Iridium 9603N
      digitalWrite(Enable_9603N, LOW); // Disconnect power to the 9603N

      loop_step = menu_choice;
      }
      break;

    case flush_queue:
      {
      digitalWrite(ledPin, HIGH);
        
      digitalWrite(Enable_9603N, HIGH); // Enable the 9603N
      delay(2000);

      err = isbd.begin(); // isbd.begin powers up the 9603
      if (err != ISBD_SUCCESS) {
        Serial.print("ERROR: isbd.begin failed with error ");
        Serial.println(err);
        flash_LED(5); // Flash LED quickly for 5 seconds
        digitalWrite(ledPin, LOW);
        while (true) ; // Do nothing more...          
      }

      char txBuffer[20];
      sprintf(txBuffer, "FLUSH_MT");

      err = isbd.sendSBDText(txBuffer); // Send the message
      if (err == ISBD_SUCCESS) {
        Serial.println(isbd.getWaitingMessageCount());
      }
      else {
        flash_LED(2); // Flash LED quickly for 2 seconds
        Serial.print("ERROR: sendSBDText failed with error ");
        Serial.println(err);
      }

      // Very messy work-around to clear MO buffer so start_9603 doesn't send FLUSH_MT again!
      ssIridium.println("AT+SBDD0");
      delay(5000);
      while(ssIridium.available()) { // Flush RX buffer
        ssIridium.read(); // Discard anything in the buffer
        //Serial.write(ssIridium.read()); // Or use this for debugging
      }

      isbd.sleep(); // Put 9603 to sleep
      delay(1000);
      
      digitalWrite(IridiumSleepPin, LOW); // Disable the Iridium 9603N
      digitalWrite(Enable_9603N, LOW); // Disconnect power to the 9603N

      loop_step = menu_choice;
      }
      break;

    case power_down:
      {
      isbd.sleep(); // Put 9603 to sleep
      delay(1000);
      digitalWrite(IridiumSleepPin, LOW); // Disable the Iridium 9603N
      digitalWrite(Enable_9603N, LOW); // Disconnect power to the 9603N
      digitalWrite(LTC3225shutdown, LOW); // Disable the LTC3225 supercapacitor charger
      digitalWrite(GPS_EN, GPS_OFF); // Disable the GPS
      delay(2000); // Allow two seconds for 9603N voltages to decay
      digitalWrite(ledPin, LOW); // Turn LED off
      while (true) ; // Do nothing more... (Wait for reset)
      }
      break; // redundant!

    case send_message:
      {
      digitalWrite(ledPin, HIGH);
        
      digitalWrite(Enable_9603N, HIGH); // Enable the 9603N
      delay(2000);

      err = isbd.begin(); // isbd.begin powers up the 9603
      if (err != ISBD_SUCCESS) {
        Serial.print("ERROR: isbd.begin failed with error ");
        Serial.println(err);
        flash_LED(5); // Flash LED quickly for 5 seconds
        digitalWrite(ledPin, LOW);
        while (true) ; // Do nothing more...          
      }

      char txBuffer[51]; // Buffer for outgoing message [50 chars plus NULL]
      byte ptr = 0; // Buffer pointer
      bool keep_going = true; // Flag to keep going
      char rc; // Serial received character

      while (keep_going) { // Keep going until \r or 50 characters have been received
        if (Serial.available() > 0) { // Are there any characters waiting?
          rc = Serial.read(); // Read a single character
          if (rc != '\r') { // Check for a carriage return
            txBuffer[ptr] = rc; // Copy character into txBuffer
            ptr++; // Increment pointer
          }
          else {
            // carriage return has been received so don't keep going
            keep_going = false;
          }
          txBuffer[ptr] = 0; // Ensure string is NULL terminated
          if (ptr == 50) keep_going = false; // If 50 characters have been received then don't keep going
        }
      }

      err = isbd.sendSBDText(txBuffer);
      if (err == ISBD_SUCCESS) {
        Serial.println(isbd.getWaitingMessageCount());
      }
      else {
        flash_LED(2); // Flash LED quickly for 2 seconds
        Serial.print("ERROR: sendSBDText failed with error ");
        Serial.println(err);
      }

      // Very messy work-around to clear MO buffer so start_9603 doesn't send the same message again!
      ssIridium.println("AT+SBDD0");
      delay(5000);
      while(ssIridium.available()) { // Flush RX buffer
        ssIridium.read(); // Discard anything in the buffer
        //Serial.write(ssIridium.read()); // Or use this for debugging
      }

      isbd.sleep(); // Put 9603 to sleep
      delay(1000);
      
      digitalWrite(IridiumSleepPin, LOW); // Disable the Iridium 9603N
      digitalWrite(Enable_9603N, LOW); // Disconnect power to the 9603N

      loop_step = menu_choice;
      }
      break;

  }
}


