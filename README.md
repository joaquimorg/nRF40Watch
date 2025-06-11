# nRF40Watch Firmware

Basic firmware for a custom nRF52840-based smartwatch using:

- **LVGL 9.3** for GUI rendering on ST7789V LCD
- **BLE (SoftDevice S140)** for communication
- **CST816S** touch controller via I2C

---

## 📁 Project Structure
```
ProjectRoot/
├── main/            # Application entry and GUI
├── services/        # BLE and time services
├── drivers/         # LCD, touch, and backlight drivers
├── config/          # sdk_config.h
├── lvgl/            # LVGL 9.3 library sources
├── nrf5_sdk/        # Copied Nordic SDK files
├── Makefile         # Cross-platform build system
```

---

## 🚀 Features
- Display current time on boot
- Touchscreen interface using CST816S
- BLE advertising (visible as "nRF40Watch")
- Settings page support scaffolded

---

## 🛠 Build Instructions
### Prerequisites
- GCC ARM Embedded toolchain (`arm-none-eabi-gcc`)
- GNU Make
- Nordic SDK 17.1.0 (place required files in `nrf5_sdk/`)
- LVGL 9.3 source files copied into `lvgl/`

### Linux/macOS
```bash
make
```

### Windows (CMD/Git Bash/MSYS2)
```cmd
make
```

This builds:
- `./_build/nrf40watch_fw.hex` for flashing

---

## 📦 Flashing
Use nRF Connect Programmer or `nrfjprog`:
```bash
nrfjprog --eraseall
nrfjprog --program _build/nrf40watch_fw.hex --verify --reset
```

> Ensure you also flash `s140_nrf52_7.2.0_softdevice.hex` if needed.

---

## 📌 License
MIT or compatible. You are free to modify and use for hardware based on nRF52840.

---

