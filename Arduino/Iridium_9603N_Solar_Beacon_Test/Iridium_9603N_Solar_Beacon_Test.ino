// ###################################
// # Iridium 9603N Solar Beacon Test #
// ###################################

// This code tests the components of the Iridium 9603N Solar Beacon PCB

// Power the beacon PCB via USB and - if possible - monitor the current draw at the same time

// The tests won't start until the Serial Monitor is opened

#include <IridiumSBD.h> // Requires V2: https://github.com/mikalhart/IridiumSBD

static const int ledPin = 13; // Red LED on pin D13

static const int GPS_EN = 11; // GPS Enable on pin D11
#define GPS_ON LOW
#define GPS_OFF HIGH

#define VAP A7 // Bus voltage analog pin (bus voltage divided by 2)
#define VREF A0 // 1.25V precision voltage reference
#define VBUS_NORM 3.3 // Normal bus voltage for battery voltage calculations
#define VREF_NORM 1.25 // Normal reference voltage for battery voltage calculations
#define VBAT_LOW 3.05 // Minimum voltage for LTC3225

// Serial2 pin and pad definitions (in Arduino files Variant.h & Variant.cpp)
#define PIN_SERIAL2_RX       (34ul)               // Pin description number for PIO_SERCOM on D12
#define PIN_SERIAL2_TX       (36ul)               // Pin description number for PIO_SERCOM on D10
#define PAD_SERIAL2_TX       (UART_TX_PAD_2)      // SERCOM pad 2 (SC1PAD2)
#define PAD_SERIAL2_RX       (SERCOM_RX_PAD_3)    // SERCOM pad 3 (SC1PAD3)
// Instantiate the Serial2 class
Uart Serial2(&sercom1, PIN_SERIAL2_RX, PIN_SERIAL2_TX, PAD_SERIAL2_RX, PAD_SERIAL2_TX);
HardwareSerial &ssIridium(Serial2); // Use M0 Serial2 to interface to the Iridium 9603N

static const int LTC3225shutdown = 5; // LTC3225 ~Shutdown on pin D5
static const int LTC3225PGOOD = 15; // LTC3225 PGOOD on pin A1 / D15
static const int Enable_9603N = 22; // 9603N Enable (enables EXT_PWR via P-MOSFET)
static const int IridiumSleepPin = 6; // Iridium Sleep connected to D6
IridiumSBD isbd(ssIridium, IridiumSleepPin); // This should disable the 9603

#define ssGPS Serial1 // Use M0 Serial1 to interface to the MAX-M8Q

// Globals
float vbat = 5.3;
float vref = VREF_NORM;
float vrail = VBUS_NORM;
unsigned long tnow;
int PGOOD;

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

// http://forum.arduino.cc/index.php?topic=288234.0
const byte numChars = 32;
char receivedChars[numChars]; // an array to store the received data
boolean newData = false;

void recvWithEndMarker() {
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (rc != endMarker) {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    }
    else {
      receivedChars[ndx] = '\0'; // terminate the string
      ndx = 0;
      newData = true;
    }
  }
}

void waitForLF() {
  Serial.println("Press Send to continue...");
  Serial.println();
  while (newData == false) {
    recvWithEndMarker();
  }
  newData = false;
}

// Iridium SBD V2 console and diagnostic callbacks (replacing attachConsole and attachDiags)
void ISBDConsoleCallback(IridiumSBD *device, char c) { Serial.write(c); }
void ISBDDiagsCallback(IridiumSBD *device, char c) { Serial.write(c); }

void setup()
{
  pinMode(LTC3225shutdown, OUTPUT); // LTC3225 supercapacitor charger shutdown pin
  digitalWrite(LTC3225shutdown, LOW); // Disable the LTC3225 supercapacitor charger
  pinMode(LTC3225PGOOD, INPUT); // Define an input for the LTC3225 Power Good signal
  
  pinMode(Enable_9603N, OUTPUT); // 9603N enable via P-FET and NPN transistor
  digitalWrite(Enable_9603N, LOW); // Disable the 9603N
  
  pinMode(GPS_EN, OUTPUT); // GPS enable
  digitalWrite(GPS_EN, GPS_OFF); // Disable the GPS
  
  //pinMode(IridiumSleepPin, OUTPUT); // Iridium 9603N Sleep Pin
  //digitalWrite(IridiumSleepPin, LOW); // Disable the Iridium 9603

  pinMode(ledPin, OUTPUT); // LED
  digitalWrite(ledPin, LOW); // Disable the LED
}

void loop()
{
  // Start the serial console
  Serial.begin(115200);
  while (!Serial) ; // Wait for the user to open the serial console

  // Send welcome message
  Serial.println("Iridium 9603N Solar Beacon Test");
  Serial.println();
  Serial.println("Check that the Serial Monitor baud rate is set to 115200");
  Serial.println("and that the line ending is set to Newline");
  Serial.println();
  Serial.println("Confirm that the beacon is being powered via USB");
  waitForLF();

  // Check VREF and VBUS
  get_vbat(); // Read 'battery' voltage
  Serial.print("VREF is ");
  Serial.print(vref);
  Serial.print("V : ");
  if ((vref >= 1.20) and (vref <= 1.30))
    Serial.println("PASS");
  else
    Serial.println("FAIL!");
  Serial.println();
  
  Serial.print("VBUS is ");
  Serial.print(vbat);
  Serial.print("V : ");
  if ((vbat >= 4.60) and (vbat <= 5.20))
    Serial.println("PASS");
  else
    Serial.println("FAIL!");
  Serial.println();

  // Check current draw
  Serial.println("Check current draw is approx. 14mA");
  waitForLF();
  Serial.println();

  // Test LED
  Serial.println("Check LED is illuminated");
  digitalWrite(ledPin, HIGH);
  waitForLF();
  
  Serial.println("Check LED is off");
  digitalWrite(ledPin, LOW);
  waitForLF();
  Serial.println();

  // Power up GNSS
  Serial.println("Powering up MAX-M8Q");
  Serial.println("Check current draw rises to approx. 40mA");
  digitalWrite(GPS_EN, GPS_ON); // Enable the GPS
  waitForLF();

  // Check GNSS
  // Start the GPS serial port
  ssGPS.begin(9600);

  delay(1000); // Allow time for the port to open

  // Configure GNSS
  Serial.println("Configuring MAX-M8Q...");

  // Disable all messages except GGA and RMC
  ssGPS.println("$PUBX,40,GLL,0,0,0,0*5C"); // Disable GLL
  delay(100);
  ssGPS.println("$PUBX,40,ZDA,0,0,0,0*44"); // Disable ZDA
  delay(100);
  ssGPS.println("$PUBX,40,VTG,0,0,0,0*5E"); // Disable VTG
  delay(100);
  ssGPS.println("$PUBX,40,GSV,0,0,0,0*59"); // Disable GSV
  delay(100);
  ssGPS.println("$PUBX,40,GSA,0,0,0,0*4E"); // Disable GSA
  delay(1100);
      
  // Flush GNSS serial buffer
  while(ssGPS.available()){ssGPS.read();} // Flush RX buffer

  Serial.println();

  for (tnow = millis(); millis() - tnow < 1UL * 5UL * 1000UL;) {
    while(ssGPS.available()){Serial.write(ssGPS.read());}
  }

  Serial.println();
  Serial.println();
  Serial.println("Confirm that GNSS is producing _only_ GNGGA and GNRMC messages");
  Serial.println("Any other messages - or no messages - is a fail");
  waitForLF();

  digitalWrite(GPS_EN, GPS_OFF); // Disable the GPS

  // Power up the LTC3225
  Serial.println("Powering up LTC3225");
  Serial.println("Check current draw peaks at approx. 140mA");
  digitalWrite(LTC3225shutdown, HIGH); // Enable the LTC3225 supercapacitor charger
  waitForLF();

  Serial.println("Waiting for up to 600 seconds for PGOOD to go high...");

  PGOOD = digitalRead(LTC3225PGOOD);
  for (tnow = millis(); !PGOOD && millis() - tnow < 1UL * 600UL * 1000UL;) {
    PGOOD = digitalRead(LTC3225PGOOD);
  }

  if (PGOOD) Serial.println("PGOOD has gone high : PASS");
  else Serial.println("PGOOD did not go high : FAIL!");

  Serial.println();
  Serial.println("(Now would be a good time to measure the super capacitor voltage)");
  Serial.println();

  // Enable and test 9603N
  Serial.println("Powering up the Iridium 9603N");
  Serial.println("(Could take up to 240 seconds)");
  Serial.println();
  digitalWrite(Enable_9603N, HIGH); // Enable the 9603N
  delay(2000); // Wit for 9603N to power up

  ssIridium.begin(19200);
  delay(1000);

  if (isbd.begin() == ISBD_SUCCESS) { // isbd.begin powers up the 9603
    Serial.println();
    Serial.println("Iridium 9603N begin was successful : PASS");
  }
  else {
    Serial.println();
    Serial.println("Iridium 9603N begin was unsuccessful : FAIL!");
  }
  Serial.println();
  
  isbd.sleep(); // Put 9603N to sleep
  delay(1000);

  // Put processor to sleep, confirm minimal current draw

  ssIridium.end(); // Close GPS and Iridium serial ports
  ssGPS.end();

  digitalWrite(LTC3225shutdown, LOW); // Disable the LTC3225 supercapacitor charger
  digitalWrite(Enable_9603N, LOW); // Disable the 9603N
  digitalWrite(GPS_EN, GPS_OFF); // Disable the GPS
  
  Serial.println();
  Serial.println("Last test: putting the processor into deep sleep");
  Serial.println("Confirm current draw falls to approx. 3mA");
  delay(1000); // Wait for serial port to clear
  Serial.end(); // Close the serial console
  USBDevice.detach(); // Safely detach the USB prior to sleeping

  // Deep sleep...
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
  __WFI();

  while (true) ; // Wait for reset
}


