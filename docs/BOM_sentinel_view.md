# Reef Sentinel – Bill of Materials
## Module 4: Sentinel View (Panel dotykowy)

> **Version:** 1.1
> **Date:** 2026-03-07
> **Status projektu:** ⏸️ Q2 2026 – planowany start budowy
> **Autor:** reef-sentinel.com | github.com/reef-sentinel
> **Poprzednia wersja:** reef_view_BOM.md (v2.0, archiwalna)

---

## 📦 STATUS KOMPONENTÓW – PODSUMOWANIE

| Kategoria | Co masz | Czego brakuje | Koszt zakupu |
|-----------|---------|---------------|--------------|
| Nextion Display | ❌ Brak | NX8048P050-011C | ~295 zł |
| ESP32 + zasilanie | ❌ Brak | ESP32, LM2596, zasilacz 12V/2A | ~53 zł |
| Mechanika (obudowa, montaż) | ❌ Brak | Obudowa 3D, magnesy, przegub, śruby | ~27 zł |
| **RAZEM** | | | **~375 zł** |

---

## FUNKCJE MODUŁU

### Sentinel View – panel dotykowy
- Wyświetlanie danych Reef Sentinel Lab w czasie rzeczywistym (pH, KH, Ca, Mg, Temp ×3, EC)
- 5 ekranów dotykowych (Home, Charts, Tests, Settings, Alerts)
- Wykresy trendów 7-dniowych
- Uruchamianie testów KH/Ca/Mg przyciskiem dotykowym
- Alerty dźwiękowe (buzzer pasywny) dla wartości poza normą
- RTC wbudowany w Nextion – wyświetla datę i godzinę bez NTP
- Podświetlenie LCD z auto-brightness (opcja: czujnik BH1750)

### Komunikacja
- ESP32 łączy się z AP `SentinelHub` (10.42.0.1)
- Nextion odbiera dane przez UART od ESP32 (4 przewody)
- Aktualizacja danych na ekranie co 30 sekund

### Dlaczego Nextion Intelligent Series?

| | Nextion NX8048P050 (P-series) | Nextion NX8048K050 (K-series) | ESP32-S3 + IPS LCD |
|--|-------------------------------|-------------------------------|-------------------|
| MCU | Cortex-M4, 200 MHz | Cortex-M0, 48 MHz | – (brak) |
| Flash | 120 MB | 16 MB | ~4 MB |
| RAM | 512 KB | 8 KB | – |
| EEPROM | 1024 B | Brak | – |
| Touch | Pojemnościowy | Rezystancyjny | – |
| Cena | ~295 zł | ~370 zł | ~148 zł |
| Czas wdrożenia UI | 3–5 dni | 3–5 dni | 2–4 tygodnie |
| Ryzyko tech. | Niskie | Niskie | Wysokie |

> P-series jest tańsza od K-series, szybsza i ma więcej pamięci. ✅

---

## 🔌 ELEKTRONIKA

### Wyświetlacz (główny komponent)

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **Nextion Intelligent Display** | NX8048P050-011C, 5.0", 800×480, pojemnościowy | elty.pl (Kraków) | 1 | 295 zł | 295 zł | 🔴 KUP | Własny MCU 200MHz, 120MB flash, RTC wbudowany |

**Specyfikacja Nextion NX8048P050-011C:**

| Parametr | Wartość |
|---------|---------|
| Przekątna | 5.0" |
| Rozdzielczość | 800 × 480 px |
| MCU | Cortex-M4, 200 MHz |
| Flash | 120 MB |
| SRAM | 512 KB |
| EEPROM | 1024 B |
| Touch | Pojemnościowy (smooth jak smartfon) |
| RTC | Wbudowany |
| Interfejs | UART (TTL 3.3V/5V) |
| Zasilanie | 5V DC, max 500 mA |
| Wymiary modułu | 132.0 × 82.8 × 12.9 mm |
| Otwór wyświetlacza | 122.0 × 72.6 mm |

### Kontroler WiFi (bridge)

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **ESP32 DevKit** | 30-pin ESP-WROOM-32 | Botland / Kamami | 1 | 25 zł | 25 zł | 🔴 KUP | Rola: WiFi → UART bridge. Zwykły ESP32 wystarczy – brak potrzeby S3. |

### Zasilanie

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **Zasilacz DC** | 12V/2A, gniazdo 5.5/2.1mm | Botland / Allegro | 1 | 20 zł | 20 zł | 🔴 KUP | Dedykowany dla View |
| **LM2596 Buck Converter** | 12V→5V z wyświetlaczem | Botland / AliExpress | 1 | 8 zł | 8 zł | 🔴 KUP | ⚠️ Ustaw DOKŁADNIE 5.0V multimetrem! Nextion + ESP32 = ~750 mA łącznie. |

> **Bilans prądowy:**
> Nextion NX8048P050: ~500 mA (max) + ESP32: ~250 mA (WiFi aktywne) = **~750 mA łącznie**
> LM2596 wytrzymuje do 3A → margines bezpieczeństwa ✅

### Dźwięk

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **Buzzer pasywny** | 5V, 85dB | Botland / AliExpress | 1 | 3 zł | 3 zł | 🔴 KUP | Alerty dźwiękowe – sterowany PWM z GPIO25 |

### Opcje dodatkowe

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **Czujnik światła BH1750** | I2C, 1–65535 lux | AliExpress / Botland | 1 | 8 zł | 8 zł | 🟢 OPCJONALNY | Auto-brightness Nextion |
| **Głośniejszy buzzer** | 95dB, aktywny, 5V | AliExpress | 1 | 5 zł | 5 zł | 🟢 OPCJONALNY | Zastępuje 85dB – głośniejsze alerty |

---

## 📌 GPIO Assignment – ESP32 (bridge)

| Funkcja | GPIO | Interfejs | Uwagi |
|---------|------|-----------|-------|
| Nextion RX | GPIO17 (TX2) | UART2 | ESP32 nadaje → Nextion odbiera |
| Nextion TX | GPIO16 (RX2) | UART2 | Nextion nadaje → ESP32 odbiera |
| Buzzer | GPIO25 | PWM (LEDC) | Pasywny buzzer wymaga PWM |
| BH1750 SDA | GPIO21 | I2C | Opcjonalny – auto-brightness |
| BH1750 SCL | GPIO22 | I2C | Opcjonalny |
| Status LED | GPIO2 (built-in) | Digital | Wbudowana dioda |
| WiFi | Wbudowane | – | Połączenie z SentinelHub AP |

**GPIO użytych: 3 (bez opcji) / 5 (z BH1750) ✅**

---

## 🔧 MECHANIKA

### Obudowa

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **Obudowa drukowana 3D** | PETG, 2 części (przód + tył) | Własna drukarka / Printuj3D.pl | 1 kpl | 15 zł | 15 zł | 🔴 KUP | Custom pod wymiary Nextion. PETG – lepsza odporność na ciepło niż PLA |

**Parametry projektowe obudowy:**

| Wymiar | Wartość |
|--------|---------|
| Otwór wyświetlacza | 122.0 × 72.6 mm |
| Moduł Nextion (całkowity) | 132.0 × 82.8 × 12.9 mm |
| Grubość ścianek (min.) | 2 mm |
| Materiał | PETG (30% infill wystarczy) |
| Otwór kablowy | min. 8mm Ø, dolna ścianka |
| Otwór gniazda DC | 5.5/2.1mm, boczna ścianka |

### Podstawka i montaż

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **Magnesy neodymowe N52** | 10mm × 2mm | AliExpress / Allegro | 4 | 0.50 zł | 2 zł | 🔴 KUP | Mocowanie do szafki/krawędzi akwarium |
| **Podstawka drukowana 3D** | PETG | Własna / Printuj3D.pl | 1 | 5 zł | 5 zł | 🔴 KUP | Na magnesy + gniazdo przegubu M6 |
| **Przegub kulowy M6** | Regulowany kąt, stal | AliExpress | 1 | 8 zł | 8 zł | 🔴 KUP | Regulacja kąta widzenia |

### Złącza i śruby

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **Śruby M2.5 × 6mm** | Phillips, stal | Sklep lokalny / AliExpress | 4 | 0.20 zł | 0.80 zł | 🔴 KUP | Mocowanie Nextion do obudowy |
| **Wkładki gwintowane M2.5** | Mosiądz, do druku 3D | AliExpress | 4 | 0.30 zł | 1.20 zł | 🔴 KUP | Wcisnąć lutownicą w otwory obudowy |

### Materiały montażowe

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **Taśma dwustronna 3M VHB** | Piankowa, mocna | Sklep lokalny | 1 pasek | 2 zł | 2 zł | 🔴 KUP | Montaż ESP32 i LM2596 wewnątrz obudowy |
| **Klej epoksydowy** | 2-składnikowy | Castorama / Leroy | 1 | 8 zł | 8 zł | 🔴 KUP | Wklejenie magnesów w podstawkę |
| **Przewody Dupont F-F** | 20cm | Botland | 4 szt. | 0.10 zł | 0.40 zł | 🔴 KUP | TX, RX, 5V, GND do Nextion |

---

## 💻 OPROGRAMOWANIE / NARZĘDZIA

| Narzędzie | Wersja | Gdzie pobrać | Koszt |
|-----------|--------|-------------|-------|
| **Nextion Editor** | v1.67+ | nextion.com/download | Bezpłatny |
| **ESPHome** | 2026.x | esphome.io / pip install esphome | Bezpłatny |

**Planowane ekrany Sentinel View:**

| Ekran | Nazwa | Zawartość |
|-------|-------|-----------|
| 0 | Home | pH, KH, Temp ×3, EC – kolorowe wskaźniki OK/WARN/ALARM |
| 1 | Charts | Wykresy trendów 7-dniowych dla pH i temperatury |
| 2 | Tests | Przyciski Start KH Test / Ca Test + status na żywo |
| 3 | Settings | Harmonogram testów, kalibracja, info WiFi |
| 4 | Alerts | Lista alertów + wyciszenie buzzera |

---

## 💰 PODSUMOWANIE KOSZTÓW

### Wersja Standard

| Kategoria | Koszt |
|-----------|-------|
| Nextion NX8048P050-011C | 295 zł |
| ESP32 DevKit | 25 zł |
| Zasilacz 12V/2A | 20 zł |
| LM2596 Buck Converter | 8 zł |
| Buzzer pasywny 85dB | 3 zł |
| Obudowa PETG (przód + tył) | 15 zł |
| Podstawka PETG + magnesy N52 + przegub M6 | 15 zł |
| Śruby M2.5 + wkładki mosiężne | 2 zł |
| Taśma VHB + klej epoksydowy + Dupont | ~12 zł |
| **SUBTOTAL (komponenty)** | **~395 zł** |

### Wersja Pro (z opcjami)

| Kategoria | Koszt |
|-----------|-------|
| Komponenty Standard | ~395 zł |
| Czujnik światła BH1750 (auto-brightness) | 8 zł |
| Głośniejszy buzzer 95dB | 5 zł |
| **SUBTOTAL (komponenty Pro)** | **~408 zł** |

### Szacowany koszt urządzenia gotowego (przy sprzedaży)

| Kategoria | Wersja Standard | Wersja Pro |
|-----------|----------------|------------|
| Komponenty | ~395 zł | ~408 zł |
| Montaż | 25 zł | 25 zł |
| UI (Nextion Editor, czas) | 30 zł | 30 zł |
| QA + pakowanie | 25 zł | 30 zł |
| **KOSZT CAŁKOWITY** | **~475 zł** | **~493 zł** |

**Sugerowana cena sprzedaży:** Standard 620 zł / Pro 680 zł
**Marża:** Standard ~145 zł (23%) / Pro ~187 zł (27.5%)

---

## 🛒 LISTA ZAKUPÓW – GDZIE KUPIĆ

### elty.pl (Kraków) – TYLKO TUTAJ w Polsce
- **Nextion NX8048P050-011C** (~295 zł)
  Oficjalny dystrybutor Nextion w Polsce. Szybka wysyłka z Krakowa.
  *(Alternatywnie: bezpośrednio z nextion.com – dłuższy czas dostawy z CN)*

### Botland (botland.com.pl)
- ESP32 DevKit 30-pin
- LM2596 Buck Converter
- Buzzer pasywny 5V/85dB
- Przewody Dupont F-F 20cm ×5
- Taśma dwustronna 3M VHB

### Allegro / sklep lokalny
- Zasilacz 12V/2A
- Śruby M2.5 × 6mm ×8 (zapas)
- Magnesy neodymowe N52 10×2mm ×8 (zapas)

### AliExpress
- Wkładki gwintowane M2.5 mosiądz ×10 (zapas)
- Przegub kulowy M6 ×2 (zapas)
- BH1750 moduł I2C (opcja)

### Castorama / Leroy Merlin
- Klej epoksydowy 2-składnikowy (Poxipol lub odpowiednik)

### Własna drukarka 3D lub Printuj3D.pl (printuj3d.pl)
- Obudowa Sentinel View PETG (plik STL do stworzenia)
- Podstawka magnetyczna PETG
- Materiał: PETG (nie PLA – Nextion generuje trochę ciepła)

---

## ⚠️ KRYTYCZNE UWAGI

```
PRZED PIERWSZYM WŁĄCZENIEM – CHECKLIST:

☐ LM2596 ustawiony na DOKŁADNIE 5.0V (multimetrem!)
   → Nextion jest wrażliwy: poniżej 4.8V może się nie inicjalizować
   → Powyżej 5.2V ryzyko uszkodzenia wyświetlacza

☐ UART skrzyżowany: TX ESP32 → RX Nextion, RX ESP32 → TX Nextion
   → To NAJCZĘSTSZY błąd: TX→TX = brak komunikacji

☐ UI wgrane na Nextion przed zamknięciem obudowy
   → Bez pliku .tft ekran jest czarny po uruchomieniu

☐ Test ESP32 USB PRZED zamontowaniem w obudowie
   → Po skręceniu obudowy dostęp do portu USB może być utrudniony

☐ Buzzer pasywny wymaga PWM (nie prostego HIGH/LOW)
   → Użyj platform: ledc w ESPHome dla sygnału audio
```

---

## 🔗 POWIĄZANE DOKUMENTY

- `BUILD_GUIDE_sentinel_view.md` – instrukcja montażu krok po kroku
- `reef_hub_chem_monitor_BOM.md` – BOM dla Modułów 1+2
- `BOM_sentinel_photometer.md` – BOM dla Modułu 3
- `BOM_sentinel_connector.md` – BOM dla Modułu 5
- `firmware/sentinel_view.yaml` *(do stworzenia)*

---

*Reef Sentinel Lab – Open-source aquarium controller*
*reef-sentinel.com | github.com/reef-sentinel*
*Ostatnia aktualizacja: 2026-03-07*
