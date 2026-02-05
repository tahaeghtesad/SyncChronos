/**
 * VFD Driver Implementation
 *
 * Driver for FUTABA 8-MD-06INKM Dot Matrix VFD Display
 *
 * Communication Protocol:
 * - SPI Mode 3 (CPOL=1, CPHA=1)
 * - LSB first
 * - CS is active LOW
 */

#include "vfd_driver.h"

VFDDriver::VFDDriver()
    : _brightness(VFD_DEFAULT_BRIGHTNESS), _cursorPos(0), _initialized(false) {}

void VFDDriver::begin() {
  // Configure pins
  pinMode(VFD_PIN_CS, OUTPUT);
  pinMode(VFD_PIN_CLK, OUTPUT);
  pinMode(VFD_PIN_DATA, OUTPUT);

  digitalWrite(VFD_PIN_CS, HIGH); // CS inactive

// Handle reset pin if connected
#if VFD_PIN_RST >= 0
  pinMode(VFD_PIN_RST, OUTPUT);
  digitalWrite(VFD_PIN_RST, LOW);
  delay(10);
  digitalWrite(VFD_PIN_RST, HIGH);
  delay(10);
#endif

  // Initialize SPI
  // Note: PT6301 uses Mode 3 and LSB first
  SPI.begin();

  delay(100); // Wait for VFD to stabilize

  // Initialize display
  wake();
  setBrightness(_brightness);
  clear();

  _initialized = true;

  Serial.println("VFD Driver initialized");
}

void VFDDriver::clear() {
  // Clear all digit positions
  for (uint8_t i = 0; i < VFD_NUM_DIGITS; i++) {
    setCursor(i);
    sendData(' '); // Space character
  }
  setCursor(0);
}

void VFDDriver::setBrightness(uint8_t brightness) {
  _brightness = brightness;

  // PT6301 brightness is typically 0-240
  // Map 0-255 to 0-240 range
  uint8_t mappedBrightness = map(brightness, 0, 255, 0, 240);

  beginTransaction();
  transferByte(VFD_CMD_SET_BRIGHTNESS);
  transferByte(mappedBrightness);
  endTransaction();
}

uint8_t VFDDriver::getBrightness() const { return _brightness; }

void VFDDriver::setCursor(uint8_t position) {
  if (position >= VFD_NUM_DIGITS) {
    position = 0;
  }
  _cursorPos = position;

  beginTransaction();
  transferByte(VFD_CMD_SET_CURSOR | position);
  endTransaction();
}

void VFDDriver::print(const char *text) {
  setCursor(0);

  uint8_t pos = 0;
  while (*text && pos < VFD_NUM_DIGITS) {
    printChar(*text);
    text++;
    pos++;
  }

  // Pad with spaces if string is shorter than display
  while (pos < VFD_NUM_DIGITS) {
    printChar(' ');
    pos++;
  }
}

void VFDDriver::printChar(char c) {
  beginTransaction();
  transferByte(VFD_CMD_WRITE_DATA);
  transferByte(c);
  endTransaction();

  _cursorPos++;
  if (_cursorPos >= VFD_NUM_DIGITS) {
    _cursorPos = 0;
  }
}

void VFDDriver::writeRaw(uint8_t position, uint16_t data) {
  if (position >= VFD_NUM_DIGITS)
    return;

  setCursor(position);

  beginTransaction();
  transferByte(VFD_CMD_WRITE_DATA | 0x80); // Raw mode flag
  transferByte(data & 0xFF);
  transferByte((data >> 8) & 0xFF);
  endTransaction();
}

void VFDDriver::standby() {
  beginTransaction();
  transferByte(VFD_CMD_STANDBY);
  endTransaction();
}

void VFDDriver::wake() {
  beginTransaction();
  transferByte(VFD_CMD_DISPLAY_ON);
  endTransaction();

  delay(10); // Allow display to wake up
}

void VFDDriver::defineCustomChar(uint8_t slot, const uint8_t *pattern) {
  if (slot >= 8)
    return; // Only 8 custom character slots

  // Custom character definition command
  beginTransaction();
  transferByte(0x80 | slot); // Custom char definition command

  // Send pattern data (typically 5 bytes for 5x7 matrix)
  for (uint8_t i = 0; i < 5; i++) {
    transferByte(pattern[i]);
  }

  endTransaction();
}

void VFDDriver::sendCommand(uint8_t cmd) {
  beginTransaction();
  transferByte(cmd);
  endTransaction();
}

void VFDDriver::sendData(uint8_t data) {
  beginTransaction();
  transferByte(VFD_CMD_WRITE_DATA);
  transferByte(data);
  endTransaction();
}

void VFDDriver::beginTransaction() {
  // SPI Mode 3: CPOL=1, CPHA=1
  SPI.beginTransaction(SPISettings(VFD_SPI_SPEED, LSBFIRST, SPI_MODE3));
  digitalWrite(VFD_PIN_CS, LOW);
  delayMicroseconds(1);
}

void VFDDriver::endTransaction() {
  delayMicroseconds(1);
  digitalWrite(VFD_PIN_CS, HIGH);
  SPI.endTransaction();
}

void VFDDriver::transferByte(uint8_t data) { SPI.transfer(data); }
