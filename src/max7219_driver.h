/**
 * MAX7219 LED Matrix Driver Header
 *
 * Driver for 4x 8x8 LED Dot Matrix Display with MAX7219 controller
 * Uses SPI interface, cascaded configuration
 */

#ifndef MAX7219_DRIVER_H
#define MAX7219_DRIVER_H

#include "config.h"
#include "display_driver.h"
#include <Arduino.h>
#include <SPI.h>

// MAX7219 Register addresses
#define MAX7219_REG_NOOP        0x00
#define MAX7219_REG_DIGIT0      0x01
#define MAX7219_REG_DIGIT1      0x02
#define MAX7219_REG_DIGIT2      0x03
#define MAX7219_REG_DIGIT3      0x04
#define MAX7219_REG_DIGIT4      0x05
#define MAX7219_REG_DIGIT5      0x06
#define MAX7219_REG_DIGIT6      0x07
#define MAX7219_REG_DIGIT7      0x08
#define MAX7219_REG_DECODE      0x09
#define MAX7219_REG_INTENSITY   0x0A
#define MAX7219_REG_SCANLIMIT   0x0B
#define MAX7219_REG_SHUTDOWN    0x0C
#define MAX7219_REG_DISPLAYTEST 0x0F

// Number of cascaded MAX7219 modules
#ifndef MAX7219_NUM_MODULES
#define MAX7219_NUM_MODULES 4
#endif

// Each module is 8 columns wide
#define MAX7219_COLS_PER_MODULE 8
#define MAX7219_TOTAL_COLS (MAX7219_NUM_MODULES * MAX7219_COLS_PER_MODULE)

class MAX7219Driver : public DisplayDriver {
public:
    MAX7219Driver();
    
    /**
     * Initialize the LED matrix display
     */
    void begin() override;
    
    /**
     * Clear the display
     */
    void clear() override;
    
    /**
     * Set display brightness
     * @param brightness 0-255 (mapped to 0-15 for MAX7219)
     */
    void setBrightness(uint8_t brightness) override;
    
    /**
     * Get current brightness setting
     * @return Current brightness value (0-255)
     */
    uint8_t getBrightness() const override;
    
    /**
     * Print a string to the display
     * @param text String to display
     */
    void print(const char* text) override;
    
    /**
     * Set cursor position (column)
     * @param position Column position (0-31)
     */
    void setCursor(uint8_t position) override;
    
    /**
     * Print a single character at current cursor position
     * @param c Character to display
     */
    void printChar(char c) override;
    
    /**
     * Set a specific column of LEDs
     * @param col Column index (0-31)
     * @param data 8-bit column data (each bit = one LED row)
     */
    void setColumn(uint8_t col, uint8_t data);
    
    /**
     * Update display from internal framebuffer
     */
    void refresh();

private:
    uint8_t _brightness;
    uint8_t _cursorCol;
    bool _initialized;
    
    // Framebuffer: 8 rows x 32 columns (4 modules)
    uint8_t _framebuffer[MAX7219_TOTAL_COLS];
    
    /**
     * Send command to all modules
     * @param reg Register address
     * @param data Data byte
     */
    void sendToAll(uint8_t reg, uint8_t data);
    
    /**
     * Send command to specific module
     * @param module Module index (0 = first in chain)
     * @param reg Register address
     * @param data Data byte
     */
    void sendToModule(uint8_t module, uint8_t reg, uint8_t data);
    
    /**
     * Get character glyph from font
     * @param c Character
     * @param width Output: width of character
     * @return Pointer to glyph data
     */
    const uint8_t* getGlyph(char c, uint8_t& width);
};

#endif // MAX7219_DRIVER_H
