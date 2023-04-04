#include <FastLED.h> //FastLED by Daniel Garcia, version 3.5.0

#define NUM_CH 5  // Number of channels
#define DATA_PIN 2 // WS2812b LED digital data pin

// Configuration variables
int pins[NUM_CH] = {A0, A1, A2, A3, A4 /*, please add if more needed*/};   // Analog pins for each channel
uint32_t baseColors[NUM_CH] = {0xFF00FF, 0x00FFFF, 0x00FF00, 0xFFFF00, 0xFF3F00 /*, please add if more needed*/};   // Channel base colors
uint8_t brightnessDivider = 4;  // 1 = full brightness
bool fixedBrightness = false; // if true, does not set brightness = slider value for the channel but leaves it at max.
uint8_t adcBits = 10;           // Normal arduino adc = 10 bits, some boards are 12 bits

// Internal variables
CRGB leds[NUM_CH];
uint16_t adcRange;

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(100);

  // Use NEOPIXEL to keep true colors
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_CH);

  // Initial RGB flash
  LEDS.showColor(CRGB(64, 0, 0));
  delay(500);
  LEDS.showColor(CRGB(0, 64, 0));
  delay(500);
  LEDS.showColor(CRGB(0, 0, 64));
  delay(500);
  LEDS.showColor(CRGB(0, 0, 0));
  delay(500);
}

void loop() 
{
  String outboundMessage = "";
  String inboundMessage = "";

  //Receive custom base colors
  if (Serial.available())
  {
    inboundMessage = Serial.readString();
    
    String optionsString = split(inboundMessage, '|', 0);
    optionsString.trim();
    uint32_t options = optionsString.toInt();
    fixedBrightness = options & 0xFF;
    brightnessDivider = (options >> 8) & 0xFF;
    
    for (uint8_t i=1; i < NUM_CH + 1; i++)
    {
      String colorString = split(inboundMessage, '|', i);
      colorString.trim();
      baseColors[i] = (uint64_t) colorString.toInt(); 
    }
  }


  //For each channel
  for (uint8_t i=0; i < NUM_CH; i++)
  {
    int value = analogRead(pins[i]); //Read pin

    //Build outboundMessage string
    if (i > 0) outboundMessage += "|"; 
    outboundMessage += String(value);

    // Get channel color
    uint64_t color = (uint64_t)baseColors[i]; 
  
    // Set final RGB values
    uint16_t adcRange = (1 << adcBits);

    uint16_t displayedValue = fixedBrightness ? adcRange : value;

    leds[i].r = ((color & 0xFF0000) >> 16) * displayedValue / adcRange / brightnessDivider;
    leds[i].g = ((color & 0x00FF00) >> 8 ) * displayedValue / adcRange / brightnessDivider;
    leds[i].b =  (color & 0x0000FF)        * displayedValue / adcRange / brightnessDivider;
  }

  FastLED.show();
  
  Serial.println(outboundMessage);
  delay(10);
}

String split(String& str, char c, int occ)
{
	String result;
	unsigned int i;

	bool isCopying=false;

	for (i=0; i<str.length(); i++)
	{
		if (str[i] == c)
		{
			if (isCopying)
				isCopying = false;
			occ--;
		}

		if (occ==0)
			isCopying = true;

		if (isCopying && str[i] != c)
			result += str[i];
	}
	
	return result;
}
