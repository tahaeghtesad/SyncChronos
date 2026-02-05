# VFD Clock Build System
# Wraps PlatformIO commands for convenience

.PHONY: all build upload monitor clean install-deps help

# Default target
all: build

# Build the project
build:
	pio run

# Build and upload to ESP8266
upload:
	pio run --target upload

# Open serial monitor (115200 baud)
monitor:
	pio device monitor --baud 115200

# Build, upload, and monitor in one command
run: upload monitor

# Clean build artifacts
clean:
	pio run --target clean
	rm -rf .pio/

# Install PlatformIO and project dependencies
install-deps:
	pip install --upgrade platformio
	pio pkg install

# Update all libraries to latest versions
update-libs:
	pio pkg update

# Show project environment info
info:
	pio project config

# Run unit tests (if any)
test:
	pio test

# Generate compilation database for IDE support
compiledb:
	pio run --target compiledb

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
	@echo "  test         Run unit tests"
	@echo "  compiledb    Generate compile_commands.json"
	@echo "  help         Show this help"
