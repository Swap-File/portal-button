/*********************************************************************
This is a library for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

These displays use SPI to communicate, 4 or 5 pins are required to  
interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen below must be included in any redistribution
*********************************************************************/

#include "Adafruit_SSD1306.h"

#include "Adafruit_GFX.h"

// the memory buffer for the LCD

static uint8_t buffer[SSD1306_LCDHEIGHT * SSD1306_LCDWIDTH / 8];

// the most basic function, set a single pixel
void Adafruit_SSD1306::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height()))
    return;

  // check rotation, move pixel around if necessary
  switch (getRotation()) {
    case 1:
      swap(x, y);
      x = WIDTH - x - 1;
      break;
    case 2:
      x = WIDTH - x - 1;
      y = HEIGHT - y - 1;
      break;
    case 3:
      swap(x, y);
      y = HEIGHT - y - 1;
      break;
  }

  // x is which column
  if (color == WHITE)
    buffer[x + (y / 8) * SSD1306_LCDWIDTH] |= (1 << (y & 7));
  else
    buffer[x + (y / 8) * SSD1306_LCDWIDTH] &= ~(1 << (y & 7));
}

// initializer for I2C - we only indicate the reset pin!
Adafruit_SSD1306::Adafruit_SSD1306(int8_t reset) : Adafruit_GFX(SSD1306_LCDWIDTH, SSD1306_LCDHEIGHT) {
  sclk = dc = cs = sid = -1;
  rst = reset;
}

void Adafruit_SSD1306::reset(bool invert) {
  // Init sequence for 128x32 OLED module
  ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);  // 0xD5
  ssd1306_command(0x80);                        // the suggested ratio 0x80
  ssd1306_command(SSD1306_SETMULTIPLEX);        // 0xA8
  ssd1306_command(0x1F);
  ssd1306_command(SSD1306_SETDISPLAYOFFSET);    // 0xD3
  ssd1306_command(0x0);                         // no offset
  ssd1306_command(SSD1306_SETSTARTLINE | 0x0);  // line #0
  ssd1306_command(SSD1306_CHARGEPUMP);          // 0x8D

  if (_vccstate == SSD1306_EXTERNALVCC) {
    ssd1306_command(0x10);
  } else {
    ssd1306_command(0x14);
  }
  ssd1306_command(SSD1306_MEMORYMODE);  // 0x20
  ssd1306_command(0x00);                // 0x0 act like ks0108
  ssd1306_command(SSD1306_SEGREMAP | 0x1);
  ssd1306_command(SSD1306_COMSCANDEC);
  ssd1306_command(SSD1306_SETCOMPINS);  // 0xDA
  ssd1306_command(0x02);
  ssd1306_command(SSD1306_SETCONTRAST);  // 0x81
  ssd1306_command(0x8F);
  ssd1306_command(SSD1306_SETPRECHARGE);  // 0xd9
  if (_vccstate == SSD1306_EXTERNALVCC) {
    ssd1306_command(0x22);
  } else {
    ssd1306_command(0xF1);
  }
  ssd1306_command(SSD1306_SETVCOMDETECT);  // 0xDB
  ssd1306_command(0x40);
  ssd1306_command(SSD1306_DISPLAYALLON_RESUME);  // 0xA4
  //ssd1306_command(SSD1306_NORMALDISPLAY);        // 0xA6
  invertDisplay(invert);
  ssd1306_command(SSD1306_DISPLAYON);  //--turn on oled panel
}
void Adafruit_SSD1306::begin(uint8_t vccstate, uint8_t i2caddr) {
  _vccstate = vccstate;
  _i2caddr = i2caddr;

  // I2C Init
  Wire.reset();
  Wire.setSpeed(400000);
  Wire.begin();


}

void Adafruit_SSD1306::invertDisplay(uint8_t i) {
  if (i) {
    ssd1306_command(SSD1306_INVERTDISPLAY);
  } else {
    ssd1306_command(SSD1306_NORMALDISPLAY);
  }
}

void Adafruit_SSD1306::ssd1306_command(uint8_t c) {
  // I2C
  uint8_t control = 0x00;  // Co = 0, D/C = 0
  Wire.beginTransmission(_i2caddr);
  Wire.write(control);
  Wire.write(c);
  Wire.endTransmission();
}

// Dim the display
// dim = true: display is dimmed
// dim = false: display is normal
void Adafruit_SSD1306::dim(bool dim) {
  uint8_t contrast;

  if (dim) {
    contrast = 0;  // Dimmed display
  } else {
    if (_vccstate == SSD1306_EXTERNALVCC) {
      contrast = 0x9F;
    } else {
      contrast = 0xCF;
    }
  }
  // the range of contrast to too small to be really useful
  // it is useful to dim the display
  ssd1306_command(SSD1306_SETCONTRAST);
  ssd1306_command(contrast);
}

void Adafruit_SSD1306::ssd1306_data(uint8_t c) {
  // I2C
  uint8_t control = 0x40;  // Co = 0, D/C = 1
  Wire.beginTransmission(_i2caddr);
  Wire.write(control);
  Wire.write(c);
  Wire.endTransmission();
}

void Adafruit_SSD1306::display(void) {
  ssd1306_command(SSD1306_COLUMNADDR);
  ssd1306_command(0);    // Column start address (0 = reset)
  ssd1306_command(127);  // Column end address (127 = reset)

  ssd1306_command(SSD1306_PAGEADDR);
  ssd1306_command(0);                                  // Page start address (0 = reset)
  ssd1306_command((SSD1306_LCDHEIGHT == 64) ? 7 : 3);  // Page end address

  for (uint16_t i = 0; i < (SSD1306_LCDWIDTH * SSD1306_LCDHEIGHT / 8); i++) {
    // send a bunch of data in one xmission
    Wire.beginTransmission(_i2caddr);
    Wire.write(0x40);
    Wire.write(&buffer[i], 16);
    i += 15;
    Wire.endTransmission();
  }
}

// clear everything
void Adafruit_SSD1306::clearDisplay(void) {
  memset(buffer, 0, (SSD1306_LCDWIDTH * SSD1306_LCDHEIGHT / 8));
}
