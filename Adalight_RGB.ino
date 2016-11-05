// Arduino serial-interface for controlling a single rgb LED (or LED strip). The serial interface is based on the
// Adalight protocol. The protocol is as follows:
// [prefix][led count][checksum][red, green, blue]*
// prefix    := 'Ada'
// led count := number of leds - 1  (uint16_t big endian)
// checksum  := led count high bit XOR led count low bit XOR 0x55 (uint8_t)
// red, green, blue := channel intensity [0-255] (uint8_t)

// Wiring diagram
// Check http://www.jerome-bernard.com/blog/2013/01/12/rgb-led-strip-controlled-by-an-arduino/


/// Definition of the serial-baudrate
/// NOTE: 500kHz seemed to be the maximum (test at 1MHz failed)
#define serialRate 115200

/// The prefix for start sending led-data
static const uint8_t prefix[] = {'A', 'd', 'a'};

/// Set PWM pins (Do not forget GND pin)
/// Order can be set also in .conf file
static const int redPin = 11;
static const int greenPin = 10;
static const int bluePin = 9;

void setup()
{
  // Run test
  runTestSequence();
   
  // Open the serial-connection
  Serial.begin(serialRate);
  
  // Send 'Ada' string to host
  Serial.print("Ada\n"); 
}

void loop() 
{ 
  // Wait for first byte of 'Ada'
  for(int i = 0; i < sizeof(prefix); ++i) 
  {
    while (!Serial.available());
    
    // Check next byte in Magic Word
    if(prefix[i] != Serial.read()) 
      return;
  }
 
  // Wait for highByte, lowByte, Checksum to be available
  while(Serial.available() < 3);
  // Read high and low byte and the checksum
  int highByte = Serial.read();
  int lowByte  = Serial.read();
  int checksum = Serial.read();

  if (checksum != (highByte ^ lowByte ^ 0x55))
  {
    // Checksum check failed
    return;
  }
  
  uint16_t ledCount = ((highByte & 0x00FF) << 8 | (lowByte & 0x00FF) ) + 1;

  // read the transmission data and set LED values
  // Wait for the next color spec to be available
  while(Serial.available() < 3);
  // Read the color spec
  int r = Serial.read();// - 255;
  int g = Serial.read();// - 255;
  int b = Serial.read();// - 255;

  // Set the LED colour
  setColourRgb(r, g, b);
}

// Sets colour of LED via RGB value
void setColourRgb(unsigned int red, unsigned int green, unsigned int blue) {
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}

// Cycles through R G B W to check connections
void runTestSequence()
{
  setColourRgb(255, 0, 0);
  delay(500);
  setColourRgb(0, 255, 0);
  delay(500);
  setColourRgb(0, 0, 255);
  delay(500);
  setColourRgb(255, 255, 255);
  delay(500);
  setColourRgb(0, 0, 0);
}
