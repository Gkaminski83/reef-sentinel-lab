# Sentinel View Display - Bill of Materials (BOM)

## Version: 2.0 (Nextion-based)
## Status: ⏸️ Faza 2 – Q2 2026 (po stabilizacji Sentinel Hub + Sentinel Chem/Monitor)

> **Ważna zmiana vs v1.0 BOM:**  
> Poprzedni BOM opisywał ESP32-S3 + LCD IPS (SPI) – podejście porzucone.  
> Obecny BOM oparty na **Nextion NX8048P050-011C** (Intelligent Series) –  
> wyświetlacz ze własnym procesorem, pamięcią i komunikacją UART.  
> ESP32 pełni rolę WiFi bridge (odbiera dane z Sentinel Hub, wysyła do Nextion przez UART).

---

## ARCHITEKTURA – JAK TO DZIAŁA

```
Sentinel Hub (10.42.0.1)
       │
    WiFi / MQTT
       │
  ESP32 (bridge)   ← tylko WiFi + UART, brak SPI/I2C do wyświetlacza
       │
     UART (TX/RX)
       │
  Nextion NX8048P050-011C
  ┌─────────────────────────────┐
  │  Własny MCU (200 MHz)       │
  │  Flash 120 MB (UI assets)   │
  │  SRAM 512 KB                │
  │  Touch capacitive           │
  │  RTC wbudowany              │
  │  Graficzny UI (Nextion Ed.) │
  └─────────────────────────────┘
```

**Zalety vs ESP32-S3 + IPS LCD:**
- Nextion ma własny MCU – ESP32 nie traci zasobów na rendering
- UI projektowane w Nextion Editor (drag & drop, bez kodowania grafiki)
- Prostsze połączenie: tylko 4 przewody (TX, RX, 5V, GND)
- RTC wbudowany (wyświetla godzinę bez zewnętrznego modułu)
- 120 MB flash (przechowuje grafikę, animacje, ikonki)

---

## ELECTRONICS

### Wyświetlacz (główny komponent)

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Part# |
|-----------|-----------|----------|-------|-----------|-------|-------|
| **Nextion Intelligent Display** | NX8048P050-011C, 5.0", 800×480, pojemnościowy dotyk | elty.pl (Kraków) | 1 | 295 zł | 295 zł | NX8048P050-011C |

**Specyfikacja Nextion NX8048P050-011C:**

| Parametr | Wartość |
|---------|---------|
| Przekątna | 5.0" |
| Rozdzielczość | 800 × 480 px |
| MCU | Cortex-M4, 200 MHz |
| Flash | 120 MB (UI + assets) |
| SRAM | 512 KB |
| EEPROM | 1024 Byte |
| Touch | Pojemnościowy (smooth jak smartfon) |
| RTC | Wbudowany (data + czas) |
| Interfejs | UART (TTL 3.3V/5V) |
| Zasilanie | 5V DC |
| Pobór prądu | ~500 mA (max) |

**Dlaczego P-series (nie K-series):**
- 75 zł taniej od K-series przy lepszych parametrach
- 2.5× szybszy MCU (płynniejszy UI)
- 7.5× więcej flash (więcej ikon, animacji, ekranów)
- 64× więcej RAM (bez lagów przy wykresach)
- Jedyny który ma EEPROM (zapis ustawień lokalnie)
- Pojemnościowy dotyk zamiast rezystancyjnego

**Dostawca:**
- **elty.pl** – Kraków, Polska, szybka wysyłka
- URL: elty.pl (szukaj: NX8048P050-011C)

---

### Kontroler WiFi (bridge)

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|-------|
| **ESP32 DevKit** | 30-pin ESP-WROOM-32 | Botland / Kamami | 1 | 25 zł | 25 zł | Zwykły ESP32 wystarczy – brak potrzeby S3 |

**Uwaga:** ESP32-S3 (z poprzedniego BOM) nie jest już potrzebny.  
Nextion ma własny MCU, ESP32 pełni tylko rolę WiFi → UART bridge.  
Zwykły ESP32-WROOM-32 (30-pin) w zupełności wystarcza.

---

### Zasilanie

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|-------|
| **LM2596 Buck Converter** | 12V→5V, z wyświetlaczem | Botland / AliExpress | 1 | 8 zł | 8 zł | ⚠️ Ustaw DOKŁADNIE 5.0V multimetrem! |

**Uwaga dotycząca zasilania:**
- Nextion wymaga stabilnego 5V / min. 1A
- ESP32 (przez pin 5V lub VIN): ~250 mA
- Nextion NX8048P050: ~500 mA
- **Łącznie: ~750 mA → LM2596 z marginesem OK**
- Nextion i ESP32 mogą dzielić jedno 5V z LM2596

---

### Połączenie Nextion ↔ ESP32

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|-------|
| **Przewody Dupont** | F-F, 20cm | Botland | 4 szt. | 0.10 zł | 0.40 zł | TX, RX, 5V, GND |

**Schemat podłączenia UART:**

```
ESP32           Nextion
GPIO17 (TX2) ──→ RX (pin 2)
GPIO16 (RX2) ←── TX (pin 1)
GND          ──→ GND (pin 3)
5V           ──→ 5V (pin 4)
```

> ⚠️ Nextion pracuje na poziomach TTL 3.3V/5V – bezpośrednie połączenie z ESP32 (3.3V) jest bezpieczne. Brak potrzeby konwertera poziomów.

---

### Audio (opcjonalnie)

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|-------|
| **Buzzer pasywny** | 5V, 85dB | Botland / AliExpress | 1 | 3 zł | 3 zł | Alerty dźwiękowe – sterowany przez ESP32 PWM |

---

## GPIO Assignment – ESP32 (bridge)

| Funkcja | GPIO | Uwagi |
|---------|------|-------|
| Nextion TX | GPIO16 (RX2) | UART2 receive |
| Nextion RX | GPIO17 (TX2) | UART2 transmit |
| Buzzer | GPIO25 | PWM |
| WiFi | wbudowane | Połączenie z Sentinel Hub (MQTT) |
| Status LED | GPIO2 | Built-in |

**GPIO użytych: 3 – minimalistyczne! ✅**

---

## OPROGRAMOWANIE / NARZĘDZIA

### Nextion Editor

| Narzędzie | Wersja | Gdzie pobrać | Koszt |
|-----------|--------|-------------|-------|
| **Nextion Editor** | v1.67+ | nextion.com/download | Bezpłatny |

**Co robi Nextion Editor:**
- Projektowanie ekranów drag & drop (bez kodowania UI)
- Definiowanie przycisków, wykresów, etykiet tekstowych
- Upload UI na Nextion przez UART lub kartę microSD
- Symulator (preview bez fizycznego urządzenia)

**Planowane ekrany Sentinel View:**
1. **Home** – aktualne odczyty (pH, KH, temp, zasolenie) z kolorowym statusem
2. **Charts** – wykresy trendów 7-dniowych
3. **Tests** – ręczne uruchamianie testów ("Start KH Test")
4. **Settings** – konfiguracja WiFi, harmonogram testów, kalibracja
5. **Alerts** – ostrzeżenia wizualne + buzzer dla wartości poza normą

---

## MECHANIKA

### Obudowa

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|-------|
| **Obudowa drukowana 3D** | PLA/PETG, 2 części (przód + tył) | Własna drukarka / serwis | 1 kpl | 15 zł | 15 zł | Custom pod wymiary Nextion 5" |

**Wymiary Nextion NX8048P050-011C:**
- Moduł: 132.0 × 82.8 × 12.9 mm
- Otwór wyświetlacza: 122.0 × 72.6 mm
- Obudowę zaprojektować pod te wymiary (Nextion Editor podaje dokładne wymiary)

**Jeśli brak drukarki:**
- Serwis Printuj3D.pl: ~20–25 zł/szt
- Zamówienie batch (50+ szt): ~12 zł/szt

---

### Podstawka (magnetyczna)

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|-------|
| **Magnesy neodymowe** | N52, 10mm × 2mm | AliExpress / Allegro | 4 | 0.50 zł | 2 zł | Mocowanie do szafki/krawędzi |
| **Podstawka drukowana 3D** | PLA/PETG | Własna / serwis | 1 | 5 zł | 5 zł | |
| **Przegub kulowy** | M6, regulowany kąt | AliExpress | 1 | 8 zł | 8 zł | Regulacja kąta widzenia |

```
[Wyświetlacz Sentinel View]
        │
  [Przegub M6]  ← regulowany kąt
        │
  [Podstawka z magnesami]
  🧲🧲🧲🧲
```

---

### Złącza i śruby

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|-------|
| **Śruby M2.5** | 6mm, Phillips | Sklep lokalny / AliExpress | 4 | 0.20 zł | 0.80 zł | |
| **Wkładki gwintowane M2.5** | Mosiądz, do druku 3D | AliExpress | 4 | 0.30 zł | 1.20 zł | Profesjonalne mocowanie, nie rwie plastiku |

---

## MATERIAŁY MONTAŻOWE

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem |
|-----------|-----------|----------|-------|-----------|-------|
| **Przewody Dupont F-F** | 20cm | Botland | 4 szt. | 0.10 zł | 0.40 zł |
| **Taśma dwustronna 3M VHB** | Piankowa | Sklep lokalny | 1 pasek | 2 zł | 2 zł |

---

## OPCJONALNIE (wersja Pro)

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|-------|
| **Czujnik światła** | BH1750 lub VEML7700, I2C | AliExpress | 1 | 8 zł | 8 zł | Auto-jasność ekranu |
| **Głośniejszy buzzer** | 95dB, aktywny | AliExpress | 1 | 5 zł | 5 zł | Głośniejsze alerty |
| **Podstawka aluminiowa** | CNC, frezowana | AliExpress / custom | 1 | 35 zł | 35 zł | Premium look |

---

## PODSUMOWANIE KOSZTÓW

### Wersja Standard:

| Kategoria | Koszt |
|-----------|-------|
| Nextion NX8048P050-011C | 295 zł |
| ESP32 DevKit (bridge) | 25 zł |
| LM2596 Buck Converter | 8 zł |
| Buzzer | 3 zł |
| Obudowa drukowana 3D | 15 zł |
| Podstawka magnetyczna (magnesy + wydruk + przegub) | 15 zł |
| Śruby + wkładki | 2 zł |
| Przewody + taśma | 2.40 zł |
| **SUBTOTAL (komponenty)** | **365 zł** |
| Montaż (przy sprzedaży gotowej) | 25 zł |
| Kontrola jakości | 10 zł |
| Pakowanie (pudełko, pianka, instrukcja) | 15 zł |
| **KOSZT CAŁKOWITY / szt.** | **415 zł** |

**Sugerowana cena sprzedaży:** 550 zł  
**Marża:** 135 zł (24.5%)

> Nextion jest droższy od IPS LCD (~295 zł vs ~55 zł), ale eliminuje potrzebę ESP32-S3,
> upraszcza firmware i daje profesjonalny efekt końcowy przy mniejszym nakładzie pracy.

---

### Wersja Pro (+opcje):

| Kategoria | Koszt |
|-----------|-------|
| Komponenty standard | 365 zł |
| Czujnik światła BH1750 | 8 zł |
| Głośniejszy buzzer | 5 zł |
| Podstawka aluminium (zamiast plastiku) | +20 zł |
| Premium packaging | +8 zł |
| **SUBTOTAL (komponenty)** | **406 zł** |
| Montaż + QC + pakowanie | 55 zł |
| **KOSZT CAŁKOWITY / szt.** | **461 zł** |

**Sugerowana cena sprzedaży:** 650 zł  
**Marża:** 189 zł (29.1%)

---

### Porównanie z poprzednim BOM (ESP32-S3 + IPS LCD):

| | Stary BOM (ESP32-S3 + IPS) | Nowy BOM (Nextion) |
|--|--------------------------|-------------------|
| Koszt komponentów | ~148 zł | ~365 zł |
| Trudność firmware | Wysoka (LVGL/custom graphics) | Niska (Nextion Editor drag & drop) |
| Czas wdrożenia UI | 2–4 tygodnie | 3–5 dni |
| Jakość końcowa | Zależy od umiejętności | Profesjonalna od razu |
| Ryzyko techniczne | Wysokie (SPI timing, RAM) | Niskie |
| **Rekomendacja** | ❌ Porzucone | ✅ Aktualne |

---

## GDZIE KUPIĆ

| Komponent | Sklep | Uwagi |
|-----------|-------|-------|
| Nextion NX8048P050-011C | **elty.pl** (Kraków) | Polska, szybka wysyłka, oficjalny dystrybutor |
| ESP32 DevKit | Botland / Kamami | Dostępny od ręki |
| LM2596 | Botland / AliExpress | |
| Magnesy N52, buzzer, przegub M6 | AliExpress / Allegro | |
| Obudowa | Printuj3D.pl lub własna drukarka | Plik STL do stworzenia |

---

## KOLEJNE KROKI (Q2 2026)

1. Sentinel Hub + Sentinel Chem/Monitor stabilne na akwarium ✅
2. Zamówienie Nextion z elty.pl (295 zł)
3. Pobranie Nextion Editor (nextion.com/download, bezpłatny)
4. Projektowanie UI w Nextion Editor (ekrany 1–5)
5. Firmware ESP32 – bridge MQTT → UART
6. Druk 3D obudowy (wymiary z datasheetu Nextion)
7. Integracja z Sentinel Hub (MQTT subscribe)
8. Testy na akwarium

**Szacowany czas wdrożenia:** 2–3 tygodnie (po zamówieniu Nextion)

---

## POWIĄZANE DOKUMENTY

- [`sentinel_hub_chem_monitor_BOM.md`](./sentinel_hub_chem_monitor_BOM.md) – BOM dla Sentinel Hub + Sentinel Chem/Monitor (Faza 1)
- `BUILD_GUIDE.md` *(do stworzenia)* – Instrukcja montażu
- `NEXTION_UI_GUIDE.md` *(do stworzenia)* – Projektowanie UI w Nextion Editor

---

*Reef Sentinel – Open-source aquarium controller*  
*reef-sentinel.com | github.com/reef-sentinel*  
*Ostatnia aktualizacja: 2026-03-06*
