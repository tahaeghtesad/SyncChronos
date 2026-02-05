/**
 * VFD Driver Header
 *
 * Driver for FUTABA 8-MD-06INKM Dot Matrix VFD Display
 * Uses PT6301 controller with SPI interface
 */

#ifndef VFD_DRIVER_H
#define VFD_DRIVER_H

#include "config.h"
#include <Arduino.h>
#include <SPI.h>

// PT6301 Commands (typical for Futaba VFD with PT6301 controller)
#define VFD_CMD_DISPLAY_ON 0x00
#define VFD_CMD_DISPLAY_OFF 0x01
#define VFD_CMD_STANDBY 0x02
#define VFD_CMD_SET_BRIGHTNESS 0x04
#define VFD_CMD_SET_CURSOR 0x10
#define VFD_CMD_WRITE_DATA 0x20
#define VFD_CMD_CLEAR_DISPLAY 0x40

class VFDDriver {
public:
  VFDDriver();

  /**
   * Initialize the VFD display
   * Sets up SPI and initializes the display controller
   */
  void begin();

  /**
   * Clear the display
   */
  void clear();

  /**
   * Set display brightness
   * @param brightness 0-240 (0 = off, 240 = maximum)
   */
  void setBrightness(uint8_t brightness);

  /**
   * Get current brightness setting
   * @return Current brightness value
   */
  uint8_t getBrightness() const;

  /**
   * Set cursor position
   * @param position 0-7 for 8-digit display
   */
  void setCursor(uint8_t position);

  /**
   * Print a string to the display
   * @param text String to display (max 8 characters)
   */
  void print(const char *text);

  /**
   * Print a single character at current cursor position
   * @param c Character to display
   */
  void printChar(char c);

  /**
   * Write raw data to a digit position
   * @param position Digit position (0-7)
   * @param data Raw segment data
   */
  void writeRaw(uint8_t position, uint16_t data);

  /**
   * Enter standby mode (low power)
   */
  void standby();

  /**
   * Wake from standby mode
   */
  void wake();

  /**
   * Define a custom character pattern
   * @param slot Custom character slot (0-7)
   * @param pattern Character pattern data
   */
  void defineCustomChar(uint8_t slot, const uint8_t *pattern);

private:
  uint8_t _brightness;
  uint8_t _cursorPos;
  bool _initialized;

  /**
   * Send a command to the VFD controller
   * @param cmd Command byte
   */
  void sendCommand(uint8_t cmd);

  /**
   * Send data to the VFD controller
   * @param data Data byte
   */
  void sendData(uint8_t data);

  /**
   * Begin SPI transaction
   */
  void beginTransaction();

  /**
   * End SPI transaction
   */
  void endTransaction();

  /**
   * Transfer a byte via SPI (LSB first for PT6301)
   * @param data Byte to transfer
   */
  void transferByte(uint8_t data);
};

#endif // VFD_DRIVER_H
