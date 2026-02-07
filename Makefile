# SyncChronos Build System
# Wraps PlatformIO commands for convenience

# PlatformIO Executable
PIO = $(HOME)/.platformio/penv/bin/pio

.PHONY: all build build-vfd build-max upload upload-vfd upload-max monitor clean install-deps help test release

# Default target
all: build

# Build all firmware variants
build: build-vfd build-max

# Build VFD firmware (default)
build-vfd:
	$(PIO) run -e esp8266

# Build MAX7219 LED matrix firmware
build-max:
	$(PIO) run -e esp8266_max7219

# Build and upload VFD firmware to ESP8266
upload: upload-vfd

upload-vfd:
	$(PIO) run -e esp8266 --target upload

# Build and upload MAX7219 firmware
upload-max:
	$(PIO) run -e esp8266_max7219 --target upload

# Open serial monitor (115200 baud)
monitor:
	$(PIO) device monitor --baud 115200

# Build, upload, and monitor in one command
run: upload monitor

run-max: upload-max monitor

# Clean build artifacts
clean:
	$(PIO) run --target clean
	rm -rf release/

# Install PlatformIO and project dependencies
install-deps:
	pip install --upgrade platformio
	$(PIO) pkg install

# Update all libraries to latest versions
update-libs:
	$(PIO) pkg update

# Show project environment info
info:
	$(PIO) project config
	@echo ""
	@echo "Source files:"
	@ls -la src/*.cpp src/*.h 2>/dev/null || true

# Run all tests
test: test-native

# Run native logic tests (host)
test-native:
	$(PIO) test -e native

# Run embedded hardware tests (device) - requires connected device
test-embedded:
	$(PIO) test -e esp8266

# Verify API keys and network (host)
verify:
	python3 scripts/verify_apis.py

# Generate compilation database for IDE support
compiledb:
	$(PIO) run --target compiledb

# Create local release package
release: build
	@mkdir -p release
	@VERSION=$$(git describe --tags --always 2>/dev/null || echo "dev"); \
	cp .pio/build/esp8266/firmware.bin release/SyncChronos_VFD_$$VERSION.bin; \
	cp .pio/build/esp8266_max7219/firmware.bin release/SyncChronos_MAX7219_$$VERSION.bin; \
	echo "Release files created in release/"
	@ls -la release/

# Upload filesystem (LittleFS) to device
upload-fs:
	$(PIO) run -e esp8266 --target uploadfs

# Help
help:
	@echo "SyncChronos Build System"
	@echo ""
	@echo "Usage: make [target]"
	@echo ""
	@echo "Build Targets:"
	@echo "  build          Build all firmware variants (default)"
	@echo "  build-vfd      Build VFD display firmware"
	@echo "  build-max      Build MAX7219 LED matrix firmware"
	@echo ""
	@echo "Upload Targets:"
	@echo "  upload         Build and upload VFD firmware"
	@echo "  upload-vfd     Build and upload VFD firmware"
	@echo "  upload-max     Build and upload MAX7219 firmware"
	@echo "  upload-fs      Upload LittleFS filesystem"
	@echo ""
	@echo "Development:"
	@echo "  monitor        Open serial monitor (115200 baud)"
	@echo "  run            Build, upload VFD, and monitor"
	@echo "  run-max        Build, upload MAX7219, and monitor"
	@echo "  clean          Clean build artifacts"
	@echo "  compiledb      Generate compile_commands.json"
	@echo ""
	@echo "Testing:"
	@echo "  test           Run all tests (native)"
	@echo "  test-native    Run logic tests on host"
	@echo "  test-embedded  Run hardware tests on device"
	@echo "  verify         Verify API keys and network"
	@echo ""
	@echo "Release:"
	@echo "  release        Create local release package"
	@echo ""
	@echo "Setup:"
	@echo "  install-deps   Install PlatformIO and dependencies"
	@echo "  update-libs    Update libraries to latest versions"
	@echo "  info           Show project configuration"
	@echo "  help           Show this help"
