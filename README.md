# ESP32 OBD2 Reader

A real-time OBD2 data reader for ESP32 that connects to ELM327 Bluetooth devices to display vehicle diagnostics on an LCD screen and web interface.

## Features

- **Bluetooth Low Energy (BLE) Connection**: Connects to ELM327 v2.1 Chinese adapters
- **WiFi Web Interface**: Web control panel for fuel monitoring
- **Real-time Display**: Shows RPM, engine temperature and engine load on 16x2 LCD
- **Fuel Management System**: Calculates and monitors fuel consumption
- **Modular Architecture**: Clean separation between OBD handling, message processing and web interface
- **Dual-Core System**: WiFi runs on Core 0, OBD2 on Core 1 for better performance
- **Debug Support**: Built-in serial debugging for troubleshooting
- **Preferences Storage**: Saves fuel settings in non-volatile memory

## Hardware Requirements

- ESP32 Development Board (ESP32 DOIT DEVKIT V1)
- 16x2 LCD with I2C backpack (address 0x27)
- ELM327 Bluetooth OBD2 Adapter (v2.1 Chinese version)
- Jumper wires and breadboard

## Wiring

| ESP32 Pin | Component | Notes |
|-----------|-----------|-------|
| GPIO 21 (SDA) | LCD SDA | I2C Data |
| GPIO 22 (SCL) | LCD SCL | I2C Clock |
| 3.3V | LCD VCC | Power |
| GND | LCD GND | Ground |

## Software Dependencies

The project uses PlatformIO with the following libraries:
- `LiquidCrystal_I2C` - LCD display control
- Built-in ESP32 BLE libraries
- Arduino framework
- `Preferences` - Non-volatile storage (ESP32)

## Configuration

Update the following variables in `main.cpp`:

```cpp
// Your ELM327 device MAC address
static String targetAddress = "66:1e:32:7a:35:0e";

// LCD I2C address (usually 0x27)
LiquidCrystal_I2C lcd(0x27, 16, 2);
```

## Web Interface

The system creates a WiFi Access Point called **"ESP32_PAINEL"**:

1. **Connect to WiFi**: ESP32_PAINEL (no password)
2. **Access**: http://192.168.4.1
3. **Available features**:
   - View estimated fuel in liters and percentage
   - Reset tank (full tank)
   - Add 10 liters
   - Consumption factor calibration

## Usage

1. **Initialization**: The system initializes BLE, LCD and WiFi AP on startup
2. **OBD2 Connection**: Automatically attempts to connect to the ELM327 device
3. **ECU Detection**: Waits for vehicle ECU to become available
4. **Data Reading**: Continuously reads and displays:
   - RPM (revolutions per minute)
   - Engine coolant temperature
   - Engine load
5. **Web Interface**: Access the web panel for fuel management

## Fuel Management

The system automatically calculates fuel consumption based on OBD2 data:
- **Consumption Factor**: Calibratable via web interface
- **Tank Capacity**: Configurable for your vehicle
- **Remaining Fuel**: Calculated in real time based on consumption

## Dual-Core System

- **Core 0**: Runs WiFi task and web interface
- **Core 1**: Processes OBD2 data and updates LCD

## Status Indicators

- **LCD Screen**:
  - "Connecting OBD..." - Trying to connect to OBD2 adapter
  - "Wait ECU..." - Connected to adapter, waiting for ECU
  - Real-time data - Normal operation, ECU active

## Project Structure

```
src/
├── main.cpp                 # Main application logic
lib/
├── OBDHandle/              # OBD2 communication handling
│   ├── obdhandle.h
│   └── obdhandle.cpp
├── MessageHandle/          # Message processing and display
│   ├── messagehandle.h
│   └── messagehandle.cpp
├── HTMLInterface/          # WiFi web interface
│   ├── htmlinterface.h
│   └── htmlinterface.cpp
├── PreferencesHandle/      # Settings management
│   ├── preferenceshandle.h
│   └── preferenceshandle.cpp
└── datadefinition.h        # Enums and data structures
```

## Key Classes

### OBDHandle
- Manages BLE connection to ELM327
- Handles OBD2 command sending/receiving
- Provides debug capabilities
- Checks ECU availability

### MessageHandle
- Processes OBD2 response messages
- Handles LCD display updates
- Parses RPM, temperature and engine load data
- Calculates fuel consumption

### HTMLInterface
- Creates WiFi Access Point
- Serves web interface for fuel management
- Handles HTTP requests for panel controls

### PreferencesHandle
- Manages non-volatile storage
- Saves fuel settings
- Implements Singleton pattern for global access

## Debug Mode

Enable debug output by uncommenting these lines in `setup()`:

```cpp
OBDHandle::setDebugSerial(&Serial);
OBDHandle::enableDebug(true);
```

This will output detailed connection and communication logs to the Serial Monitor.

## Supported OBD2 Commands

- `010C` - Engine RPM
- `0105` - Engine coolant temperature
- `0104` - Engine Load
- `0100` - ECU availability check

## Compatible Vehicles

This project works with vehicles that support:
- OBD2 standard (1996+ vehicles in most countries)
- ISO 9141-2 protocol (Ford Street protocol)

**Specifically tested on:**
- Ford Fiesta Street

## Troubleshooting

1. **OBD2 Connection Issues**: 
   - Verify ELM327 MAC address
   - Ensure the adapter is paired
   - Check if vehicle is running or in accessory mode

2. **Web Interface not loading**:
   - Connect to "ESP32_PAINEL" WiFi
   - Access http://192.168.4.1
   - Check if Core 0 is not overloaded

3. **LCD not working**: 
   - Verify I2C address with an I2C scanner
   - Confirm SDA/SCL connections

4. **Build Errors**: 
   - Ensure all library dependencies are installed
   - Check if PlatformIO is updated

5. **Incorrect Fuel Calculation**:
   - Calibrate consumption factor via web interface
   - Check if tank capacity is correct

## Upcoming Features

- [ ] Trip history
- [ ] Real-time consumption graphs
- [ ] Maintenance alerts
- [ ] CSV data export
- [ ] Mobile app connectivity
- [ ] Support for more OBD2 protocols

## License

This project is open source and available under the MIT License.

## Contributing

Feel free to submit issues, fork the repository, and create pull requests for improvements.

## Changelog

### v2.0.0
- ✅ Added WiFi web interface
- ✅ Fuel management system
- ✅ Preferences storage
- ✅ Dual-core architecture
- ✅ Engine Load support (0104)
- ✅ Responsive web interface with modern design

### v1.0.0
- ✅ BLE connection to ELM327
- ✅ LCD display of OBD2 data
- ✅ RPM and temperature reading
- ✅ Debug system
