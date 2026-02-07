/**
 * Display Driver Interface
 * 
 * Abstract base class for display drivers (VFD, LED Matrix, etc.)
 * Allows build-time selection of display hardware
 */

#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include <Arduino.h>

/**
 * Abstract display driver interface
 * All display implementations must inherit from this class
 */
class DisplayDriver {
public:
    virtual ~DisplayDriver() {}
    
    /**
     * Initialize the display hardware
     */
    virtual void begin() = 0;
    
    /**
     * Clear the display
     */
    virtual void clear() = 0;
    
    /**
     * Set display brightness
     * @param brightness 0-255 (0 = off, 255 = maximum)
     */
    virtual void setBrightness(uint8_t brightness) = 0;
    
    /**
     * Get current brightness setting
     * @return Current brightness value (0-255)
     */
    virtual uint8_t getBrightness() const = 0;
    
    /**
     * Print a string to the display
     * @param text String to display
     */
    virtual void print(const char* text) = 0;
    
    /**
     * Set cursor position (for displays that support it)
     * @param position Character position
     */
    virtual void setCursor(uint8_t position) { (void)position; }
    
    /**
     * Print a single character
     * @param c Character to display
     */
    virtual void printChar(char c) { (void)c; }
    
    /**
     * Set display rotation
     * @param flipped true = 180 degree rotation
     */
    virtual void setRotation(bool flipped) { (void)flipped; }
    
    /**
     * Check if display is rotated
     * @return true if display is flipped 180 degrees
     */
    virtual bool isRotated() const { return false; }
};

#endif // DISPLAY_DRIVER_H
