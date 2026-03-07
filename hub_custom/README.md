# Reef Sentinel Hub - Custom Firmware

This is the active Hub firmware path (ESP32 + PlatformIO + native web dashboard).

## Features

- Native web UI served by ESP32 (`/`)
- Wi-Fi provisioning via WiFiManager captive portal (`SentinelHub`)
- mDNS host: `reef-sentinel.local`
- OLED SSD1306 status output (I2C)
- Chem module integration (data + control + calibration commands)
- Cloud sync to `reef-sentinel.com`
- Offline cloud queue with retry backoff (`1m`, `5m`, `15m`)

## Build

1. `cd hub_custom`
2. `pio run -t upload --upload-port COMx`
3. `pio device monitor --port COMx --baud 115200`

## Main Endpoints

- `GET /` - dashboard
- `GET /api/status`
- `POST /api/module/chem/report`
- `POST /api/module/chem/settings`
- `POST /api/module/chem/command`
- `POST /api/cloud/settings`
- `POST /api/cloud/sync_now`

## OLED

Expected config:

- SDA: `GPIO21`
- SCL: `GPIO22`
- Address: `0x3C`
- Power: `3V3`

## Notes

- `firmware/secrets.yaml` is not used in this firmware path.
- Config is persisted in NVS (`Preferences`).
- Sentinel Photometer endpoint is not added yet (planned next).
