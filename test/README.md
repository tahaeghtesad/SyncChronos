# Unit Tests

This project uses PlatformIO's Unity testing framework.

## Running Tests

To run the tests on the embedded device (ESP8266):

```bash
pio test -e esp8266
```

Note: The device must be connected via USB.

## Test Groups

- **test_weather**: Verifies JSON parsing logic for weather data.
- **test_config**: Verifies configuration defaults and accessors.
- **test_time**: Verifies time formatting helpers.
