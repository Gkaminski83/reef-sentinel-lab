# Reef Sentinel – Bill of Materials
## Module 3: Sentinel Photometer (Fotometr Ca/Mg)

> **Version:** 0.1
> **Date:** 2026-03-07
> **Status projektu:** ⏸️ Q3 2026 – planowany start budowy
> **Autor:** reef-sentinel.com | github.com/reef-sentinel

---

## 📦 STATUS KOMPONENTÓW – PODSUMOWANIE

| Kategoria | Co masz | Czego brakuje | Koszt zakupu |
|-----------|---------|---------------|--------------|
| Elektronika (ESP32, LM2596, zasilacz) | ❌ Brak – osobny moduł | ESP32, LM2596, zasilacz 12V/2A, kondensatory, breadboard | ~78 zł |
| Blok optyczny | ❌ Brak | LED 650nm ×2, LED 520nm ×2, BPW34 ×2, ADS1115, komora 3D, rezystory | ~40 zł |
| Blok fluidyczny | ❌ Brak | Pompki ×3, MOSFET ×3, rurki, złączki Y, butelki | ~124 zł |
| **RAZEM** | | | **~242 zł** |

> ⚠️ BOM na etapie planowania – ceny orientacyjne. Weryfikacja przy budowie Q3 2026.

---

## FUNKCJE MODUŁU

### Reef Photometer – Ca i Mg
- Pomiar wapnia (Ca) metodą fotometryczną – barwnik Arsenazo III, LED 650 nm
- Pomiar magnezu (Mg) metodą fotometryczną – barwnik Calmagite, LED 520 nm
- Dokładność: Ca ±10 ppm, Mg ±30 ppm (przy kalibracji 2-punktowej)
- Czas pomiaru: ~8 minut (Ca + Mg sekwencyjnie)
- Harmonogram: 1× dziennie domyślnie, na żądanie z panelu Hub
- Automatyczne spłukiwanie komory wodą RO po każdym pomiarze
- Kalibracja 2-punktowa przez panel Hub (`http://reef-sentinel.local`)

### Komunikacja
- ESP32 łączy się z AP `SentinelHub` (10.42.0.1)
- Dane Ca i Mg → HTTP REST do Hub co 15 minut
- Hub → reef-sentinel.com (AI insights dla dozowania)

---

## 🔌 ELEKTRONIKA

### Główny kontroler

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **ESP32 DevKit** | 30-pin ESP-WROOM-32 | Botland / Kamami | 1 | 25 zł | 25 zł | 🔴 KUP | Dedykowany dla Photometer |

### Zasilanie

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **Zasilacz DC** | 12V/2A, gniazdo 5.5/2.1mm | Botland / Allegro | 1 | 20 zł | 20 zł | 🔴 KUP | Dedykowany – bez pompek 2A wystarczy |
| **LM2596 Buck Converter** | 12V→5V z wyświetlaczem | Botland / AliExpress | 1 | 8 zł | 8 zł | 🔴 KUP | ⚠️ Ustaw DOKŁADNIE 5.0V multimetrem! |
| **Kondensator elektrolityczny** | 100µF/25V | Botland | 3 | 1 zł | 3 zł | 🔴 KUP | Filtrowanie zasilania |
| **Kondensator ceramiczny** | 100nF | Botland | 5 | 0.20 zł | 1 zł | 🔴 KUP | Decoupling ESP32 i ADS1115 |

### Blok optyczny – detekcja absorpcji

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **LED 650 nm** | 5mm, narrow beam, peak 650nm | AliExpress / Botland | 2 | 2 zł | 4 zł | 🔴 KUP | Kanał Ca (Arsenazo III) |
| **LED 520 nm** | 5mm, narrow beam, peak 520nm | AliExpress / Botland | 2 | 2 zł | 4 zł | 🔴 KUP | Kanał Mg (Calmagite) |
| **Fotodioda BPW34** | Silicon PIN, 400–1100nm, THT | Botland / TME | 2 | 3 zł | 6 zł | 🔴 KUP | Szeroka odpowiedź spektralna |
| **ADS1115** | 16-bit ADC, I2C, 4-kanałowy | Botland / AliExpress | 1 | 15 zł | 15 zł | 🔴 KUP | Precyzja 16-bit vs 12-bit ESP32 ADC |
| **Komora pomiarowa** | Druk 3D PETG, ścieżka 10mm | Własna drukarka / Printuj3D.pl | 1 | 10 zł | 10 zł | 🔴 KUP | Plik STL do stworzenia |
| **Rezystor 10 kΩ** | 1/4W, THT, pull-up fotodioda | Botland | 2 | 0.10 zł | 0.20 zł | 🔴 KUP | Jeden na każdy kanał |
| **Rezystor 100 Ω** | 1/4W, THT, ogranicznik LED | Botland | 4 | 0.10 zł | 0.40 zł | 🔴 KUP | Dwa na LED (×2 kanały) |

> **Dlaczego ADS1115 zamiast wbudowanego ADC ESP32?**
> ESP32 ADC ma tylko 12-bit i jest nieliniowy przy aktywnym WiFi. ADS1115 ma 16-bit
> i komunikuje się przez I2C – brak konfliktu z WiFi. Różnica w dokładności: ~0.001 AU
> vs ~0.01 AU → kluczowe dla osiągnięcia ±10 ppm Ca.

### Blok fluidyczny – sterowanie przepływem

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **Pompka perystaltyczna 12V** | ~1 ml/s, przezroczysta rurka silikonowa | Allegro / AliExpress | 3 | 15 zł | 45 zł | 🔴 KUP | Próbka + reagent Ca + reagent Mg. 4. pompka (spłukiwanie) jeśli chcesz dedykowaną |
| **MOSFET DFRobot Gravity** | 5–36V/20A, DFR-08104 | Botland / DFRobot | 3 | 18 zł | 54 zł | 🔴 KUP | Jeden na pompkę |
| **Rurka silikonowa 2mm** | ID 2mm, chemoodporna | AliExpress / laboratoria | ~50 cm | – | ~8 zł | 🔴 KUP | Odporność na Arsenazo III i Calmagite |
| **Złączka Y 2mm** | Rozgałęzienie 1→2, PP lub PTFE | AliExpress | 2 | 2 zł | 4 zł | 🔴 KUP | Łączenie kanałów reagentów do komory |
| **Butelka 50 ml + korek** | Ciemna (brązowa), szkło lub HDPE | Apteka / laboratorium | 2 | 2.50 zł | 5 zł | 🔴 KUP | Przechowywanie Arsenazo III i Calmagite |
| **Butelka 500 ml** | HDPE, zakrętka | Apteka / Castorama | 1 | 3 zł | 3 zł | 🔴 KUP | Woda RO – spłukiwanie komory |
| **Pojemnik na odpady** | ~100 ml, zakrętka | Sklep lokalny | 1 | 3 zł | 3 zł | 🔴 KUP | Zbiera zużyte roztwory |

### Reagenty chemiczne

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **Arsenazo III** | 0.05% w wodzie dest., 100 ml | Sklep chemiczny / Allegro | 1 | 35 zł | 35 zł | 🔴 KUP | ⚠️ TOKSYCZNY (GHS06) – rękawiczki! |
| **Calmagite** | 0.05% w wodzie dest. + pH 10 bufor, 100 ml | Sklep chemiczny / Allegro | 1 | 25 zł | 25 zł | 🔴 KUP | ⚠️ Drażniący (GHS07) |
| **Bufor pH 10** | Amonowy, 100 ml | Sklep chemiczny / Allegro | 1 | 15 zł | 15 zł | 🔴 KUP | Wymagany dla pomiaru Mg |
| **Roztwory wzorcowe Ca/Mg** | NSW standard lub sztuczny | Własne / sklep akwarystyczny | 2 porcje | – | ~10 zł | 🔴 KUP | Do kalibracji 2-punktowej |

> ⚠️ **Bezpieczeństwo chemiczne:**
> Arsenazo III – GHS06 (toksyczny). Calmagite – GHS07 (drażniący).
> Praca wyłącznie w rękawiczkach nitrylowych i okularach ochronnych.
> Przechowywanie: ciemne butelki, 4–8°C, z dala od żywności.
> Termin ważności: Arsenazo III 6 mies. od otwarcia, Calmagite 12 mies.

### Narzędzia i materiały montażowe

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **Breadboard 400-hole** | Płytka stykowa | Botland | 1 | 8 zł | 8 zł | 🔴 KUP | |
| **Przewody Dupont** | F-F, M-F, M-M 20cm | Botland | 1 kpl | 8 zł | 8 zł | 🔴 KUP | |
| **Klej silikonowy** | Przezroczysty, wodoodporny | Castorama / Leroy | 1 | 8 zł | 8 zł | 🔴 KUP | Uszczelnianie połączeń rurkowych i komory |

---

## 📌 GPIO Assignment – Sentinel Photometer

| Funkcja | GPIO | Interfejs | Uwagi |
|---------|------|-----------|-------|
| ADS1115 SDA | GPIO21 | I2C | Adres 0x48 (ADDR→GND) |
| ADS1115 SCL | GPIO22 | I2C | 100 kHz |
| LED 650nm (Ca) | GPIO25 | Digital | Przez rezystor 100Ω |
| LED 520nm (Mg) | GPIO26 | Digital | Przez rezystor 100Ω |
| Pompa próbki | GPIO15 | MOSFET | 12V |
| Pompa barwnika Ca | GPIO4 | MOSFET | 12V, dozowanie µl |
| Pompa barwnika Mg | GPIO16 | MOSFET | 12V, dozowanie µl |
| Pompa spłukiwania | GPIO17 | MOSFET | 12V, woda RO |
| Status LED | GPIO2 (built-in) | Digital | Wbudowana |

**GPIO użytych: 9 z ~25 dostępnych ✅**

> GPIO32–39 (ADC1) zarezerwowane dla Modułu 2 (Chem/Monitor) jeśli działają równolegle.
> Photometer używa ADS1115 przez I2C – brak konfliktu ADC/WiFi.

---

## 💰 PODSUMOWANIE KOSZTÓW

### Pełny BOM (wszystko do kupienia)

| Kategoria | Koszt |
|-----------|-------|
| ESP32 DevKit | 25 zł |
| Zasilacz 12V/2A | 20 zł |
| LM2596 Buck Converter | 8 zł |
| Kondensatory (100µF ×3, 100nF ×5) | 4 zł |
| **Blok optyczny** (LED ×4, BPW34 ×2, ADS1115, komora, rezystory) | ~40 zł |
| **Blok fluidyczny** (pompki ×3, MOSFET ×3, rurki, butelki) | ~117 zł |
| **Reagenty** (Arsenazo III, Calmagite, bufor pH 10, wzorce) | ~85 zł |
| Breadboard + Dupont + klej silikonowy | ~24 zł |
| **RAZEM** | **~323 zł** |

> Koszt reagentów (~85 zł) jest kosztem jednorazowym – wystarczają na ~12 miesięcy
> przy 1 teście dziennie (pobór ~0.5 ml/test × 365 testów = ~182 ml/rok).

### Szacowany koszt urządzenia gotowego (przy sprzedaży)

| Kategoria | Koszt |
|-----------|-------|
| Komponenty (BOM) | ~323 zł |
| Montaż | ~40 zł |
| Kontrola jakości + kalibracja fabryczna | ~25 zł |
| Pakowanie | ~15 zł |
| **KOSZT CAŁKOWITY / szt.** | **~403 zł** |

**Sugerowana cena sprzedaży:** 550 zł
**Marża:** ~147 zł (26.7%)

---

## 🛒 LISTA ZAKUPÓW – GDZIE KUPIĆ

### Botland (botland.com.pl) – elektronika podstawowa
- ESP32 DevKit 30-pin
- LM2596 Buck Converter
- Fotodioda BPW34 ×2
- ADS1115 moduł ×1
- Kondensatory 100µF/25V ×3
- Kondensatory 100nF ×5
- Rezystory 10kΩ ×2, 100Ω ×4
- MOSFET DFRobot DFR-08104 ×3
- Przewody Dupont (zestaw)
- Breadboard 400-hole

### AliExpress – komponenty optyczne i rurki
- LED 650nm 5mm narrow beam ×5 (zapas)
- LED 520nm 5mm narrow beam ×5 (zapas)
- Złączki Y 2mm PP/PTFE ×5 (zapas)
- Rurka silikonowa 2mm ID, 1m

### Allegro / lokalne sklepy chemiczne
- Zasilacz 12V/2A
- Pompka perystaltyczna 12V ×3
- Arsenazo III 0.05% 100ml (szukaj: "arsenazo III aquarium" lub sklep odczynników)
- Calmagite 0.05% 100ml
- Bufor pH 10, 100ml

### Apteka / sklep laboratoryjny
- Butelki 50ml ciemne (brązowe) ×2
- Butelka 500ml HDPE ×1

### Castorama / Leroy Merlin
- Klej silikonowy przezroczysty, wodoodporny

### Własna drukarka 3D lub Printuj3D.pl
- Komora pomiarowa PETG (plik STL do stworzenia)
- Materiał: PETG (odporność chemiczna lepsza niż PLA)

---

## ⚠️ KRYTYCZNE UWAGI BEZPIECZEŃSTWA

```
PRZED PIERWSZYM WŁĄCZENIEM – CHECKLIST:

☐ LM2596 ustawiony na DOKŁADNIE 5.0V (multimetrem!)
   → Wyższe napięcie = natychmiastowa śmierć ESP32

☐ ADS1115: kondensator 100nF między VDD a GND blisko układu
   → Bez decoupling szumy zasilania przekłamują odczyty absorpcji

☐ Komora pomiarowa: test szczelności wodą RO pod ciśnieniem pompki
   → Wyciek barwnika = zabarwienie i zanieczyszczenie akwarium

☐ PIERWSZA pompka: test z wodą RO, NIE z reagentem
   → Sprawdź kierunek przepływu i szczelność złączek Y

☐ Arsenazo III (GHS06): rękawiczki nitrylowe + okulary ZAWSZE
   → Chronić skórę i oczy – substancja toksyczna

☐ Butelki z barwnikami: ciemne, w lodówce (4–8°C), z dala od żywności
   → Degradacja pod wpływem światła i ciepła → błędne wyniki
```

---

## 🔗 POWIĄZANE DOKUMENTY

- `BUILD_GUIDE_sentinel_photometer.md` – instrukcja montażu krok po kroku
- `reef_hub_chem_monitor_BOM.md` – BOM dla Modułów 1+2
- `BOM_sentinel_view.md` – BOM dla Modułu 4
- `firmware/sentinel_photometer.yaml` *(do stworzenia)*

---

*Reef Sentinel Lab – Open-source aquarium controller*
*reef-sentinel.com | github.com/reef-sentinel*
*Ostatnia aktualizacja: 2026-03-07*
