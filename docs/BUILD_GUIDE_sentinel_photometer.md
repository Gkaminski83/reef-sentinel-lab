## English (Primary)

# Reef Sentinel Lab - Build Guide
## Module 3: Sentinel Photometer (Ca + Mg)

Version: 0.1 (framework)
Date: 2026-03-06
Status: planned for Q3 2026
Difficulty: advanced

---

## Scope

This document defines the design framework for automated Ca/Mg photometry.
Detailed assembly and firmware instructions are scheduled for Q3 2026.

---

## Measurement Principle

Based on Beer-Lambert law:
- Compare blank light intensity (I0) and sample intensity (I)
- Compute absorbance
- Convert absorbance to concentration using calibration curves

Target channels:
- Ca channel: ~650nm LED with Ca reagent chemistry
- Mg channel: ~520nm LED with Mg reagent chemistry

---

## Proposed Hardware Architecture
- ESP32 controller
- ADS1115 (16-bit ADC) for photodiode precision
- dual LED optical path (650nm/520nm)
- photodiode receiver
- shared optical chamber/cuvette
- pump-driven fluid handling (sample, reagents, drain/flush)

---

## Preliminary BOM Themes
- ESP32 + LM2596 power stage
- ADS1115 + passives
- narrow-angle LEDs + photodiode
- 12V pumps + MOSFET control channels
- light-tight 3D-printed optical enclosure
- reagent containers and waste path

---

## Planned Sequence
1. RO flush and blank measurement
2. Ca sample + reagent reaction + reading
3. drain/flush
4. Mg sample + reagent reaction + reading
5. publish results via ESPHome API

---

## Calibration Plan
- multi-point standards for Ca and Mg
- empirical curve fit for each channel
- periodic recalibration (new reagent lot / drift checks)

---

## Known Risks
- optical leakage into chamber
- temperature effect on color reactions
- bubbles in optical path
- reagent stability over time

---

## Integration
- publish to Hub Home Assistant entities for Ca/Mg
- expose values to HA and Sentinel View

---

## Polish (PL)
# Reef Sentinel Lab – BUILD GUIDE
## Module 3: Sentinel Photometer (Fotometr Ca + Mg)

> **Version:** 0.1 – SZKIELET  
> **Data:** 2026-03-06  
> **Status:** ⏸️ Faza 2 – Q3 2026  
> **Poziom trudności:** ⭐⭐⭐⭐☆ (Zaawansowany – optyka + chemia)  
> **Wymaganie wstępne:** Sentinel Hub + Sentinel Chem/Monitor działające i stabilne

---

> **đź"‹ NOTA O TYM DOKUMENCIE**  
> To jest szkielet build guide – zawiera architekturę, koncepcję i kluczowe decyzje projektowe.  
> Szczegółowe instrukcje montażu i firmware zostaną uzupełnione w Q3 2026,  
> gdy moduł wejdzie w fazę aktywnego rozwoju.  
> Sekcje oznaczone `[ TODO ]` czekają na uzupełnienie.

---

## CO TEN MODUŁ ROBI

Sentinel Photometer to fotometryczny analizator wapnia (Ca) i magnezu (Mg) – dwa z trzech najważniejszych parametrów chemicznych akwarium morskiego.

Razem z Sentinel Chem/Monitor (KH/pH) tworzy kompletny system **BIG THREE**:

```
Sentinel Chem/Monitor  →  KH (titracja HCl)     ← automatyczne
                       →  pH (monitoring ciągły) ← automatyczne
Sentinel Photometer    →  Ca (fotometria)        ← automatyczne
                       →  Mg (fotometria)        ← automatyczne

RĘCZNIE (rzadziej):    →  NO₃ (raz w tygodniu)
                       →  PO₄ (Milwaukee – masz)
```

**Wartość dla użytkownika:**  
Typowy reef keeper testuje Ca i Mg 2–3× w tygodniu (Salifert/Red Sea, ~10 min/test).  
Sentinel Photometer robi to automatycznie w ~12 min, bez udziału człowieka.

**Porównanie z konkurencją:**

| | Reef Sentinel Lab (v1+v3) | Neptune Trident | Hanna Checker (ręczny) |
|--|--------------------------|-----------------|------------------------|
| KH/Ca/Mg | ✅ Automatyczne | ✅ Automatyczne | ❌ Ręczny |
| Cena hardware | ~900 zł | ~2500 zł | ~600 zł (3 checkersy) |
| Reagenty/mies. | ~50 zł DIY | ~200 zł (zamknięte) | ~150 zł |
| Open-source | ✅ Tak | ❌ Nie | ❌ Nie |

---

## ZASADA DZIAŁANIA – FOTOMETRIA

Sentinel Photometer wykorzystuje prawo Beera-Lamberta:

```
A = ε × c × l

A = absorbancja (ile światła pochłonięte)
ε = współczynnik ekstynkcji (stały dla danej reakcji chemicznej)
c = stężenie (szukana wartość: Ca lub Mg w ppm)
l = długość drogi optycznej (grubość kuwety, np. 10mm)
```

**W praktyce:**

```
Krok 1: BLANK
  LED świeci przez czystą wodę RO → fotodioda mierzy I₀ (100% światła)

Krok 2: PRÓBKA
  Dodaj odczynnik do próbki wody z akwarium
  Woda zmienia kolor (intensywność zależy od stężenia Ca/Mg)
  LED świeci przez zabarwioną próbkę → fotodioda mierzy I (mniej światła)

Krok 3: OBLICZENIE
  Absorbancja = log₁₀(I₀ / I)
  Stężenie = f(Absorbancja)  ← z krzywej kalibracyjnej
```

---

## ARCHITEKTURA SPRZĘTU

### Schemat blokowy

```
                    SENTINEL HUB
                         │
                      WiFi ESPHome API
                         │
                    ESP32 (bridge)
                         │
              ┌──────────┴──────────┐
              │                     │
         Kanał Ca               Kanał Mg
              │                     │
    ┌─────────┴─────┐     ┌─────────┴─────┐
    │  LED 650nm    │     │  LED 520nm    │
    │  (czerwony)   │     │  (zielony)    │
    └───────┬───────┘     └───────┬───────┘
            │                     │
            ▼                     ▼
    ┌───────────────────────────────────┐
    │      KOMORA POMIAROWA             │
    │      (wspólna, ~10ml)             │
    │                                   │
    │  Kuweta szklana 10mm              │
    │  + próbka + odczynnik             │
    │                                   │
    └───────────┬───────────────────────┘
                │
    ┌───────────┴───────────┐
    │     Fotodioda         │
    │     (BPW34 lub equiv) │
    └───────────┬───────────┘
                │
    ┌───────────┴───────────┐
    │   ADS1115 (16-bit ADC)│  ← precyzja lepsza niż wbudowane ADC ESP32
    └───────────┬───────────┘
                │ I2C
    ┌───────────┴───────────┐
    │       ESP32           │
    └───────────────────────┘
```

### Dlaczego ADS1115 zamiast wbudowanego ADC ESP32?

Wbudowane ADC ESP32 ma rozdzielczość 12-bit i jest nieliniowe przy brzegach zakresu.  
Fotometria wymaga rozróżnienia bardzo małych różnic napięcia (~1–5mV) odpowiadających różnicom stężenia ±10 ppm.  
ADS1115 daje 16-bit, liniowość ±0.1% i wzmacniacz PGA – wystarczające dla wymaganej dokładności Ca ±10 ppm, Mg ±30 ppm.

---

## KOMPONENTY (wstępna lista)

### Elektronika

| Komponent | Model/Spec | Szacunkowa cena | Uwagi |
|-----------|-----------|-----------------|-------|
| ESP32 DevKit | ESP-WROOM-32, 30-pin | 25 zł | Jak w innych modułach |
| LM2596 | 12V→5V buck | 8 zł | Jak w innych modułach |
| ADS1115 | 16-bit ADC, I2C | 15 zł | Precyzyjny odczyt fotodiody |
| LED 650nm | Czerwony, 5mm, wąski kąt (< 15°) | 2 zł | Kanał Ca (arsenazo III) |
| LED 520nm | Zielony, 5mm, wąski kąt (< 15°) | 2 zł | Kanał Mg (calmagite) |
| Fotodioda | BPW34 lub VEMT2023 | 5 zł | Czuła na 400–1100nm |
| Rezystor 1kΩ | Dla LED (ogranicznik prądu) | 0.10 zł × 2 | |
| Rezystor 10kΩ | Dla fotodiody (transimpedancja) | 0.10 zł | |
| Pompka perystaltyczna 12V | Próbka + odczynniki | 26 zł × 3 | Próbka, odczynnik Ca, odczynnik Mg |
| MOSFET DFRobot | 5-36V/20A | 18 zł × 3 | Sterowanie pompek |

**Szacunkowy koszt elektroniki: ~150 zł**

### Optyka i mechanika

| Komponent | Spec | Szacunkowa cena | Uwagi |
|-----------|------|-----------------|-------|
| Kuweta szklana | 10mm droga optyczna, ~3ml | 20 zł | **Musi być szklana (nie plastik!)** |
| Obudowa optyczna | Czarne PLA/ABS, druk 3D | 15 zł | Całkowicie światłoszczelna |
| Kolimator LED | Soczewka lub tunel 5mm | 5 zł | Skupia światło LED na kuwecie |
| Uchwyt kuwety | Druk 3D | 5 zł | Precyzyjna pozycja kuwety |

**Szacunkowy koszt mechaniki: ~45 zł**

### Reagenty

**Opcja A – Hanna Checker (komercyjne, wygodne):**

| Reagent | Model Hanna | Cena | Koszt/test | Testy/butelka |
|---------|------------|------|-----------|---------------|
| Wapń (Ca) | HI758 | ~150 zł | ~6 zł | 25 testów |
| Magnez (Mg) | HI719 | ~120 zł | ~4.80 zł | 25 testów |
| **Razem/test** | | | **~10.80 zł** | |

**Opcja B – DIY reagenty (open-source, tańsze ~75%):**

| Reagent | Składniki | Koszt/test | Uwagi |
|---------|-----------|-----------|-------|
| Wapń (Ca) | Arsenazo III + bufor octanowy pH 6 | ~1.50 zł | Metoda spektrofotometryczna |
| Magnez (Mg) | Calmagite + bufor amonowy pH 10 | ~1.20 zł | Metoda kompleksometryczna |
| **Razem/test** | | **~2.70 zł** | 75% taniej niż Hanna! |

> **Rekomendacja na start:** Użyj odczynników Hanna do kalibracji i weryfikacji systemu.  
> Gdy system działa poprawnie, przejdź na DIY reagenty.

**Pojemniki na reagenty:**
- 2× butelka ~100ml (Ca odczynnik, Mg odczynnik)  
- 1× kanister 5L odpad  
- 1× kanister 5L woda RO (płukanie)

---

## GPIO ASSIGNMENT (wstępny)

| GPIO | Funkcja | Typ |
|------|---------|-----|
| GPIO21 | ADS1115 SDA | I2C |
| GPIO22 | ADS1115 SCL | I2C |
| GPIO26 | LED 650nm (Ca) | Digital OUT |
| GPIO27 | LED 520nm (Mg) | Digital OUT |
| GPIO15 | Pompka próbka | MOSFET SIG |
| GPIO4 | Pompka odczynnik Ca | MOSFET SIG |
| GPIO16 | Pompka odczynnik Mg | MOSFET SIG |
| GPIO17 | Pompka odpad/RO | MOSFET SIG |

> GPIO może ulec zmianie w trakcie projektowania PCB.

---

## SEKWENCJA POMIARU Ca + Mg (~12 min)

```
START
  │
  ▼
1. Płukanie komory wodą RO (pompka RO, 30s)
  │
  ▼
2. Pomiar BLANK:
   LED 650nm ON → odczyt I₀_Ca (referencja)
   LED 520nm ON → odczyt I₀_Mg (referencja)
   Oba LED OFF
  │
  ▼
3. Pobierz 5ml próbki z akwarium (pompka próbka, 10s)
  │
  ▼
  ├──── TEST Ca (5 min) ────────────────────────────────────┐
  │     4. Dodaj 0.5ml odczynnika Ca (Arsenazo III)          │
  │     5. Czekaj 3 min (reakcja chemiczna)                  │
  │     6. LED 650nm ON → odczyt I_Ca                        │
  │     7. Oblicz Ca: A_Ca = log₁₀(I₀_Ca / I_Ca)            │
  │                   Ca [ppm] = f(A_Ca) ← krzywa kalib.    │
  │     8. LED 650nm OFF                                     │
  └──────────────────────────────────────────────────────────┘
  │
  ▼
9. Wypompuj próbkę (pompka odpad, 30s)
10. Płukanie RO × 2 (30s każde)
  │
  ▼
11. Pobierz 5ml świeżej próbki (pompka próbka, 10s)
  │
  ▼
  ├──── TEST Mg (5 min) ────────────────────────────────────┐
  │     12. Dodaj 0.5ml odczynnika Mg (Calmagite)            │
  │     13. Czekaj 3 min (reakcja chemiczna)                 │
  │     14. LED 520nm ON → odczyt I_Mg                       │
  │     15. Oblicz Mg: A_Mg = log₁₀(I₀_Mg / I_Mg)           │
  │                    Mg [ppm] = f(A_Mg) ← krzywa kalib.   │
  │     16. LED 520nm OFF                                    │
  └──────────────────────────────────────────────────────────┘
  │
  ▼
17. Finalne płukanie RO × 3 (30s każde)
18. Wyślij wyniki przez ESPHome API → Sentinel Hub
    {ca: 425, mg: 1340, unit: "ppm", timestamp: ...}

KONIEC (~12 min total)
```

---

## PROJEKT KOMORY OPTYCZNEJ

Komora musi być **całkowicie światłoszczelna** – nawet mały wyciek zewnętrznego światła fałszuje odczyty.

```
Widok z góry (przekrój):

     [LED 650nm]  [LED 520nm]
           │             │
           ▼             ▼
    ┌──────────────────────────────────┐
    │  tunel          tunel            │
    │  kolimujący     kolimujący       │
    │  (5mm dia)      (5mm dia)        │
    │       │               │          │
    │       ▼               ▼          │
    │  ┌─────────────────────────┐     │  ← czarne PLA/ABS, druk 3D
    │  │   KUWETA SZKLANA 10mm  │     │
    │  │   próbka z odczynnikiem │     │
    │  └──────────┬──────────────┘     │
    │             │                    │
    │             ▼                    │
    │        [Fotodioda]               │
    │        BPW34                     │
    └──────────────────────────────────┘

Wymiary orientacyjne:
- Kuweta: 10mm ścieżka optyczna, ~3ml
- LED → kuweta: 10–15mm (z kolimatoremm)
- Kuweta → fotodioda: 5–10mm
- Całość: ~50 × 30 × 30mm
```

**Kluczowe zasady projektowe:**
1. Czarny materiał wewnątrz (PLA czarny lub malowanie matową czarną farbą)
2. LED z wąskim kątem świecenia (< 15°) lub tunel kolimujący
3. Kuweta szklana (plastik przepuszcza IR, szkło nie – ważne dla dokładności)
4. Fotodioda prostopadle do osi LED (minimalizacja odbić)
5. Uszczelnienie O-ring wokół kuwety (żeby odczynnik nie wyciekał)

---

## KALIBRACJA [ TODO – szczegóły Q3 2026 ]

### Metoda kalibracyjna

Krzywa kalibracyjna wymaga roztworów wzorcowych o znanych stężeniach Ca i Mg.

**Roztwory wzorcowe Ca:**
```
Standard 1: 300 ppm Ca (poniżej normy reefowej)
Standard 2: 420 ppm Ca (typowa wartość reefowa)
Standard 3: 500 ppm Ca (powyżej normy)
```

**Roztwory wzorcowe Mg:**
```
Standard 1: 1100 ppm Mg
Standard 2: 1350 ppm Mg  
Standard 3: 1500 ppm Mg
```

> Roztwory wzorcowe można przygotować z:
> - Chlorek wapnia (CaCl₂) – reagent klasy analitycznej
> - Chlorek magnezu (MgCl₂) – reagent klasy analitycznej
> - Lub zakupić gotowe standardy (~50 zł/500ml)

**Wynik kalibracji:** równanie liniowe lub wielomianowe:
```
Ca [ppm] = a × Absorbancja + b
```
Współczynniki `a` i `b` wpisujesz do firmware.

### Częstotliwość rekalibracji

- Po każdej nowej partii odczynników
- Co 3 miesiące (drift fotodiody)
- Po mechanicznej ingerencji w komorę optyczną

---

## FIRMWARE [ TODO – szczegóły Q3 2026 ]

### Planowane funkcje

```yaml
# Szkielet ESPHome – do uzupełnienia
esphome:
  name: sentinel-photometer

esp32:
  board: esp32dev

wifi:
  ssid: "SentinelHub"
  password: "reef1234"

# MQTT removed in no-broker architecture
# Data path: ESPHome API via Home Assistant
# Brak lokalnego broker/client bus na module.
# Publikacja pomiarów przez encje ESPHome -> Home Assistant.

# ADS1115 (16-bit ADC)
i2c:
  sda: GPIO21
  scl: GPIO22

sensor:
  - platform: ads1115
    multiplexer: 'A0_GND'
    gain: 6.144
    name: "Fotodioda Raw"
    id: photodiode_raw
    update_interval: never  # Tylko podczas pomiaru

# LEDs
switch:
  - platform: gpio
    pin: GPIO26
    name: "LED 650nm Ca"
    id: led_ca

  - platform: gpio
    pin: GPIO27
    name: "LED 520nm Mg"
    id: led_mg

# [ TODO ] Algorytm pomiaru:
# - pomiar blank
# - pompowanie próbki
# - dodanie odczynnika
# - czas reakcji (timer)
# - pomiar absorbancji
# - obliczenie stężenia (krzywa kalibracyjna)
# - wysłanie przez ESPHome API
```

---

## INTEGRACJA Z SYSTEMEM

Sentinel Photometer dołącza do istniejącej sieci ESPHome API jako kolejny klient:

```
Sentinel Hub (Home Assistant bridge 10.42.0.1)
       │
       ├── reef/chem/ph        ← Sentinel Chem/Monitor
       ├── reef/chem/kh        ← Sentinel Chem/Monitor
       ├── reef/chem/temp      ← Sentinel Chem/Monitor
       ├── reef/chem/ec        ← Sentinel Chem/Monitor
       │
       ├── reef/photo/ca       ← Sentinel Photometer (NOWY)
       └── reef/photo/mg       ← Sentinel Photometer (NOWY)
```

Sentinel View automatycznie wyświetli Ca i Mg po dodaniu odpowiednich etykiet do UI Nextion.

---

## HARMONOGRAM ROZWOJU

```
Q2 2026 (TERAZ):
  ✅ Sentinel Hub
  ✅ Sentinel Chem/Monitor
  ⏸️ Sentinel View (do uruchomienia)

Q3 2026 (Sentinel Photometer v0.1):
  [ ] Projekt komory optycznej (FreeCAD/Fusion360)
  [ ] Druk 3D prototyp komory
  [ ] Test optyki (blank measurement)
  [ ] Kalibracja 3-punktowa Ca
  [ ] Kalibracja 3-punktowa Mg
  [ ] Firmware ESPHome (algorytm pomiaru)
  [ ] Integracja z Hub (ESPHome API)
  [ ] Testy na wodzie akwaryjnej (porównanie z Salifert)

Q4 2026 (Sentinel Photometer v1.0):
  [ ] Stabilność długoterminowa (1 miesiąc testów)
  [ ] DIY reagenty (arsenazo III, calmagite)
  [ ] Automatyczna rekalibracja blank
  [ ] Dokumentacja pełna BUILD_GUIDE
  [ ] Integracja z Sentinel Photometer BOM
```

---

## ZNANE WYZWANIA I RYZYKA

### Wyzwanie 1: Światłoszczelność komory

Zewnętrzne światło (szczególnie przy akwarium z lampami) to największy wróg fotometrii.  
Nawet 1% zewnętrznego światła może powodować błąd ~10 ppm.

**Mitygacja:** Czarne PLA + test w całkowitej ciemności przed pierwszym uruchomieniem.

### Wyzwanie 2: Temperatura próbki

Reakcje kolorometryczne są wrażliwe na temperaturę (±2°C = ±5 ppm błąd Ca).  
DS18B20 z Sentinel Chem/Monitor może mierzyć temperaturę komory.  
Kompensacja temperaturowa musi być wbudowana w algorytm.

### Wyzwanie 3: Bąbelki powietrza w kuwecie

Bąbelek powietrza blokuje wiązkę LED → wynik zaniżony lub błędny.  
**Mitygacja:** Mieszadełko magnetyczne (stir bar) w komorze – identyczne jak w Sentinel Chem.

### Wyzwanie 4: Kompatybilność odczynników

Odczynniki Hanna są zaprojektowane dla ich własnych fotometrów.  
W DIY fotometrze możliwe różnice długości drogi optycznej (kuweta vs Hanna checker).  
**Mitygacja:** Kalibracja empiryczna (nie teoretyczna) – zawsze weryfikacja z Salifert.

### Wyzwanie 5: Żywotność odczynników

Arsenazo III i Calmagite rozkładają się w świetle i w podwyższonej temperaturze.  
**Mitygacja:** Przechowywanie w ciemnych butelkach z brązowego szkła, w temperaturze pokojowej.

---

## OTWARTE PYTANIA (do rozstrzygnięcia w Q3 2026)

```
[ ] Czy używamy wspólnej kuwety dla Ca i Mg (sekwencyjnie)
    czy dwóch osobnych kuwet (równolegle)?
    → Wspólna: tańsza, wolniejsza (płukanie między testami)
    → Dwie osobne: droższe, szybsze (~6 min zamiast ~12 min)

[ ] Odczynniki: Hanna (gotowe) czy DIY (tańsze) jako domyślne?
    → Na start Hanna (weryfikacja systemu)
    → DIY po walidacji

[ ] Czy dodać DS18B20 w komorze dla kompensacji temperatury?
    → Raczej TAK – koszty minimalne, zysk dokładności duży

[ ] Czy Sentinel Photometer ma własny ESP32
    czy współdzieli z Sentinel Chem/Monitor?
    → Własny ESP32 (modularność systemu, prostszy firmware)

[ ] Jaka częstotliwość testów Ca/Mg?
    → Domyślna propozycja: 1× dziennie (o 09:00)
    → Konfigurowalna: 1–7× na tydzień
```

---

## POWIĄZANE DOKUMENTY

- [`sentinel_hub_chem_monitor_BOM.md`](./sentinel_hub_chem_monitor_BOM.md) – BOM Module 1+2
- [`BUILD_GUIDE_sentinel_chem_monitor.md`](./BUILD_GUIDE_sentinel_chem_monitor.md) – Module 2 (wymaganie)
- `sentinel_photometer_BOM.md` *(do stworzenia – Q3 2026)*
- `CHEMISTRY_GUIDE.md` *(do stworzenia – reagenty DIY, przygotowanie standardów)*

---

*Reef Sentinel Lab – Open-source aquarium controller*  
*reef-sentinel.com | github.com/reef-sentinel*  
*Ostatnia aktualizacja: 2026-03-06*

