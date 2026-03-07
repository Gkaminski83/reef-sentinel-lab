## English (Primary)

# Reef Sentinel - BOM
## Module 1 (Sentinel Hub) + Module 2 (Sentinel Chem/Monitor)

Version: 1.0
Date: 2026-03-06

---

## Summary

This BOM lists:
- components already owned,
- missing parts,
- purchase priority,
- estimated cost to complete the Phase 1 stack.

---

## Module 1: Sentinel Hub

Functions:
- AP+STA WiFi coordination
- Home Assistant bridging and data aggregation
- OLED status output
- optional SD logging

Main parts:
- ESP32 DevKit
- 12V supply + LM2596
- OLED display
- SD reader + optional microSD
- breadboard and Dupont wiring

Status:
- Module 1 is effectively build-ready.

---

## Module 2: Sentinel Chem/Monitor

Functions:
- KH titration workflow
- pH continuous measurement
- EC/salinity and 3-point temperature monitoring
- pumps and stirrer control

Main parts:
- ESP32 DevKit
- 12V supply + LM2596
- pH kit, EC sensor, DS18B20 probes
- 5x MOSFET channels
- 4x 12V peristaltic pumps
- stirrer motor + magnet/stir bar assembly
- fluid containers and tubing

---

## GPIO Allocation

### Hub
- I2C OLED: GPIO21/22
- SPI SD: GPIO5/18/19/23

### Chem/Monitor
- pH analog: GPIO34
- EC analog: GPIO32
- DS18B20 bus: GPIO33
- Pumps: GPIO15/2/4/16
- Stirrer PWM: GPIO25

---

## Purchase Priority
- Critical: electrical completion items (remaining MOSFETs, pull-up resistor, filtering capacitors)
- High: mechanical/fluidics completion (stirrer assembly, canisters, chamber parts, reagent)
- Optional: convenience/storage extras

---

## Safety Notes
- Set LM2596 to exact 5.0V before ESP32 connection.
- Use common ground between ESP32 and all MOSFET boards.
- Handle HCl with appropriate PPE.

---

## Polish (PL)
# Reef Sentinel – Bill of Materials
## Module 1: Sentinel Hub + Module 2: Sentinel Chem/Monitor

> **Version:** 1.0  
> **Date:** 2026-03-06  
> **Status projektu:** Sentinel Hub 🟢 90% ready | Sentinel Chem/Monitor 🟡 60% ready  
> **Autor:** reef-sentinel.com | github.com/reef-sentinel

---

## 📦 STATUS KOMPONENTÓW – PODSUMOWANIE

| Moduł | Co masz | Czego brakuje | Koszt dokupienia |
|-------|---------|---------------|-----------------|
| **Sentinel Hub** | ESP32, LM2596, OLED, SD reader, breadboard, zasilacz | Przewody dupont (jeśli brak) | ~0–10 zł |
| **Sentinel Chem/Monitor** | ESP32, LM2596, pH Kit (w drodze), EC sensor, DS18B20 ×3, MOSFET ×2, pompki ×4 | MOSFET ×3, rezystor 4.7kΩ, kondensatory, mieszadełko, magnesy, stir bar, epoksyd | **~177 zł** |
| **RAZEM** | | | **~177 zł** |

---

## MODULE 1: REEF HUB (Koordynator WiFi)

### Funkcje
- WiFi Access Point dla modułów (10.42.0.1)
- WiFi Client do sieci domowej (dual mode: AP+STA)
- Lokalny web dashboard (HTTP server)
- Scheduler testów (cron jobs)
- Agregacja danych z modułów
- Cloud sync → reef-sentinel.com/api (co 15 min)
- Status display (OLED)
- Data logging (SD card – opcjonalnie)

---

### 🔌 ELEKTRONIKA

#### Główny kontroler

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **ESP32 DevKit** | 30-pin ESP-WROOM-32 | Botland / Kamami | 1 | 25 zł | 25 zł | ✅ MASZ | Dual-core 240MHz, WiFi+BT |

#### Zasilanie

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **Zasilacz DC** | 12V/5A, gniazdo 5.5/2.1mm | Botland / Allegro | 1 | 25 zł | 25 zł | ✅ MASZ | Dedykowany dla Sentinel Hub |
| **LM2596 Buck Converter** | 12V→5V, z wyświetlaczem | Botland / AliExpress | 1 | 8 zł | 8 zł | ✅ MASZ | ⚠️ Ustaw DOKŁADNIE 5.0V multimetrem! |

#### Wyświetlacz

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **Grove OLED** | 0.96" I2C, SSD1306 | Botland / Seeed | 1 | 20 zł | 20 zł | ✅ MASZ | GPIO21 (SDA), GPIO22 (SCL) |

#### Zapis danych

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **SD Card Reader** | SPI module, 3.3V/5V | Botland / AliExpress | 1 | 8 zł | 8 zł | ✅ MASZ | Opcjonalny – backup danych |
| **Karta microSD** | 8–32 GB, Class 10 | Allegro / RTV Euro | 1 | 15 zł | 15 zł | 🟢 OPCJONALNY | Wymagana jeśli chcesz logowanie |

#### Narzędzia deweloperskie

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **Breadboard** | 400-hole | Botland | 1 | 8 zł | 8 zł | ✅ MASZ | |
| **Przewody Dupont** | F-F, M-F, M-M 20cm | Botland | 1 zestaw | 8 zł | 8 zł | ✅ MASZ | Sprawdź czy masz wystarczająco |

---

### 📌 GPIO Assignment – Sentinel Hub

| Funkcja | GPIO | Interfejs | Uwagi |
|---------|------|-----------|-------|
| OLED SDA | GPIO21 | I2C | |
| OLED SCL | GPIO22 | I2C | |
| SD CS | GPIO5 | SPI | |
| SD MOSI | GPIO23 | SPI | |
| SD MISO | GPIO19 | SPI | |
| SD SCK | GPIO18 | SPI | |
| Status LED | GPIO2 | Digital | Built-in LED |
| Home Assistant bridge | – | WiFi | Agregacja przez Home Assistant (ESPHome API) |

**GPIO użytych: 6 z ~25 dostępnych ✅**

---

### 💰 Koszt – Sentinel Hub

| Kategoria | Koszt |
|-----------|-------|
| ESP32 DevKit | 25 zł |
| Zasilacz 12V/5A | 25 zł |
| LM2596 Buck Converter | 8 zł |
| Grove OLED 0.96" | 20 zł |
| SD Card Reader | 8 zł |
| Karta microSD (opcja) | 15 zł |
| Breadboard + Dupont | 16 zł |
| **SUBTOTAL (bez microSD)** | **102 zł** |
| **SUBTOTAL (z microSD)** | **117 zł** |

> ✅ **Sentinel Hub: MOŻESZ BUDOWAĆ TERAZ** – wszystkie kluczowe komponenty są dostępne.

---

---

## MODULE 2: REEF CHEM + REEF MONITOR (Shared ESP32)

### Funkcje – Sentinel Chem
- Automatyczna titracja KH (HCl 0.1M, ~10 min, 1–7×/dzień)
- Monitoring pH 24/7 (z kompensacją temperatury)
- Magnetyczne mieszadełko (sterowanie PWM)
- Przechowywanie sondy pH w wodzie RO (żywotność 18–36 mies.)

### Funkcje – Sentinel Monitor
- Temperatura × 3 (akwarium, sump, komora) – DS18B20
- Zasolenie/EC (kompensacja temperatury, ciągłe)

---

### 🔌 ELEKTRONIKA

#### Główny kontroler

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **ESP32 DevKit** | 30-pin ESP-WROOM-32 | Botland / Kamami | 1 | 25 zł | 25 zł | ✅ MASZ | |

#### Zasilanie

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **Zasilacz DC** | 12V/3A, gniazdo 5.5/2.1mm | Botland / Allegro | 1 | 20 zł | 20 zł | 🔴 KUP | Dedykowany dla Sentinel Chem/Monitor |
| **LM2596 Buck Converter** | 12V→5V, z wyświetlaczem | Botland / AliExpress | 1 | 8 zł | 8 zł | ✅ MASZ | ⚠️ Ustaw DOKŁADNIE 5.0V! |
| **Kondensator elektrolityczny** | 100µF/25V | Botland | 2 | 1 zł | 2 zł | 🔴 KUP | Filtrowanie pH (dokładność!) |
| **Kondensator ceramiczny** | 100nF | Botland | 5 | 0.20 zł | 1 zł | 🔴 KUP | Decoupling ESP32 |

#### Czujniki – Sentinel Monitor

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **DS18B20 Waterproof** | 3m kabel, metal probe | Botland / Kamami | 3 | 18 zł | 54 zł | ✅ MASZ (3 szt.) | GPIO33, 1-Wire |
| **Rezystor 4.7kΩ** | 1/4W, THT | Botland | 5 | 0.10 zł | 0.50 zł | 🔴 KUP | Pull-up dla 1-Wire DS18B20 |
| **EC/TDS Sensor** | DFRobot Gravity TDS | Botland / DFRobot | 1 | 45 zł | 45 zł | ✅ MASZ | GPIO32, analog ADC1_CH4 |

#### Czujniki – Sentinel Chem

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **pH Sensor Kit** | DFRobot Gravity pH V2 | Botland / DFRobot | 1 | 184 zł | 184 zł | ✅ W DRODZE | GPIO34, ADC1_CH6 + kompensacja temp |

#### Sterowanie pompkami (MOSFET)

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **MOSFET Switch Board** | DFRobot Gravity 5-36V/20A (DFR-08104) | Botland / DFRobot | 5 | 18 zł | 90 zł | ⚠️ MASZ 2, KUP +3 | Low-side switching |
| **Rezystor 10kΩ** | 1/4W, THT (Gate pull-down) | Botland | 5 | 0.10 zł | 0.50 zł | 🔴 KUP | Opcjonalny – stabilizacja Gate |

> ⚠️ **WAŻNE – Common Ground:**  
> GND ESP32 MUSI być połączony z COM na każdej płytce MOSFET!  
> Bez tego pompki nie będą działać.

#### Pompki

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **Pompka perystaltyczna 12V** | 12V DC, ~30–50 ml/min | Allegro / AliExpress | 4 | 26 zł | 104 zł | ✅ MASZ (4 szt.) | #1 próbka, #2 HCl, #3 odpad, #4 RO |

#### Mieszadełko magnetyczne

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **Silnik gearmotor 12V** | 12V DC, 60 RPM | Allegro / AliExpress | 1 | 30 zł | 30 zł | ❌ BRAK | Sterowanie PWM – GPIO25 |
| **Magnesy neodymowe** | N52, 10×5mm | Allegro / AliExpress | 4 | 2 zł | 8 zł | ❌ BRAK | Do wirnika DIY |
| **Stir bar PTFE** | 25mm, cylindryczny | Allegro / AliExpress | 1 | 10 zł | 10 zł | ❌ BRAK | Musi być PTFE (odporny na HCl!) |
| **Klej epoksydowy** | 2-składnikowy, wodoodporny | Castorama / Leroy | 1 | 12 zł | 12 zł | ❌ BRAK | Do montażu magnesów |

#### Komora pomiarowa i pojemniki

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **Pojemnik komory** | Szklanka/pojemnik ~100 ml | Ikea / lokalny | 1 | 5 zł | 5 zł | ❌ BRAK | Przezroczysty (widać co się dzieje) |
| **Kanister 5L** | HDPE, zakrętka | Castorama / Leroy | 2 | 10 zł | 20 zł | ❌ BRAK | Jeden na odpad, jeden na wodę RO |

#### Reagenty

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **HCl 0.1M (kwas solny)** | 0.1 mol/L, 500 ml | Laboratorium / Allegro | 1 | 25 zł | 25 zł | ❓ SPRAWDŹ | Titrant do KH |

> ⚠️ **Bezpieczeństwo:** HCl to kwas – używaj rękawic i okularów. Przechowuj z dala od dzieci.

#### Narzędzia

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **Breadboard** | 400-hole | Botland | 1 | 8 zł | 8 zł | ✅ MASZ | |
| **Przewody Dupont** | F-F, M-F, M-M | Botland | 1 zestaw | 8 zł | 8 zł | 🟡 SPRAWDŹ | |
| **Multimetr** | Dowolny z DC Voltage | Rebel / Botland | 1 | 25 zł | 25 zł | ✅ MASZ | Konieczny do ustawienia LM2596! |
| **Zaciski śrubowe** | 2-pin, 3.5mm | Botland | 10 | 0.50 zł | 5 zł | 🟡 OPCJONALNE | Wygodniejsze niż Dupont dla produkcji |

---

### 📌 GPIO Assignment – Sentinel Chem/Monitor

| Funkcja | GPIO | Pin | Typ | Moduł |
|---------|------|-----|-----|-------|
| pH sensor (analog) | GPIO34 | D34 | ADC1_CH6 | Sentinel Chem |
| EC/TDS sensor (analog) | GPIO32 | D32 | ADC1_CH4 | Sentinel Monitor |
| DS18B20 (1-Wire shared) | GPIO33 | D33 | Digital | Sentinel Monitor |
| Pompka #1 – próbka | GPIO15 | D15 | Digital | Sentinel Chem |
| Pompka #2 – HCl | GPIO2 | D2 | Digital | Sentinel Chem |
| Pompka #3 – odpad | GPIO4 | D4 | Digital | Sentinel Chem |
| Pompka #4 – woda RO | GPIO16 | D16 | Digital | Sentinel Chem |
| Stirrer PWM | GPIO25 | D25 | PWM | Sentinel Chem |

**GPIO użytych: 8 z ~25 dostępnych ✅**

> **Nota:** GPIO34, 35, 36, 39 to wyłącznie INPUT na ESP32 – idealne dla czujników analogowych.  
> GPIO2 ma wbudowane LED – może migać podczas bootowania (normalne).

---

### 💰 Koszt – Sentinel Chem/Monitor

#### Co MASZ (koszt już poniesiony)

| Kategoria | Koszt |
|-----------|-------|
| ESP32 DevKit | ~25 zł |
| LM2596 | ~8 zł |
| DFRobot pH Gravity V2 Kit | ~184 zł |
| DFRobot EC/TDS Sensor | ~45 zł |
| DS18B20 × 3 | ~54 zł |
| MOSFET DFRobot × 2 | ~36 zł |
| Pompki 12V × 4 | ~104 zł |
| **Razem już wydane** | **~456 zł** |

#### Co MUSISZ KUPIĆ

| Priorytet | Komponent | Ilość | Cena | Gdzie |
|-----------|-----------|-------|------|-------|
| 🔴 CRITICAL | MOSFET DFRobot × 3 | 3 | ~54 zł | Botland |
| 🔴 CRITICAL | Rezystor 4.7kΩ (DS18B20 pull-up) | 5 | ~0.50 zł | Botland |
| 🔴 CRITICAL | Kondensator 100µF/25V | 2 | ~2 zł | Botland |
| 🔴 CRITICAL | Kondensator 100nF | 5 | ~1 zł | Botland |
| 🔴 CRITICAL | Przewody Dupont (jeśli brak) | 1 zestaw | ~8 zł | Botland |
| 🟡 HIGH | Silnik gearmotor 12V 60RPM | 1 | ~30 zł | Allegro |
| 🟡 HIGH | Magnesy N52 10×5mm × 4 | 1 zestaw | ~8 zł | Allegro |
| 🟡 HIGH | Stir bar PTFE 25mm | 1 | ~10 zł | Allegro |
| 🟡 HIGH | Klej epoksydowy | 1 | ~12 zł | Castorama |
| 🟡 HIGH | Kanister 5L × 2 | 2 | ~20 zł | Castorama |
| 🟡 HIGH | Pojemnik komory 100ml | 1 | ~5 zł | Ikea |
| 🟡 HIGH | HCl 0.1M 500ml | 1 | ~25 zł | Allegro |
| 🟢 OPCJONALNE | Karta microSD 8–32GB | 1 | ~15 zł | Allegro |
| 🟢 OPCJONALNE | Zaciski śrubowe 2-pin | 10 | ~5 zł | Botland |

#### Podsumowanie zakupów

| Kategoria | Koszt |
|-----------|-------|
| 🔴 CRITICAL (elektronika) | ~65 zł |
| 🟡 HIGH (mieszadełko + pojemniki + reagent) | ~110 zł |
| 🟢 OPCJONALNE | ~20 zł |
| **RAZEM do wydania (critical + high)** | **~175 zł** |

---

## 🛒 LISTA ZAKUPÓW – GDZIE KUPIĆ

### Botland (botland.com.pl)
- MOSFET DFRobot DFR-08104 × 3
- Rezystory 4.7kΩ × 10
- Kondensatory 100µF/25V × 2
- Kondensatory 100nF × 5
- Przewody Dupont (zestaw)

### Allegro
- Silnik gearmotor 12V 60RPM (szukaj: "12V DC gear motor 60rpm")
- Magnesy neodymowe N52 10×5mm (szukaj: "magnesy neodymowe N52 10x5")
- Stir bar PTFE 25mm (szukaj: "stir bar magnetic PTFE 25mm")
- HCl 0.1M (szukaj: "kwas solny 0.1M laboratoryjny")

### Castorama / Leroy Merlin
- Klej epoksydowy 2-składnikowy (Poxipol lub odpowiednik)
- Kanistry HDPE 5L × 2

### Ikea / sklep lokalny
- Pojemnik szklany lub plastikowy ~100 ml (komora pomiarowa)

---

## ⚠️ KRYTYCZNE UWAGI BEZPIECZEŃSTWA

```
PRZED PIERWSZYM WŁĄCZENIEM – CHECKLIST:

☐ LM2596 ustawiony na DOKŁADNIE 5.0V (multimetrem!)
   → Wyższe napięcie = natychmiastowa śmierć ESP32
☐ Common Ground: GND ESP32 ↔ COM każdej płytki MOSFET
   → Bez tego pompki nie działają
☐ Pompki: Plus → zasilanie 12V | Minus → MOSFET (low-side)
☐ Rezystor 4.7kΩ między DATA a VCC dla DS18B20
   → Bez pull-up czujnik nie działa / niestabilne odczyty
☐ Kondensatory filtrujące przy pH sensor
   → Bez tego odczyty pH będą skakać
☐ HCl – rękawice + okulary przy pracy z kwasem
```

---

## 🔗 POWIĄZANE DOKUMENTY

- [`sentinel_view_BOM.md`](./sentinel_view_BOM.md) – BOM dla Sentinel View Display (Faza 2, Q2 2026)
- `BUILD_GUIDE.md` *(do stworzenia)* – Instrukcja montażu krok po kroku
- `WIRING_DIAGRAM.md` *(do stworzenia)* – Schemat połączeń

---

*Reef Sentinel – Open-source aquarium controller*  
*reef-sentinel.com | github.com/reef-sentinel*  
*Ostatnia aktualizacja: 2026-03-06*

