/**
 **************************************************
 *
 * @file        UC1701-SOLDERED.h
 * @brief       Header file for LCD code.
 *
 *
 * @copyright GNU General Public License v3.0
 * @authors     Zvonimir Haramustek forß soldered.com
 ***************************************************/

#ifndef __UC1701_SOLDERED__
#define __UC1701_SOLDERED__

#include "Arduino.h"
#include "SPI.h"

#include "Adafruit_GFX.h"

class UC1701_SOLDERED : public Adafruit_GFX
{
  private:
    constexpr static uint8_t screenHMax = 101;
    constexpr static uint8_t screenVMax = 63;
    constexpr static uint8_t screenWidth = screenHMax + 1;
    constexpr static uint8_t screenHeight = screenVMax + 1;

    uint8_t buf[screenWidth * screenHeight / 8];

    int dontSendBuf;
    bool enabled;
    uint8_t xOffset = 0;
    SPIClass* spiInterface; // Pointer to externally initialized SPI
    uint32_t spiClock;
    uint8_t pinCS;
    uint8_t pinCD;
    uint8_t pinRST;
    bool curxFlip;
    bool curyFlip;

    void init();
    
    void sendCommand(uint8_t b);
    void sendData(uint8_t b);
    void sendBuf(uint8_t x0 = 0, uint8_t y0 = 0, uint8_t x1 = screenHMax, uint8_t y1 = screenVMax);


  public:
    UC1701_SOLDERED() : Adafruit_GFX(screenWidth, screenHeight){};

    void begin(SPIClass& spi, uint32_t clockFrequency, uint8_t CS, uint8_t CD, uint8_t RST, bool xFlip = false, bool yFlip = false);

    void clearDisplay();

    void writePixel(int16_t x, int16_t y, uint16_t color);
    void drawPixel(int16_t x, int16_t y, uint16_t color);

    void setInverted(bool inv);
    void setEnabled(bool en);
    void setContrast(uint8_t value);
    void setResistor(uint8_t value);

    void display(int x0 = 0, int y0 = 0, int x1 = screenHMax, int y1 = screenVMax);
};

#endif
