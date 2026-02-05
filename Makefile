# VFD Clock Build System
# Wraps PlatformIO commands for convenience

# PlatformIO Executable
PIO = $(HOME)/.platformio/penv/bin/pio

.PHONY: all build upload monitor clean install-deps help

# Default target
all: build

# Build the project
build:
	$(PIO) run

# Build and upload to ESP8266
upload:
	$(PIO) run --target upload

# Open serial monitor (115200 baud)
monitor:
	$(PIO) device monitor --baud 115200

# Build, upload, and monitor in one command
run: upload monitor

# Clean build artifacts
clean:
	$(PIO) run --target clean

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

# Run all tests (native + embedded)
test: test-native test-embedded

# Run native logic tests (host)
test-native:
	$(PIO) test -e native

# Run embedded hardware tests (device)
test-embedded:
	$(PIO) test -e esp8266

# Verify API keys and network (host)
verify:
	python3 scripts/verify_apis.py

# Build and upload for MAX7219 display
upload-max:
	$(PIO) run -e esp8266_max7219 --target upload

# Generate compilation database for IDE support
compiledb:
	$(PIO) run --target compiledb

# Help
help:
	@echo "VFD Clock Build System"
	@echo ""
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets:"
	@echo "  build        Build the project (default)"
	@echo "  upload       Build and upload to ESP8266"
	@echo "  monitor      Open serial monitor"
	@echo "  run          Build, upload, and monitor"
	@echo "  clean        Clean build artifacts"
	@echo "  install-deps Install PlatformIO and dependencies"
	@echo "  update-libs  Update libraries to latest versions"
	@echo "  info         Show project configuration"
	@echo "  test         Run all tests (native + embedded)"
	@echo "  test-native  Run logic tests on host"
	@echo "  test-embedded Run hardware tests on device"
	@echo "  verify       Verify API keys and network (host)"
	@echo "  upload-max   Build and upload for MAX7219 display"
	@echo "  compiledb    Generate compile_commands.json"
	@echo "  help         Show this help"
