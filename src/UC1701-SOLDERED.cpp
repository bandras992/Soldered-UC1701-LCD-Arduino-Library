/**
 **************************************************
 *
 * @file        UC1701_SOLDERED-SOLDERED.cpp
 * @brief       All LCD Function definitions
 *
 *
 * @copyright GNU General Public License v3.0
 * @authors   Zvonimir Haramustek forß soldered.com
 ***************************************************/


#include "UC1701-SOLDERED.h"

#include <Arduino.h>

// Helper functions
#ifndef _swap_int16_t
#define _swap_int16_t(a, b)                                                                                            \
    {                                                                                                                  \
        int16_t t = a;                                                                                                 \
        a = b;                                                                                                         \
        b = t;                                                                                                         \
    }
#endif

/**
 * @brief                   Main constructor
 *
 * @param  MOSI             MOSI pin (bit banged SPI)
 * @param  CLK              CLK pin (bit banged SPI)
 * @param  CS               Chip enable pin
 * @param  CD               Command/Data pin
 * @param  RST              Reset pin
 * @param  xFlip            Flip the display horizontally
 * @param  yFlip            Flip the display vertically
 */
void UC1701_SOLDERED::begin(SPIClass& spi,uint32_t clockFrequency, uint8_t CS, uint8_t CD, uint8_t RST, bool xFlip, bool yFlip)
{
    spiInterface = &spi;
    spiClock = clockFrequency;
    pinCS = CS;
    pinCD = CD;
    pinRST = RST;
    curxFlip = xFlip;
    curyFlip = yFlip;
    init();
}

/**
 * @brief                   Send a command to the LCD
 *
 * @param  b                Command to send
 */
void UC1701_SOLDERED::sendCommand(uint8_t b)
{
    digitalWrite(pinCD, LOW);
    // Start transmission
    spiInterface->beginTransaction(SPISettings(spiClock, MSBFIRST, SPI_MODE3));
    digitalWrite(pinCS, LOW);
    spiInterface->transfer(b);
    // End transmission
    digitalWrite(pinCS, HIGH);
    spiInterface->endTransaction();
}

/**
 * @brief                   Send data to the LCD
 *
 * @param  b                Data to send
 */
void UC1701_SOLDERED::sendData(uint8_t b)
{
    digitalWrite(pinCD, HIGH);
    // Start transmission
    spiInterface->beginTransaction(SPISettings(spiClock, MSBFIRST, SPI_MODE3));
    digitalWrite(pinCS, LOW);
    spiInterface->transfer(b);
    // End transmission
    digitalWrite(pinCS, HIGH);
    spiInterface->endTransaction();
}

/**
 * @brief                   Send screen buffer to LCD
 *
 * @param  x0               X start position
 * @param  y0               Y start position
 * @param  x1               X end position
 * @param  y1               Y end position
 */
void UC1701_SOLDERED::sendBuf(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    int p, p0, p1, x;

    if (dontSendBuf > 0)
        return;

    p0 = max(y0 / 8, 0);
    p1 = min(y1, screenVMax) / 8;

    for (p = p0; p <= p1; p++)
    {
        sendCommand(0x10 + (x0 + xOffset) / 16); // Set Column Address MSB
        sendCommand(0x00 + (x0 + xOffset) % 16); // Set Column Address LSB
        sendCommand(0xB0 + p);                   // Set Page Address

        for (x = x0; x <= x1; x++)
            sendData(buf[x + p * screenWidth]);
    }
}

/**
 * @brief                   Write pixel to screen buffer, not physical display
 *
 * @param  x                X position
 * @param  y                Y position
 * @param  color            Pixel color, 1 for white, 0 for black
 */
void UC1701_SOLDERED::writePixel(int16_t x, int16_t y, uint16_t color)
{
    if (x > width() - 1 || y > height() - 1 || x < 0 || y < 0)
        return;

    switch (rotation)
    {
    case 1:
        _swap_int16_t(x, y);
        x = height() - x - 1;
        break;
    case 2:
        x = width() - x - 1;
        y = height() - y - 1;
        break;
    case 3:
        _swap_int16_t(x, y);
        y = width() - y - 1;
        break;
    }

    if ((x >= 0) && (x <= screenHMax) && (y >= 0) && (y <= screenVMax))
    {
        if (enabled && color)
        {
            buf[x + (y / 8) * screenWidth] = buf[x + (y / 8) * screenWidth] | (1 << (y % 8));
        }
        else
        {
            buf[x + (y / 8) * screenWidth] = buf[x + (y / 8) * screenWidth] & (~(1 << (y % 8)));
        }
    }
}

/**
 * @brief                   Draws a pixel to the screen buffer
 *
 * @param  x                X position
 * @param  y                Y position
 * @param  color            Pixel color, 1 for white, 0 for black
 */
void UC1701_SOLDERED::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    writePixel(x, y, color);
}

/**
 * @brief                   Clears the screen buffer
 */
void UC1701_SOLDERED::clearDisplay()
{
    dontSendBuf = 0;
    memset(buf, 0, sizeof(buf));
}

/**
 * @brief                   Send data to screen
 *
 * @param  x0               X start position
 * @param  y0               Y start position
 * @param  x1               X end position
 * @param  y1               Y end position
 */
void UC1701_SOLDERED::display(int x0, int y0, int x1, int y1)
{
    sendBuf(x0, y0, x1, y1);
}

/**
 * @brief                   Sets screens contrast
 */
void UC1701_SOLDERED::setContrast(uint8_t value) // in range 0..63
{
    sendCommand(0x81);
    sendCommand(value & 0x3F);
}

/**
 * @brief                   Sets screens resistor ratio
 */
void UC1701_SOLDERED::setResistor(uint8_t value) // in range 0..7
{
    sendCommand(0x20 + (value & 7));
}

/**
 * @brief                   Sends invert command to the LCD
 */
void UC1701_SOLDERED::setInverted(bool inv)
{
    if (inv)
        sendCommand(0xA6);
    else
        sendCommand(0xA7);
}

/**
 * @brief                   Initializes the LCD
 */
void UC1701_SOLDERED::init()
{
    if (!spiInterface) {
        Serial.println("Error: SPI interface not set!");
        return;
    }

    pinMode(pinCS, OUTPUT);
    pinMode(pinCD, OUTPUT);

    digitalWrite(pinCS, HIGH);
    digitalWrite(pinCD, LOW);
    enabled = true;

    if (pinRST < 255)
    {
        pinMode(pinRST, OUTPUT);
        digitalWrite(pinRST, LOW);
        delay(1);
        digitalWrite(pinRST, HIGH);
        delay(1);
    }

    sendCommand(0xE2); // System Reset
    delay(10);
    sendCommand(0xAE); // Set Display Enable - off
    sendCommand(0x40); // Set Scroll Line
    if (curxFlip)
    {
        sendCommand(0xA1); // Set SEG Direction - on
        xOffset = 0;
    }
    else
    {
        sendCommand(0xA0); // Set SEG Direction - off
        xOffset = 0;
    }
    if (curyFlip)
    {
        sendCommand(0xC8); // Set COM Direction - on
    }
    else
    {
        sendCommand(0xC0); // Set COM Direction - off
    }
    sendCommand(0xA4); // Set All-Pixel - off
    sendCommand(0xA6); // Set Inverse Display - off
    sendCommand(0xA2); // Set LCD Bias Ratio
    sendCommand(0x2F); // Set Power Control - on
    delay(10);
    sendCommand(0x27); // Set VLCD Resistor Ratio
    sendCommand(0x81); // Set Electronic Volume
    sendCommand(0x06); // value
    sendCommand(0xFA); // Set Adv. Program Control
    sendCommand(0x90); // value
    sendCommand(0xA4); // Set All-Pixel - off
    sendCommand(0xAF); // Set Display Enable - on

    clearDisplay();
}

/**
 * @brief                   Sends enable or disable command to the LCD
 */
void UC1701_SOLDERED::setEnabled(bool en)
{
    if (en)
    {
        init();
    }
    else
    {
        sendCommand(0x28); // Set Power Control - off
        delay(10);
        sendCommand(0xAE); // Set Display Enable - off

        if (pinRST < 255)
            digitalWrite(pinRST, 0);
        digitalWrite(pinCS, 0);
        digitalWrite(pinCD, 0);
        delay(1);
    }
}
