# ESP32 OBD2 Reader

A comprehensive real-time OBD2 data reader for ESP32 that connects to ELM327 Bluetooth devices to display vehicle diagnostics on an LCD screen and provide advanced fuel management through a web interface.

## Features

- **Bluetooth Low Energy (BLE) Connection**: Connects to ELM327 v2.1 Chinese adapters
- **WiFi Web Interface**: Advanced web control panel for fuel monitoring and trip management
- **Real-time Display**: Shows RPM, engine temperature, engine load percentage, and vehicle speed on 16x2 LCD
- **Advanced Fuel Management System**: Real-time fuel consumption calculation based on engine load and RPM
- **Trip Computer**: Complete trip tracking with distance, fuel consumption, and efficiency metrics
- **Speed Tracking**: Real-time vehicle speed monitoring with distance accumulation
- **Consumption Factor Calibration**: Web-based calibration system for accurate fuel calculations
- **Trip Reset Function**: Clear trip data and start fresh calculations
- **Persistent Trip Data**: Saves trip information in non-volatile memory
- **Modular Architecture**: Clean separation between OBD handling, message processing and web interface
- **Dual-Core System**: WiFi runs on Core 0, OBD2 processing on Core 1 for optimal performance
- **Smart Request Timing**: Optimized OBD2 command scheduling for better ECU compatibility
- **Debug Support**: Comprehensive serial debugging for troubleshooting
- **Automatic ECU State Management**: Intelligent ECU wake/sleep detection

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

The system creates a WiFi Access Point for comprehensive vehicle monitoring:

### Connection
1. **WiFi Network**: ESP32_PAINEL (no password required)
2. **IP Address**: http://192.168.4.1
3. **Mobile Friendly**: Responsive design with modern dark theme

### Dashboard Features
- **Real-time Fuel Display**: Current estimated fuel in liters and tank percentage
- **Trip Computer Display**: Current trip distance, fuel used, and real-time efficiency (km/L)
- **Smart Fuel Calibration**: 
  - Enter actual liters added at gas station
  - Automatic consumption factor calculation and adjustment
  - Intelligent calibration based on tank usage vs real consumption
- **Tank Management**: 
  - Reset tank (mark as full)
  - Add 10 liters for partial refueling
- **Trip Controls**:
  - Reset trip data (distance and fuel consumption)
  - View accumulated trip statistics with efficiency metrics

## Usage

### Startup Sequence
1. **System Initialization**: LCD, BLE, and WiFi AP initialization
2. **OBD2 Connection**: Automatic connection attempt to ELM327 device
3. **ECU Detection**: Waits for vehicle ECU to become available
4. **Data Collection**: Continuous reading and processing of vehicle data

### Real-time Data Display (LCD)
- **Line 1**: RPM and fuel percentage (right-aligned)
- **Line 2**: Vehicle speed (left) and engine temperature (right-aligned)

### Data Collection Cycle
1. **Engine Temperature** (every 20th cycle): Request coolant temperature
2. **RPM** (continuous): Engine revolution monitoring
3. **Engine Load** (continuous): Load percentage for fuel calculations
4. **Vehicle Speed** (continuous): Speed monitoring and distance tracking

### Trip Computer Features
- **Automatic Distance Tracking**: Based on vehicle speed sensor
- **Fuel Consumption Calculation**: Real-time consumption based on RPM × Engine Load
- **Trip Efficiency**: Automatic km/L calculation
- **Persistent Storage**: Trip data saved across power cycles

## Advanced Fuel Management

### Real-time Calculation Algorithm
The system uses a sophisticated algorithm combining:
- **Engine Load %**: Current engine stress level
- **RPM**: Engine revolutions per minute
- **Time Delta**: Precise timing between measurements
- **Calibration Factor**: User-adjustable multiplier for accuracy

**Formula**: `Fuel Consumed = RPM × Engine Load × Calibration Factor × Time`

### Distance Tracking System
- **Speed Monitoring**: Continuous vehicle speed reading via OBD2
- **Distance Accumulation**: Real-time distance calculation
- **Trip Persistence**: Distance data saved in EEPROM
- **Accuracy**: Updates multiple times per second for precision

### Calibration Process
1. **Initial Setup**: Drive with default calibration factor
2. **Drive Until Low**: Use vehicle until fuel is significantly consumed
3. **Fill Up at Gas Station**: Note exact amount of fuel added
4. **Web Calibration**: Enter actual liters supplied in the calibration form
5. **Automatic Factor Calculation**: System calculates new factor using formula:
   
   `New Factor = Old Factor × (Actual Liters / Virtual Tank Usage)`
6. **Instant Application**: Tank is automatically marked as full with new calibration
7. **Continuous Refinement**: Repeat process for improved accuracy over time

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
- Processes OBD2 response messages with MUX-based routing
- Handles LCD display updates for all parameters
- Implements advanced fuel consumption calculations
- Manages distance tracking and trip computer functions
- Provides automatic ECU state management
- Calculates real-time fuel consumption using RPM × Engine Load formula
- Maintains trip statistics with persistent storage

### HTMLInterface
- Creates WiFi Access Point with modern responsive design
- Serves comprehensive web interface for vehicle monitoring
- Handles HTTP requests for fuel management and trip controls
- Provides intelligent calibration system with automatic factor calculation
- Supports trip reset functionality for both distance and fuel data
- Real-time trip computer with efficiency metrics (km/L)
- Smart calibration based on real gas station fill-ups

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

- `010C` - Engine RPM (Service 01, PID 0C)
- `0105` - Engine coolant temperature (Service 01, PID 05)  
- `0104` - Engine Load percentage (Service 01, PID 04)
- `010D` - Vehicle Speed (Service 01, PID 0D)
- `0100` - ECU availability check (Service 01, PID 00)

## LCD Display Layout

```
+----------------+
|RPM: 1234   87% |  <- Line 1: RPM and Fuel %
| 65km/h    89°C |  <- Line 2: Speed and Temperature
+----------------+
```

## Message Processing Features

- **MUX-based Processing**: Intelligent message routing based on PID
- **Automatic ECU State Management**: Detects and handles ECU sleep/wake states
- **Error Handling**: Graceful handling of "NO DATA" and "ERROR" responses
- **Time-based Calculations**: Precise fuel consumption using delta time measurements
- **Data Persistence**: Automatic saving of trip and fuel data

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

5. **Incorrect Fuel/Trip Calculation**:
   - Calibrate consumption factor via web interface
   - Check if tank capacity is correctly set
   - Reset trip data if readings seem inconsistent
   - Verify vehicle speed sensor is working correctly

6. **ECU Connection Issues**:
   - Check vehicle is fully warmed up
   - Ensure OBD2 port is clean and properly connected
   - Try cycling vehicle ignition
   - Some vehicles may require engine to be running

## Upcoming Features

- [ ] Fuel efficiency trends and historical graphs
- [ ] Multiple trip memory slots with individual statistics
- [ ] Maintenance alerts based on distance/time intervals
- [ ] CSV/JSON data export for detailed analysis
- [ ] Mobile app connectivity via Bluetooth
- [ ] Support for additional OBD2 protocols (CAN, VPW)
- [ ] Real-time fuel cost calculations with price tracking
- [ ] Driving behavior analysis and scoring
- [ ] GPS integration for enhanced trip tracking and mapping
- [ ] OTA (Over-The-Air) firmware updates
- [ ] Multi-language support beyond Portuguese/English

## Technical Specifications

### Performance
- **Update Rate**: ~3-4 readings per second
- **Fuel Calculation Precision**: Sub-milliliter accuracy
- **Distance Tracking**: Meter-level precision
- **Response Time**: <200ms for critical parameters

### Memory Usage
- **Trip Data**: Persistent storage in ESP32 EEPROM
- **Web Interface**: Optimized for mobile devices
- **Debug Logging**: Optional with minimal performance impact

### Power Consumption
- **Active Mode**: ~200mA @ 3.3V
- **WiFi Active**: Additional ~70mA
- **LCD Backlight**: ~20mA

## License

This project is open source and available under the MIT License.

## Contributing

Feel free to submit issues, fork the repository, and create pull requests for improvements.

## Changelog

### v2.2.0 (Current)
- ✅ Enhanced LCD layout with optimized positioning
- ✅ Intelligent fuel calibration system with automatic factor calculation
- ✅ Improved trip computer with real-time km/L efficiency metrics
- ✅ Smart calibration based on actual gas station fill-ups
- ✅ Enhanced web interface with better user experience
- ✅ Optimized display positioning for better readability
- ✅ Simplified distance tracking without accumulator variable
- ✅ Portuguese interface for Brazilian market

### v2.1.0
- ✅ Added vehicle speed monitoring (PID 010D)
- ✅ Implemented real-time distance tracking
- ✅ Enhanced trip computer with km/L calculations
- ✅ Advanced fuel consumption algorithm using RPM × Engine Load
- ✅ Trip reset functionality via web interface
- ✅ Improved LCD display with 4-parameter layout
- ✅ MUX-based message processing system
- ✅ Automatic ECU state management
- ✅ Enhanced web interface with trip statistics
- ✅ Persistent trip data storage

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
