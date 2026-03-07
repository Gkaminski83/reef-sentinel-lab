## English (Primary)

# Reef Sentinel – Build Guide
## Module 4: Sentinel View

Version: 1.0
Date: 2026-03-07
Planned build start: Q2 2026
Estimated build time: 2–3h hardware + 1–2 days UI
Difficulty: Basic
Requires: Module 1 (Sentinel Hub) operational

---

## Purpose

Sentinel View is a local touchscreen dashboard displaying real-time data from the
entire Reef Sentinel Lab system. The Nextion NX8048P050-011C has its own processor:
the ESP32 acts only as a WiFi→UART bridge.

| Feature              | Data source         | Notes                        |
|----------------------|---------------------|------------------------------|
| pH current           | Sentinel Chem (M2)  | Refresh every 15 min         |
| KH last result       | Sentinel Chem (M2)  | Date and time of test        |
| Temperature ×3       | Sentinel Chem (M2)  | Tank / sump / chamber        |
| EC / salinity        | Sentinel Chem (M2)  |                              |
| Ca and Mg (when M3)  | Sentinel Photometer | Q3 2026                      |
| System status        | Sentinel Hub        | Online/offline per module    |
| Start KH test        | → Hub → Chem        | Touchscreen button           |

---

## Required Hardware

- Nextion Intelligent Display NX8048P050-011C (5.0", 800×480, capacitive touch)
- ESP32 DevKit 30-pin (ESP-WROOM-32)
- LM2596 Buck Converter (12V→5V with display)
- 12V/2A power supply (dedicated)
- Dupont F-F jumper wires ×4
- Passive buzzer 5V (85dB, PWM)
- 3D printed enclosure, PETG (2 parts: front + back)
- Neodymium magnets N52, 10mm × 2mm ×4 (mount to cabinet)
- Ball joint M6 (tilt adjustment)
- M2.5 × 6mm screws ×4 + brass heat inserts M2.5 ×4

---

## Safety First

- Never connect USB and LM2596 power simultaneously.
- Tune LM2596 to exactly 5.0V before connecting ESP32 or Nextion.
- Nextion + ESP32 combined draw ~750 mA – ensure LM2596 output is stable.

---

## GPIO Assignment (ESP32 bridge)

| Function     | GPIO       | Interface | Notes              |
|--------------|------------|-----------|--------------------|
| Nextion RX   | GPIO17 (TX2) | UART2   | ESP32 transmits    |
| Nextion TX   | GPIO16 (RX2) | UART2   | ESP32 receives     |
| Buzzer       | GPIO25     | PWM       | Alert tones        |
| Status LED   | GPIO2      | Digital   | Built-in           |
| WiFi         | Built-in   | –         | Connect to SentinelHub AP |

Only 3 GPIO used. ✅

---

## Build Steps

### 1. Power stage
1. Connect 12V supply to LM2596 (IN+, IN–).
2. Adjust potentiometer to 5.0V ±0.1V (confirm with multimeter).
3. Power off before connecting ESP32 or Nextion.
4. Connect: LM2596 OUT+ → ESP32 VIN, OUT– → GND.
5. Nextion and ESP32 share the same 5V output from LM2596.

### 2. Nextion wiring (UART)

| ESP32 pin      | Nextion pin        | Wire colour (suggested) |
|----------------|--------------------|------------------------|
| GPIO17 (TX2)   | RX (connector pin 2) | Yellow               |
| GPIO16 (RX2)   | TX (connector pin 1) | Green                |
| GND            | GND (pin 3)        | Black                  |
| 5V (LM2596)    | 5V (pin 4)         | Red                    |

> Always cross TX/RX: ESP32 TX → Nextion RX, ESP32 RX → Nextion TX.
> This is the most common wiring mistake.

### 3. Buzzer wiring
- Buzzer SIG/+ → GPIO25
- Buzzer GND/– → GND

### 4. Firmware
- Flash sentinel_view.yaml via USB (12V disconnected).
- After flash: connect 12V, verify WiFi connected to SentinelHub in logs.

### 5. Nextion UI upload
1. Design screens in Nextion Editor (nextion.com/download, free).
2. Compile: File → TFT File Output.
3. Copy .tft file to microSD card (FAT32, max 32 GB).
4. Insert SD card into Nextion slot.
5. Power Nextion – auto-flashes UI from SD (~30 s).
6. Remove SD card – Nextion boots with new UI.

### 6. Mechanical assembly
1. Print PETG enclosure (30% infill recommended).
2. Insert M2.5 brass heat inserts with soldering iron.
3. Slide Nextion into front panel, secure with M2.5 screws.
4. Mount ESP32 + LM2596 inside back panel with 3M VHB tape.
5. Route 4 UART wires from ESP32 to Nextion connector.
6. Close enclosure.
7. Attach ball joint M6 to base of enclosure.
8. Epoxy neodymium magnets N52 into base plate (4 magnets for secure hold).

---

## Nextion Display Specification

| Parameter     | Value                         |
|---------------|-------------------------------|
| Diagonal      | 5.0"                          |
| Resolution    | 800 × 480 px                  |
| MCU           | Cortex-M4, 200 MHz (own CPU!) |
| Flash         | 120 MB (UI, graphics, fonts)  |
| Touch         | Capacitive (smartphone-like)  |
| RTC           | Built-in (no NTP needed)      |
| Interface     | UART (4 wires only)           |
| Power         | 5V DC, max 500 mA             |
| Module size   | 132.0 × 82.8 × 12.9 mm        |

---

## Planned UI Screens

| Screen | Name          | Content                                                    |
|--------|---------------|------------------------------------------------------------|
| 0      | Home          | pH, KH, Temp ×3, EC – colour status indicators OK/WARN/ALARM |
| 1      | Charts        | 7-day trend graphs for pH and temperature                  |
| 2      | Tests         | Start KH Test / Start Ca Test buttons + live test status   |
| 3      | Settings      | Test schedule, calibration values, WiFi info               |
| 4      | Alerts        | Last alerts list + buzzer mute button                      |

---

## ESPHome sentinel_view.yaml (skeleton)

```yaml
esphome:
  name: sentinel-view
  friendly_name: Sentinel View

esp32:
  board: esp32dev

wifi:
  ssid: SentinelHub
  password: !secret wifi_ap_password

uart:
  id: nextion_uart
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 9600

display:
  - platform: nextion
    id: nextion_display
    uart_id: nextion_uart
    update_interval: 30s
    lambda: |-
      // update screen components with current values
```

---

## Troubleshooting

### Nextion displays nothing
- Verify 5V on Nextion connector with multimeter (min 4.8V).
- Check whether UI was uploaded (.tft via SD card) – blank screen = no UI.
- Re-flash UI via SD card.

### Data not updating on screen
- Check ESPHome logs: WiFi connected to SentinelHub?
- Verify TX/RX cross-wiring: TX→RX, RX→TX (not TX→TX).
- Confirm baud_rate matches on both sides (default 9600).

### Buzzer not sounding
- Buzzer SIG/+ → GPIO25, GND → GND.
- Passive buzzer requires PWM signal – HIGH/LOW alone won't work.
- Measure GPIO25 during alert: should oscillate (PWM visible on multimeter AC range).

---

## Done Criteria
- LM2596 stable at 5.0V confirmed by multimeter
- Nextion receives 4.9–5.1V on connector
- ESP32 boot OK, WiFi connected to SentinelHub
- UI loaded on Nextion (Home screen visible)
- UART: live data from Hub visible on screen (pH, temp)
- Buzzer responds to test alert from ESPHome
- Enclosure assembled and secure on magnetic base

---

## Related Documents
- `BOM_sentinel_view.md` – full component list with prices
- `BUILD_GUIDE_sentinel_connector.md` – Module 5 build guide
- `firmware/sentinel_view.yaml` *(to be created)*

---

## Polish (PL)

# Reef Sentinel – BUILD GUIDE
## Module 4: Sentinel View (Panel dotykowy)

> **Version:** 1.0
> **Data:** 2026-03-07
> **Planowany start budowy:** Q2 2026
> **Czas montażu:** 2–3h hardware + 1–2 dni UI w Nextion Editor
> **Poziom trudności:** ★★☆☆☆ Podstawowy
> **Wymaga:** Modułu 1 (Sentinel Hub) działającego

---

## ZANIM ZACZNIESZ

### Co ten moduł robi

Sentinel View to lokalny panel dotykowy – wyświetla dane z całego systemu Reef Sentinel Lab
w czasie rzeczywistym. Nextion NX8048P050-011C ma własny procesor Cortex-M4 200MHz i 120MB
flash na grafikę – ESP32 pełni tu tylko rolę WiFi→UART bridge. Połączenie: 4 przewody.

### Dlaczego Nextion zamiast ESP32-S3 + IPS LCD?

| | Nextion NX8048P050 | ESP32-S3 + IPS LCD |
|--|-------------------|--------------------|
| Firmware UI | Nextion Editor (drag & drop) | LVGL + własny kod |
| Czas wdrożenia UI | 3–5 dni | 2–4 tygodnie |
| Połączenie | 4 przewody UART | 30+ przewodów SPI |
| Jakość efektu | Profesjonalna od razu | Zależy od umiejętności |
| Ryzyko techniczne | Niskie | Wysokie (SPI timing, RAM) |
| Koszt | ~295 zł | ~148 zł |

---

## ⚠️ ZASADY BEZPIECZEŃSTWA

- Nigdy nie podłączaj ESP32 do USB i LM2596 jednocześnie
- Ustaw LM2596 na 5.0V multimetrem ZANIM podłączysz ESP32 lub Nextion
- Nextion + ESP32 pobierają łącznie ~750 mA – LM2596 musi być stabilny

---

## LISTA KOMPONENTÓW (~375 zł)

| Komponent | Model/Spec | Ilość | Cena | Gdzie kupić |
|-----------|-----------|-------|------|-------------|
| Nextion Intelligent Display | NX8048P050-011C, 5.0", 800×480 | 1 | ~295 zł | elty.pl (Kraków) |
| ESP32 DevKit 30-pin | ESP-WROOM-32 | 1 | ~25 zł | Botland / Kamami |
| LM2596 Buck Converter | 12V→5V z wyświetlaczem | 1 | ~8 zł | Botland / AliExpress |
| Zasilacz 12V/2A | Dedykowany dla View | 1 | ~20 zł | Botland / Allegro |
| Przewody Dupont F-F 20cm | | 4 szt. | ~0.40 zł | Botland |
| Buzzer pasywny 5V | 85dB, sterowany PWM | 1 | ~3 zł | Botland / AliExpress |
| Obudowa PETG 3D | 2 części: przód + tył | 1 kpl | ~15 zł | Własna drukarka / Printuj3D.pl |
| Magnesy neodymowe N52 | 10mm × 2mm | 4 szt. | ~2 zł | AliExpress / Allegro |
| Przegub kulowy M6 | Regulowany kąt widzenia | 1 | ~8 zł | AliExpress |
| Śruby M2.5 × 6mm | + wkładki gwintowane mosiężne | 4+4 | ~2 zł | AliExpress / lokalny |

### Specyfikacja Nextion NX8048P050-011C

| Parametr | Wartość |
|---------|---------|
| Przekątna | 5.0" |
| Rozdzielczość | 800 × 480 px |
| MCU | Cortex-M4, 200 MHz (własny procesor!) |
| Flash | 120 MB (UI, grafika, animacje) |
| Touch | Pojemnościowy (jak smartfon) |
| RTC | Wbudowany (data i czas bez NTP) |
| Interfejs z ESP32 | UART – tylko 4 przewody |
| Zasilanie | 5V DC, max 500 mA |
| Wymiary modułu | 132.0 × 82.8 × 12.9 mm |

---

## MAPOWANIE GPIO (ESP32 – bridge)

| Funkcja | GPIO | Interfejs | Uwagi |
|---------|------|-----------|-------|
| Nextion RX | GPIO17 (TX2) | UART2 | ESP32 nadaje |
| Nextion TX | GPIO16 (RX2) | UART2 | ESP32 odbiera |
| Buzzer | GPIO25 | PWM | Alarmy dźwiękowe |
| Status LED | GPIO2 (built-in) | Digital | Wbudowana |
| WiFi | Wbudowane | – | Łączy z AP SentinelHub |

**GPIO użytych: 3 ✅**

---

## ETAP 1 – ZASILANIE

1. Podłącz zasilacz 12V do LM2596 (IN+, IN–) – **ESP32 i Nextion jeszcze niepodłączone**
2. Multimetr: zakres DC V 20, czarna sonda COM, czerwona V/Ω
3. Kręć potencjometrem LM2596 do 5.0V ±0.1V
4. Wyłącz zasilacz
5. Podłącz ESP32: LM2596 OUT+ → VIN, OUT– → GND
6. Nextion: 5V i GND ze wspólnego wyjścia LM2596 (patrz etap 2)
7. Włącz zasilacz – sprawdź czy ESP32 świeci LED

> ⚠️ Poniżej 4.8V Nextion może się nie inicjalizować lub wyświetlać artefakty.

---

## ETAP 2 – PODŁĄCZENIE NEXTION (UART)

```
ESP32                        Nextion
GPIO17 (TX2) ──────────────► RX  (pin 2 na złączu)   [żółty]
GPIO16 (RX2) ◄────────────── TX  (pin 1 na złączu)   [zielony]
GND          ──────────────► GND (pin 3)              [czarny]
5V (LM2596)  ──────────────► 5V  (pin 4)              [czerwony]
```

> ⚠️ TX/RX ZAWSZE krzyżujemy: TX ESP32 → RX Nextion, RX ESP32 → TX Nextion.
> To najczęstszy błąd przy UART – podłączenie TX→TX = brak komunikacji.

### Buzzer

- Buzzer SIG/+ → GPIO25
- Buzzer GND/– → GND

---

## ETAP 3 – FIRMWARE ESP32

### sentinel_view.yaml (szkielet)

```yaml
esphome:
  name: sentinel-view
  friendly_name: Sentinel View

esp32:
  board: esp32dev

wifi:
  ssid: SentinelHub
  password: !secret wifi_ap_password
  ap:
    ssid: SentinelView-fallback

uart:
  id: nextion_uart
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 9600

display:
  - platform: nextion
    id: nextion_display
    uart_id: nextion_uart
    update_interval: 30s
    lambda: |-
      // aktualizacja komponentów ekranu bieżącymi wartościami
      // np. id(nextion_display).set_component_text("ph_val", "8.15");
```

### Kroki wgrywania

1. Odłącz zasilacz 12V
2. Podłącz ESP32 do komputera kablem USB
3. ESPHome Dashboard: Install → Plug into this computer
4. Jeśli błąd "Failed to initialize" – przytrzymaj BOOT na ESP32, kliknij RETRY, puść BOOT po 2 s
5. Po wgraniu firmware odłącz USB
6. Podłącz zasilacz 12V – sprawdź w logach: `WiFi connected to SentinelHub`

---

## ETAP 4 – UI W NEXTION EDITOR

### Instalacja Nextion Editor

1. Pobierz ze strony nextion.com/download (bezpłatny, Windows)
2. Zainstaluj i uruchom
3. New Project → wybrać model: NX8048P050 → rozdzielczość 800×480

### Planowane ekrany

| Ekran | Nazwa | Zawartość |
|-------|-------|-----------|
| 0 | Home | pH, KH, Temp ×3, EC – kolorowe wskaźniki OK/WARN/ALARM |
| 1 | Charts | Wykresy trendów 7-dniowych pH i temperatury |
| 2 | Tests | Przyciski Start KH Test / Ca Test + status testu na żywo |
| 3 | Settings | Harmonogram testów, kalibracja, info WiFi |
| 4 | Alerts | Lista alertów + wyciszenie buzzera |

### Wgranie UI na Nextion (przez kartę SD)

1. Nextion Editor: File → TFT File Output → wybierz folder
2. Skopiuj plik `.tft` na kartę microSD (FAT32, max 32 GB)
3. Włóż kartę SD do gniazda w module Nextion
4. Zasilaj Nextion – automatycznie wgra UI z karty (~30 s)
5. Wyjmij kartę SD – Nextion uruchomi się z nowym UI

> Alternatywnie: użyj Nextion Upload Tool do wgrania przez UART (bez karty SD,
> szybciej przy kolejnych iteracjach UI).

---

## ETAP 5 – MONTAŻ MECHANICZNY

### Obudowa 3D (PETG)

Wymagania projektowe obudowy:
- Otwór wyświetlacza: 122.0 × 72.6 mm (dokładne wymiary z datasheetu Nextion)
- Moduł Nextion całkowity: 132.0 × 82.8 × 12.9 mm
- Otwór kablowy: min. 8mm Ø (dolna ścianka)
- Grubość ścianek: min. 2 mm
- Otwory na wkładki gwintowane M2.5 (×4, w rogach ramy Nextion)

### Kolejność montażu

1. Wydrukuj obudowę PETG (30% infill, bez supportów przy dobrej orientacji)
2. Wkładki gwintowane M2.5: wciśnij gorącą lutownicą w otwory obudowy
3. Nextion: wsadź od frontu w otwór, przykręć 4× śruby M2.5
4. ESP32 + LM2596: zamocuj taśmą VHB 3M wewnątrz tylnej części obudowy
5. Poprowadź 4 przewody UART (GPIO16/17/GND/5V) do złącza Nextion
6. Zamknij obudowę (śruby M2.5 w wkładki)
7. Przegub kulowy M6: przykręć do dołu obudowy
8. Magnesy N52: wklej epoksydem do podstawki (4 szt. dla pewnego chwytu)

> ⚠️ Przed zamknięciem obudowy przetestuj wyświetlacz i komunikację UART.
> Po skręceniu obudowy dostęp do portu USB ESP32 może być utrudniony.

---

## TROUBLESHOOTING

### Nextion nie wyświetla nic (czarny ekran)

```
Przyczyna 1: Brak wgranego UI
  → Bez pliku .tft ekran jest czarny
  → Wgraj UI przez kartę SD (patrz Etap 4)

Przyczyna 2: Za niskie napięcie zasilania
  → Zmierz multimetrem napięcie na złączu Nextion
  → Minimum 4.8V – poniżej ekran może się nie inicjalizować

Przyczyna 3: Uszkodzony plik .tft
  → Spróbuj ponownie skopiować plik na kartę SD
  → Sprawdź czy karta SD jest FAT32 (nie exFAT, nie NTFS)
```

### Dane nie aktualizują się na ekranie

```
Przyczyna 1: WiFi nie połączony z SentinelHub
  → Sprawdź logi ESPHome: szuka "WiFi connected to SentinelHub"
  → Sprawdź czy Sentinel Hub (Moduł 1) działa i AP SentinelHub jest aktywne

Przyczyna 2: TX/RX nie skrzyżowane
  → Najczęstszy błąd! TX ESP32 MUSI iść do RX Nextion (i odwrotnie)
  → Zamień GPIO16 i GPIO17 i przetestuj ponownie

Przyczyna 3: Niezgodny baud_rate
  → ESP32 i Nextion MUSZĄ mieć identyczny baud_rate (domyślnie 9600)
  → Sprawdź ustawienie w Nextion Editor: Page0 → Preinitialize Event
```

### Buzzer nie działa

```
Przyczyna 1: Zła polaryzacja
  → SIG/+ → GPIO25, GND/– → GND

Przyczyna 2: Buzzer pasywny wymaga PWM
  → Prosty sygnał HIGH/LOW nie zadziała na buzzerze pasywnym
  → Sprawdź czy w YAML używasz platform: ledc (PWM) a nie gpio

Przyczyna 3: Za niskie napięcie PWM
  → ESP32 generuje 3.3V – większość buzzerów 5V działa, ale sprawdź datasheet
```

---

## FINALNA CHECKLIST

```
HARDWARE:
☐ LM2596: 5.0V ±0.1V zmierzone multimetrem
☐ Nextion: 4.9–5.1V na złączu potwierdzone multimetrem
☐ ESP32: boot OK (LED miga)
☐ WiFi: połączony z SentinelHub (IP w logach)
☐ UI wgrane na Nextion – Home screen widoczny
☐ UART: dane z Hub widoczne na ekranie (pH, temp)
☐ Buzzer: reaguje na testowy alert z ESPHome
☐ Obudowa: Nextion przykręcony, tylna pokrywa zamknięta
☐ Podstawka: przegub M6 + magnesy N52 trzymają mocno

STATUS: ✅ Sentinel View gotowy!
        Następny krok: BUILD_GUIDE_sentinel_connector.md
```

---

## NASTĘPNE KROKI

Po ukończeniu Sentinel View:

1. **Sentinel Connector (Module 5)** – opcjonalny, dla właścicieli Apex/GHL/Hydros
2. **Rozbudowa UI** – dodaj ekran Ca/Mg gdy Moduł 3 gotowy (Q3 2026)
3. **OTA aktualizacje** – firmware aktualizowany bezprzewodowo przez ESPHome

---

## POWIĄZANE DOKUMENTY

- `BOM_sentinel_view.md` – pełna lista komponentów z cenami
- `BUILD_GUIDE_sentinel_connector.md` – Instrukcja montażu Modułu 5
- `firmware/sentinel_view.yaml` *(do stworzenia)*

---

*Reef Sentinel Lab – Open-source aquarium controller*
*reef-sentinel.com | github.com/reef-sentinel*
*Ostatnia aktualizacja: 2026-03-07*
