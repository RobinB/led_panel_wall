// Simple 3bit color test for Hackspace LED Panel 
// written by Robin Baumgarten (London Hackspace)

// draw 1 white pixel on screen that moves from first to last spot, measuring fps along the way.
// Tested on Teensy 3.6. (I get 788 fps at 240mhz!)

// with the panel the correct way up (you can read "digiled" on the black plastic) these are the pins:
// see also: https://github.com/krux702/led_panel_wall/blob/master/photos/pinout.jpg

// all 8 top pins VCC (5v). Careful: You might need to connect at least two pins, since the halfs of the board use separate vccs

// D1 LAT A1 NC
// D2 OE  A0 CLK

// all 8 bottom pins GND.
// NC = not connected

// populate with your own arduino pins!
#define PIN_CLK   13
#define PIN_ADD0  26
#define PIN_ADD1  25
#define PIN_OE    7
#define PIN_LAT   8
#define PIN_D1  11
#define PIN_D2  12

// pixel storage
bool r[1024];
bool g[1024];
bool b[1024];

int fpsCounter = 0;
long fpsMillisStart = 0;

void initPixels()
{
	for(int i =0; i < 1024; i++)
	{
		r[i] = 0; //(i % 5) == 0;
		g[i] = 0; //(i % 6) == 0;
		b[i] = 0; //(i % 7) == 0;
	}
}

void setup() {

  pinMode (PIN_CLK, OUTPUT); 
  pinMode (PIN_ADD0, OUTPUT); 
  pinMode (PIN_ADD1, OUTPUT); 
  pinMode (PIN_OE, OUTPUT); 
  pinMode (PIN_LAT, OUTPUT); 
  pinMode (PIN_D1, OUTPUT); 
  pinMode (PIN_D2, OUTPUT); 

  delay(10);
  digitalWrite(PIN_OE, LOW);

  initPixels();

  // A0 & A1 control bank to push vals to.
  // 3
  // 2
  // 1
  // 0
  // etc
  // each bank consists of 128 leds
  // d1 is data for bottom half of display
  // d2 is data for top half

  // shifting data:
  // each bank is further split into chunks
  // 0 pixel is bottom right, proceeds to left until hitting 8
  // row then changes to row+4 and continues from column 0
  // colour then changes to next in cycle
  // data is BBBBBBBB BBBBBBBB GGGGGGGG GGGGGGGG RRRRRRRR RRRRRRRR
  //         | row 0 | row 4  | row 0  | row 4  | row 0  | row 4
}

// set a pixel to a certain 3-bit color on the screen. x: 0 - 63, y: 0 - 16
void setPixel(int x, int y, bool cr, bool cg, bool cb)
{
	r[y * 64 + x] = cr;
	g[y * 64 + x] = cg;
	b[y * 64 + x] = cb;
}

bool getBit(int ledIndex, int bank)
{
    // is this the higher row?
	bool isRow4 = (ledIndex / 8) % 2 == 1;

	bool isBlue = (ledIndex % 48) / 16 == 0;
	bool isGreen = (ledIndex % 48) / 16 == 1;
	//bool isRed = (ledIndex % 48) / 16 == 2;

	int x = (ledIndex / 48) * 8 + (ledIndex % 8);

	if (isBlue)
	{
		return b[((isRow4 ? 4 : 0) + bank) * 64 + x];
	}
	else if (isGreen)
	{
		return g[((isRow4 ? 4 : 0) + bank) * 64 + x];
	}	
	else //if (isRed)
	{
		return r[((isRow4 ? 4 : 0) + bank) * 64 + x];
	}
}

bool a0Order[] = {true, false, true, false};
bool a1Order[] = {true, false, false, true};

void drawPixels()
{
	for (int bank = 0; bank < 4; bank++)
	{
		// enable correct bank
		digitalWrite(PIN_ADD0, a0Order[bank]);
	  	digitalWrite(PIN_ADD1, a1Order[bank]);

  		//set latch low to send data
  		digitalWrite(PIN_LAT, LOW);

		for (int i = 0; i < 384; i++) 
        {  // led index
			digitalWrite(PIN_D1, getBit(i, bank));     // write to bottom half
		    digitalWrite(PIN_D2, getBit(i, bank + 8)); // write to top half

		    // toggle clock
		    digitalWrite(PIN_CLK, LOW);
		    digitalWrite(PIN_CLK, HIGH);
		}
		
		digitalWrite(PIN_LAT, HIGH);
	}
}

int c = 1;
void loop() 
{
    // set one of the pixels to white
	setPixel(c % 64, c / 64, true, true, true);
	drawPixels();

    // clear pixel
    setPixel(c % 64, c / 64, false, false, false);

    c++;
    if (c > 1023) c = 0;

    // print out fps
	fpsCounter++;
	if (millis() - fpsMillisStart > 1000)
	{
		fpsMillisStart = millis();
		Serial.print("FPS: ");
		Serial.println(fpsCounter);
		fpsCounter = 0;
	}
}
