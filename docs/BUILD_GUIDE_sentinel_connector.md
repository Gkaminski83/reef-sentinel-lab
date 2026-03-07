## English (Primary)

# Reef Sentinel – Build Guide
## Module 5: Sentinel Connector

Version: 0.1 (concept)
Date: 2026-03-07
Planned build start: TBD (after Q4 2026)
Estimated build time: 2–3h hardware + configuration
Difficulty: Intermediate
Requires: Nothing – this module is fully standalone

---

## Purpose

Sentinel Connector is the only module in Reef Sentinel Lab that operates standalone –
it does not require Sentinel Hub or any other module. It bridges an existing third-party
aquarium controller (Neptune Apex, GHL ProfiLux, Hydros) with reef-sentinel.com and
Home Assistant.

### Use cases

| Scenario | What Connector does |
|----------|---------------------|
| You have Apex and want KH auto without replacing it | Polls Apex LAN API, forwards data to reef-sentinel.com. Add Sentinel Chem/Monitor separately if desired. |
| You have GHL ProfiLux and want HA dashboard | Translates GHL data to ESPHome API format – HA sees it as native entities. |
| You have Hydros and want AI insights | Polls Hydros cloud API, relays to reef-sentinel.com every 15 min. |
| You want to migrate from a closed system gradually | Connector runs alongside your existing controller – add Reef Sentinel modules over time. |

> Sentinel Connector does not replace your existing controller.
> It adds cloud analytics, HA integration, and optional chemistry automation alongside it.

---

## Required Hardware

- ESP32 DevKit 30-pin (ESP-WROOM-32)
- LM2596 Buck Converter (12V→5V)
- 12V/1A power supply (dedicated – no pumps, 1A sufficient)
- Small enclosure ABS/3D or DIN-rail mount
- A few Dupont jumper wires (power only)

**Estimated cost: ~60 zł**

This is the cheapest module in the entire system.
If you already have an ESP32 and LM2596 from another project, cost is ~15 zł (PSU only).

---

## Safety First

- Never connect USB and LM2596 power simultaneously.
- Tune LM2596 to exactly 5.0V before connecting ESP32.
- Store API keys only in secrets.yaml – never in the main YAML file.
- Do not commit secrets.yaml to any public git repository.

---

## Supported Controllers

| Controller | Protocol | Support status | Notes |
|------------|----------|---------------|-------|
| Neptune Apex | REST API (LAN) | 🟡 Planned v1 | Documented LAN API |
| GHL ProfiLux | GHL API / Modbus | 🟡 Planned v1 | API available on request |
| Hydros (CoralVue) | REST API (cloud) | 🟡 Planned v1 | Requires Hydros API key |
| Inkbird / STC-1000 | No API – 1-Wire poll | 🟢 Planned v2 | Direct sensor read |
| Raspberry Pi | MQTT / HTTP | 🟢 Planned v2 | Via local MQTT broker |
| Generic | MQTT or HTTP poll | 🟠 Config required | Via config.yaml |

> STATUS: Sentinel Connector is at concept stage (TBD).
> Sections below describe the planned architecture – subject to change before v1.0.

---

## Architecture

```
  External controller (Apex / GHL / Hydros)
            │
       LAN / Internet
            │
  ┌─────────┴──────────────────┐
  │    SENTINEL CONNECTOR      │  ← ESP32 standalone
  │    IP: DHCP from router    │
  │                            │
  │  Poll every 60s:           │
  │  GET /api/data             │
  │  → parse JSON              │
  │  → normalise to RS format  │
  │                            │
  │  Output:                   │
  │  → reef-sentinel.com       │  every 15 min (HTTPS)
  │  → ESPHome API             │  on-demand (HA)
  │  → MQTT (optional)         │  if broker present
  └────────────────────────────┘
```

### Normalised data format

| Parameter   | JSON key       | Unit  | Marine range |
|-------------|---------------|-------|--------------|
| pH          | ph             | pH    | 7.8 – 8.5    |
| Temperature | temp_c         | °C    | 24 – 27      |
| Salinity    | salinity_ppt   | ppt   | 34 – 36      |
| ORP         | orp_mv         | mV    | 300 – 450    |
| Water level | water_level_pct| %     | 0 – 100      |
| Pump status | pump_status    | bool  | 0 / 1        |

---

## Build Steps

### 1. Power stage
1. Connect 12V supply to LM2596 (IN+, IN–).
2. Adjust potentiometer to 5.0V ±0.1V (confirm with multimeter).
3. Power off before connecting ESP32.
4. Connect: LM2596 OUT+ → ESP32 VIN, OUT– → GND.

### 2. Firmware
1. Flash sentinel_connector.yaml via USB (12V disconnected).
2. Configure secrets.yaml with controller API key and reef_sentinel_api_key.
3. After flash: connect 12V, verify WiFi connected to home network in logs.
4. Verify polling: controller data visible in ESPHome logs.
5. Verify relay: data appears on reef-sentinel.com dashboard.

### 3. Enclosure
- Mount ESP32 + LM2596 in small ABS enclosure or on DIN rail.
- No sensors or pumps – compact install near network switch or router.

---

## Configuration – Neptune Apex

### Requirements
- Apex on LAN, IP known (e.g. 192.168.1.100 or apex.local)
- Apex API key: Apex panel → System → Network → API Key

### sentinel_connector.yaml excerpt

```yaml
http_request:
  timeout: 10s

interval:
  - interval: 60s
    then:
      - http_request.get:
          url: http://192.168.1.100/cgi-bin/status.json
          headers:
            Authorization: !secret apex_api_key
          on_response:
            - lambda: |-
                // parse Apex JSON response
                // extract ph, temp, salinity
                // store in global variables
```

### secrets.yaml
```yaml
apex_api_key: "Bearer YOUR-APEX-KEY"
reef_sentinel_api_key: "rs_placeholder"
tank_id: "tank_001"
```

---

## Configuration – GHL ProfiLux

### Requirements
- ProfiLux 4 or newer with GHL network module (GHL IF-USB-Wifi or LAN)
- GHL Connect active (built-in HTTP server)
- ProfiLux IP known on LAN

### GHL API endpoint (simplified)
```
GET http://192.168.1.101/api/v1/profilux/read
Response: { "pH": 8.15, "Temp1": 25.4, "S.value": 35.2 }
```

> GHL API is less documented than Apex.
> Community examples available at reef2reef.com GHL section.

---

## Configuration – Hydros (CoralVue)

### Requirements
- Account on hydrosapp.com
- API key: hydrosapp.com → Settings → Developer API
- Hydros device connected to WiFi and syncing with cloud

### Important note
Hydros has no local API – data available through cloud only. Sentinel Connector
will poll the Hydros cloud API (requires internet access), not LAN:

```
GET https://api.hydrosapp.com/v2/devices/{device_id}/readings
Authorization: Bearer YOUR-HYDROS-KEY
```

> This is exactly the vendor lock-in Reef Sentinel Lab aims to avoid.
> Treat Connector as a transitional tool while migrating to open hardware.

---

## Security & Privacy

- Store all API keys only in secrets.yaml – never in the main YAML.
- Do not push secrets.yaml to any public repository.
- Rotate API keys every 3–6 months.
- Use separate keys for dev and production.
- All data sent to reef-sentinel.com over HTTPS.
- LAN connections to Apex/GHL are unencrypted – be aware of local network risks.

---

## Roadmap

| Version | Target  | Content |
|---------|---------|---------|
| v0.1    | TBD     | Concept and architecture (current stage) |
| v0.5    | Q4 2026 | Neptune Apex integration (first tests) |
| v0.7    | Q1 2027 | GHL ProfiLux integration |
| v0.9    | Q1 2027 | Hydros integration + beta testing |
| v1.0    | Q2 2027 | Stable release – all 3 controllers, ESPHome component |
| v1.x    | 2027+   | MQTT generic, Raspberry Pi, custom HTTP polling |

---

## Done Criteria
- LM2596 stable at 5.0V confirmed by multimeter
- ESP32 boot OK, WiFi connected to home network (not SentinelHub – Connector is standalone)
- secrets.yaml: controller API key and reef_sentinel_api_key filled in
- Polling: controller data visible in ESPHome logs (pH, temp etc.)
- Relay: data appears on reef-sentinel.com dashboard
- HA integration (optional): entities updating in Home Assistant

---

## Related Documents
- `BUILD_GUIDE_sentinel_hub.md` – Module 1 (if adding full Reef Sentinel Lab)
- `firmware/sentinel_connector.yaml` *(to be created)*

---

## Polish (PL)

# Reef Sentinel – BUILD GUIDE
## Module 5: Sentinel Connector (Bridge dla zewnętrznych kontrolerów)

> **Version:** 0.1 (koncepcja)
> **Data:** 2026-03-07
> **Planowany start budowy:** TBD (po Q4 2026)
> **Czas montażu:** 2–3h hardware + konfiguracja
> **Poziom trudności:** ★★★☆☆ Średni
> **Wymaga:** NIC – ten moduł działa w pełni samodzielnie

---

## ZANIM ZACZNIESZ

### Co ten moduł robi

Sentinel Connector to jedyny moduł Reef Sentinel Lab który działa **samodzielnie** –
nie wymaga Sentinel Hub ani pozostałych modułów. Jego rola to integracja istniejącego
kontrolera akwarium (Neptune Apex, GHL ProfiLux, Hydros i innych) z platformą
reef-sentinel.com i Home Assistant.

### Przypadki użycia

| Scenariusz | Co Connector robi |
|------------|-------------------|
| Masz Apex i nie chcesz go wyrzucać | Odpytuje API Apex przez LAN, przekazuje dane do reef-sentinel.com. Możesz dokupić Sentinel Chem/Monitor jako uzupełnienie. |
| Masz GHL ProfiLux i chcesz dashboard w HA | Tłumaczy dane GHL na format ESPHome API – HA widzi je jak natywne encje. |
| Masz Hydros i chcesz AI insights | Odpytuje cloud API Hydros, przesyła do reef-sentinel.com co 15 min. |
| Chcesz migrować stopniowo | Connector działa obok starego systemu – dodajesz moduły RS kiedy chcesz. |

> Sentinel Connector nie zastępuje Twojego obecnego kontrolera. Uzupełnia go.

---

## ⚠️ ZASADY BEZPIECZEŃSTWA

- Nigdy nie podłączaj ESP32 do USB i LM2596 jednocześnie
- Ustaw LM2596 na 5.0V multimetrem ZANIM podłączysz ESP32
- Klucze API przechowuj WYŁĄCZNIE w secrets.yaml
- Nie commituj secrets.yaml do publicznego repozytorium git

---

## LISTA KOMPONENTÓW (~60 zł)

| Komponent | Spec | Ilość | Cena | Uwagi |
|-----------|------|-------|------|-------|
| ESP32 DevKit 30-pin | ESP-WROOM-32 | 1 | ~25 zł | |
| LM2596 Buck Converter | 12V→5V | 1 | ~8 zł | ⚠️ Ustaw 5.0V multimetrem! |
| Zasilacz 12V/1A | Dedykowany – brak pompek, 1A wystarczy | 1 | ~15 zł | |
| Obudowa ABS lub 3D | Mała – tylko ESP32 + LM2596 | 1 | ~10 zł | Lub na szynę DIN |
| Przewody Dupont | Tylko zasilanie (kilka szt.) | – | ~2 zł | |

**To najtańszy moduł w całym systemie Reef Sentinel Lab.**
Jeśli masz już ESP32 i LM2596 z innego projektu – koszt wynosi ~15 zł (sam zasilacz).

---

## OBSŁUGIWANE KONTROLERY

| Kontroler | Protokół | Status wsparcia | Uwagi |
|-----------|----------|----------------|-------|
| Neptune Apex | REST API (LAN) | 🟡 Planowane v1 | Udokumentowane API LAN |
| GHL ProfiLux | GHL API / Modbus | 🟡 Planowane v1 | API dostępne na żądanie |
| Hydros (CoralVue) | REST API (cloud) | 🟡 Planowane v1 | Wymaga klucza API Hydros |
| Inkbird / STC-1000 | Brak API – polling 1-Wire | 🟢 Planowane v2 | Bezpośredni odczyt czujnika |
| Raspberry Pi | MQTT / HTTP | 🟢 Planowane v2 | Przez broker MQTT |
| Inne (generic) | MQTT lub HTTP poll | 🟠 Wymaga konfiguracji | Przez config.yaml |

> STATUS: Sentinel Connector jest na etapie koncepcji (TBD).
> Poniższe sekcje opisują planowaną architekturę – może ulec zmianie przed v1.0.

---

## ARCHITEKTURA

```
  Kontroler zewnętrzny (Apex / GHL / Hydros)
            │
       LAN lub Internet
            │
  ┌─────────┴──────────────────────┐
  │    SENTINEL CONNECTOR          │  ← ESP32 standalone
  │    IP: DHCP z routera          │
  │                                │
  │  Polling co 60s:               │
  │  GET /api/data                 │
  │  → parsowanie JSON             │
  │  → normalizacja do formatu RS  │
  │                                │
  │  Wyjście:                      │
  │  → reef-sentinel.com          │  co 15 min (HTTPS)
  │  → ESPHome API                │  na żądanie (HA)
  │  → MQTT (opcja)               │  jeśli broker dostępny
  └────────────────────────────────┘
```

### Normalizacja danych

Niezależnie od źródła (Apex, GHL, Hydros) Connector tłumaczy wszystko na
ujednolicony format Reef Sentinel:

| Parametr | Klucz JSON | Jednostka | Zakres akwarium morskiego |
|----------|-----------|-----------|--------------------------|
| pH | ph | pH | 7.8 – 8.5 |
| Temperatura | temp_c | °C | 24 – 27 |
| Zasolenie | salinity_ppt | ppt | 34 – 36 |
| ORP | orp_mv | mV | 300 – 450 |
| Poziom wody | water_level_pct | % | 0 – 100 |
| Status pompy | pump_status | bool | 0 / 1 |

---

## ETAP 1 – ZASILANIE

1. Podłącz zasilacz 12V do LM2596 (IN+, IN–) – **ESP32 jeszcze niepodłączony**
2. Multimetr: zakres DC V 20, kręć potencjometrem do 5.0V ±0.1V
3. Wyłącz zasilacz
4. Podłącz ESP32: LM2596 OUT+ → VIN, OUT– → GND
5. Włącz zasilacz – sprawdź czy ESP32 świeci LED

---

## ETAP 2 – FIRMWARE

### sentinel_connector.yaml (szkielet)

```yaml
esphome:
  name: sentinel-connector
  friendly_name: Sentinel Connector

esp32:
  board: esp32dev

wifi:
  ssid: !secret wifi_home_ssid       # sieć DOMOWA (nie SentinelHub!)
  password: !secret wifi_home_password
  ap:
    ssid: SentinelConnector-setup

http_request:
  timeout: 10s

interval:
  - interval: 60s
    then:
      - http_request.get:
          url: !secret controller_api_url
          headers:
            Authorization: !secret controller_api_key
          on_response:
            - lambda: |-
                // parsowanie odpowiedzi kontrolera
                // normalizacja do formatu RS
                // zapis do zmiennych globalnych
  - interval: 900s   # 15 min
    then:
      - http_request.post:
          url: https://reef-sentinel.com/api/integrations/webhook
          headers:
            X-API-Key: !secret reef_sentinel_api_key
          json:
            tank_id: !secret tank_id
            # ...dane z kontrolera
```

> ⚠️ Connector łączy się z siecią DOMOWĄ (nie z AP SentinelHub).
> Potrzebuje dostępu do LAN gdzie jest kontroler (Apex/GHL) lub do Internetu (Hydros).

### secrets.yaml

```yaml
wifi_home_ssid: "NazwaTwojejSieci"
wifi_home_password: "HasloTwojejSieci"
controller_api_url: "http://192.168.1.100/cgi-bin/status.json"
controller_api_key: "Bearer TWOJ-KLUCZ-API"
reef_sentinel_api_key: "rs_placeholder"
tank_id: "tank_001"
```

### Kroki wgrywania

1. Odłącz zasilacz 12V
2. Podłącz ESP32 do komputera kablem USB
3. ESPHome Dashboard: Install → Plug into this computer
4. Jeśli błąd "Failed to initialize" – przytrzymaj BOOT, kliknij RETRY
5. Po wgraniu odłącz USB, podłącz zasilacz 12V
6. Sprawdź w logach: połączenie z siecią domową, pierwsze polling kontrolera

---

## KONFIGURACJA – NEPTUNE APEX

### Wymagania po stronie Apex

- Apex podłączony do sieci LAN (Ethernet lub WiFi)
- IP lub hostname Apex (np. 192.168.1.100 lub apex.local)
- Klucz API: Panel Apex → System → Network → API Key

### Format odpowiedzi Apex API

```json
{
  "status": [
    { "name": "pH_1",    "value": "8.15", "type": "pH" },
    { "name": "Temp_1",  "value": "25.4", "type": "Temp" },
    { "name": "Salt_1",  "value": "35.1", "type": "Cond" }
  ]
}
```

### secrets.yaml – dodaj

```yaml
controller_api_url: "http://192.168.1.100/cgi-bin/status.json"
controller_api_key: "Bearer TWOJ-KLUCZ-APEX"
```

---

## KONFIGURACJA – GHL PROFILUX

### Wymagania po stronie GHL

- ProfiLux 4 lub nowszy z modułem sieciowym GHL IF-USB-Wifi lub LAN
- GHL Connect aktywny (wbudowany serwer HTTP)
- Znany IP ProfiLux w sieci LAN

### Endpoint GHL API (uproszczony)

```
GET http://192.168.1.101/api/v1/profilux/read
Odpowiedź: { "pH": 8.15, "Temp1": 25.4, "S.value": 35.2 }
```

> GHL API jest słabiej udokumentowane niż Apex.
> Przykłady społeczności: reef2reef.com, sekcja GHL ProfiLux.
> Może wymagać przechwycenia pakietów Wireshark dla starszych firmware GHL.

---

## KONFIGURACJA – HYDROS (CORALVUE)

### Wymagania po stronie Hydros

- Konto na hydrosapp.com
- Klucz API: hydrosapp.com → Settings → Developer API
- Urządzenie Hydros podłączone do WiFi i synchronizujące z chmurą

### Ważna uwaga architektoniczna

Hydros nie ma lokalnego API – dane dostępne WYŁĄCZNIE przez cloud.
Sentinel Connector musi mieć dostęp do Internetu aby odpytać Hydros:

```
GET https://api.hydrosapp.com/v2/devices/{device_id}/readings
Authorization: Bearer TWOJ-KLUCZ-HYDROS
```

> ⚠️ To jest dokładnie ten vendor lock-in który Reef Sentinel Lab stara się eliminować.
> Jeśli serwery Hydros są niedostępne – Connector nie pobierze danych.
> Traktuj Connector jako narzędzie przejściowe przy migracji do otwartego hardware.

---

## BEZPIECZEŃSTWO I PRYWATNOŚĆ

### Klucze API

- Przechowuj WYŁĄCZNIE w secrets.yaml – nigdy w głównym YAML
- Nie commituj secrets.yaml do żadnego publicznego repozytorium
- Rotuj klucze API co 3–6 miesięcy
- Używaj oddzielnych kluczy dla środowiska dev i production

### Bezpieczeństwo sieci

- Connector komunikuje się przez WiFi 2.4GHz
- Dane do reef-sentinel.com przez HTTPS (szyfrowane)
- Połączenia LAN z Apex/GHL są nieszyfrowane – miej świadomość ryzyka w sieci lokalnej
- Jeśli Apex/GHL oferuje HTTPS – użyj go

---

## ROADMAP

| Wersja | Termin | Zakres |
|--------|--------|--------|
| v0.1 | TBD | Koncepcja i architektura (obecny etap) |
| v0.5 | Q4 2026 | Neptune Apex – pierwsze testy |
| v0.7 | Q1 2027 | GHL ProfiLux integration |
| v0.9 | Q1 2027 | Hydros integration + beta testy |
| v1.0 | Q2 2027 | Stable release – wszystkie 3 kontrolery |
| v1.x | 2027+ | MQTT generic, Raspberry Pi, custom HTTP polling |

> Jeśli używasz Apex lub GHL i chcesz testować wczesne wersje Connectora –
> zgłoś się na forum.reef-sentinel.com lub przez GitHub Issues.
> Szukamy beta testerów z różnymi wersjami kontrolerów i firmware.

---

## FINALNA CHECKLIST

```
HARDWARE:
☐ LM2596: 5.0V ±0.1V zmierzone multimetrem
☐ ESP32: boot OK (LED miga)
☐ WiFi: połączony z siecią DOMOWĄ (nie SentinelHub!)
☐ Obudowa: ESP32 i LM2596 zamocowane i zabezpieczone

OPROGRAMOWANIE:
☐ sentinel_connector.yaml wgrany bez błędów
☐ secrets.yaml: klucz API kontrolera uzupełniony
☐ secrets.yaml: reef_sentinel_api_key uzupełniony
☐ Polling: dane z kontrolera widoczne w logach ESPHome
☐ Relay: dane pojawiają się na reef-sentinel.com
☐ HA integration (opcja): encje aktualizują się w Home Assistant

STATUS: ✅ Sentinel Connector gotowy!
        Moduł 5 = ostatni moduł. System Reef Sentinel Lab kompletny!
```

---

## NASTĘPNE KROKI

Sentinel Connector to ostatni moduł Reef Sentinel Lab. Po ukończeniu:

1. **Pełny system działa** – wszystkie moduły zsynchronizowane z reef-sentinel.com
2. **AI Insights** – reef-sentinel.com analizuje trendy i sugeruje korekty dawkowania
3. **Społeczność** – podziel się konfiguracją na forum.reef-sentinel.com
4. **PCB** – gdy projekt przejdzie do fazy PCB (Q2 2026) – zamów profesjonalną płytkę

---

## POWIĄZANE DOKUMENTY

- `BUILD_GUIDE_sentinel_hub.md` – Moduł 1 (jeśli dodajesz pełny Reef Sentinel Lab)
- `firmware/sentinel_connector.yaml` *(do stworzenia)*

---

*Reef Sentinel Lab – Open-source aquarium controller*
*reef-sentinel.com | github.com/reef-sentinel*
*Ostatnia aktualizacja: 2026-03-07*
