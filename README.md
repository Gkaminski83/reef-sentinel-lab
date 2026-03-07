# Reef Sentinel LAB

Open-source reef aquarium controller with modular ESP32 firmware.

## Current Architecture (March 2026)

- Hub firmware is now in `hub_custom/` (PlatformIO, native web UI).
- Module communication is direct HTTP (no MQTT broker required).
- Home Assistant is optional.
- Cloud sync to `reef-sentinel.com` is supported from Hub.
- Cloud sync has offline queue + retry backoff (`1m`, `5m`, `15m`).

## Module Status

1. Sentinel Hub: active development, buildable now
2. Sentinel Chem: integrated with Hub (reporting + control)
3. Sentinel Photometer: planned integration path, not yet wired in `hub_custom`
4. Sentinel View: planned
5. Sentinel Connector: planned

## Repository Layout

- `hub_custom/` - current Hub firmware (recommended path)
- `firmware/` - ESPHome configurations and compatibility files
- `docs/` - build guides and BOM documentation

## Quick Start (Hub, recommended)

1. `cd hub_custom`
2. `pio run -t upload --upload-port COMx`
3. `pio device monitor --port COMx --baud 115200`
4. Open `http://reef-sentinel.local` (or Hub IP from serial log)

## Networking and Provisioning

- Hub first tries saved Wi-Fi credentials.
- If connect fails, captive portal starts as `SentinelHub`.
- After provisioning, Hub is available via mDNS:
  - `http://reef-sentinel.local`

## Chem Integration (current)

Implemented endpoints in Hub:

- `POST /api/module/chem/report`
- `POST /api/module/chem/settings`
- `POST /api/module/chem/command`
- `GET /api/status`

Current dashboard cards are Chem-based:

- pH
- KH
- Temperature (aquarium/sump/room)
- EC

## Cloud Integration

Hub supports:

- cloud settings update (`/api/cloud/settings`)
- manual sync (`/api/cloud/sync_now`)
- periodic snapshot enqueue
- queued retry delivery when offline

## Documentation Index

- `docs/BUILD_GUIDE_sentinel_hub.md`
- `docs/BUILD_GUIDE_sentinel_chem_monitor.md`
- `docs/BUILD_GUIDE_sentinel_photometer.md`
- `docs/BUILD_GUIDE_sentinel_view.md`
- `docs/BUILD_GUIDE_sentinel_connector.md`
- `docs/WIRING_DIAGRAM.md`
- `hub_custom/README.md`

## Notes

- `firmware/secrets.yaml` is not used by `hub_custom`.
- For `hub_custom`, runtime configuration is stored in ESP32 NVS (Preferences).
