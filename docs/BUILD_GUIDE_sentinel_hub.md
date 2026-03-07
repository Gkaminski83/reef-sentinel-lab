# BUILD GUIDE - Sentinel Hub (Current Firmware Path)

Version: 2026-03-07
Target firmware: `hub_custom` (PlatformIO)

## 1. Scope

This guide covers the current, supported Hub path:

- ESP32 + native web UI (not ESPHome web UI)
- Direct HTTP module integration
- Cloud sync to `reef-sentinel.com`
- OLED status display over I2C

## 2. Hardware

- ESP32 DevKit (ESP32-WROOM)
- LM2596 buck converter (set to 5.0V)
- OLED SSD1306 128x64 I2C (`0x3C`)
- USB data cable (stable, dry, not damaged)

## 3. Wiring

Power:

- LM2596 `OUT+` -> ESP32 `VIN`
- LM2596 `OUT-` -> ESP32 `GND`

OLED:

- ESP32 `GPIO21` -> OLED `SDA`
- ESP32 `GPIO22` -> OLED `SCL`
- ESP32 `3V3` -> OLED `VCC`
- ESP32 `GND` -> OLED `GND`

Important:

- Do not power ESP32 from USB and external `VIN` at the same time.
- Do not connect OLED VCC to 5V on this build path.

## 4. Build and Upload

From repository root:

1. `cd hub_custom`
2. `pio run -t upload --upload-port COMx`
3. `pio device monitor --port COMx --baud 115200`

If firmware size exceeds limit, `huge_app.csv` partition is already configured in `platformio.ini`.

## 5. First Boot and Wi-Fi

Hub boot behavior:

1. Try saved Wi-Fi credentials.
2. If failed, start captive portal AP: `SentinelHub`.
3. After provisioning, Hub available at:
   - `http://reef-sentinel.local`
   - fallback: IP from serial log

## 6. Dashboard and API

Main UI:

- `/` (native custom dashboard)

Status:

- `GET /api/status`

Chem module:

- `POST /api/module/chem/report`
- `POST /api/module/chem/settings`
- `POST /api/module/chem/command`

Cloud:

- `POST /api/cloud/settings`
- `POST /api/cloud/sync_now`

## 7. Cloud Sync Behavior

Hub cloud sync includes:

- periodic payload enqueue by `cloud_sync_interval_min`
- offline queue (FIFO)
- retry policy per item: `+1 min`, `+5 min`, `+15 min` (then repeat 15 min)
- queue status exposed in `/api/status`:
  - `cloud_queue_size`
  - `cloud_next_retry_s`

## 8. Current Integration State

Implemented now:

- Sentinel Chem data + control + calibration flow

Not yet implemented in `hub_custom`:

- Sentinel Photometer ingestion endpoint and cards

## 9. Troubleshooting

No COM port:

- run `pio device list`
- run `Get-PnpDevice -Class Ports`
- if CP210x is `Unknown`, reinstall CP210x driver and reconnect
- replace cable if it was exposed to water

OLED not visible:

- verify I2C address `0x3C`
- verify wiring (`21/22`, `3V3`, `GND`)
- check serial log for `OLED init failed`

mDNS not resolving:

- open via IP first
- ensure same LAN/subnet and mDNS allowed on router/client
