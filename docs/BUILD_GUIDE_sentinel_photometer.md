## English (Primary)

# Reef Sentinel – Build Guide
## Module 3: Sentinel Photometer

Version: 0.1 (skeleton)
Date: 2026-03-07
Planned build start: Q3 2026
Estimated build time: 5–8 hours
Difficulty: Advanced
Requires: Modules 1 + 2 operational

---

## Purpose

Sentinel Photometer measures calcium (Ca) and magnesium (Mg) using photometric absorption
(Beer-Lambert law). The same principle used in ICP-OES labs, adapted for ESP32.
One test cycle takes ~8 minutes and runs automatically once daily or on-demand from Hub panel.

| Parameter | Reagent       | LED wavelength     | Target accuracy |
|-----------|---------------|--------------------|-----------------|
| Ca        | Arsenazo III  | 650 nm (red)       | ±10 ppm         |
| Mg        | Calmagite     | 520 nm (green)     | ±30 ppm         |

---

## Required Hardware

### Optical block
- LED 650 nm, 5mm, narrow beam ×2
- LED 520 nm, 5mm, narrow beam ×2
- BPW34 photodiode ×2
- ADS1115 16-bit ADC (I2C, 4-channel) ×1
- Measurement cell (3D print PETG, 10mm path length) ×1
- Resistor 10 kΩ (photodiode pull-up) ×2
- Resistor 100 Ω (LED limiter) ×4

### Fluidic block
- Peristaltic pump 12V ~1 ml/s ×3
- DFRobot Gravity MOSFET driver ×3
- Silicone tubing 2mm ID ×50 cm
- Y-fitting 2mm ×2
- 50 ml bottle with cap (Arsenazo III) ×1
- 50 ml bottle with cap (Calmagite) ×1
- 500 ml bottle (RO water flush) ×1

### Electronics & power
- ESP32 DevKit 30-pin (ESP-WROOM-32)
- LM2596 Buck Converter (12V→5V with display)
- 12V/2A power supply (dedicated)
- Capacitor 100µF/25V ×3
- Capacitor 100nF ×5
- Breadboard 400-hole
- Dupont jumper wire set

---

## Safety First

- Never connect USB and LM2596 power simultaneously.
- Tune LM2596 to exactly 5.0V before connecting ESP32.
- Arsenazo III is toxic (GHS06). Always use nitrile gloves and eye protection.
- Calmagite is irritant (GHS07). Same precautions.
- Test all pumps with RO water before introducing reagents.
- Store reagents in dark glass bottles at 4–8°C, away from food.

---

## GPIO Assignment

| Function          | GPIO       | Interface | Notes                         |
|-------------------|------------|-----------|-------------------------------|
| ADS1115 SDA       | GPIO21     | I2C       | Address 0x48 (ADDR→GND)       |
| ADS1115 SCL       | GPIO22     | I2C       | 100 kHz                       |
| LED 650nm (Ca)    | GPIO25     | Digital   | Via 100Ω resistor             |
| LED 520nm (Mg)    | GPIO26     | Digital   | Via 100Ω resistor             |
| Pump sample       | GPIO15     | MOSFET    | 12V pulses                    |
| Pump reagent Ca   | GPIO4      | MOSFET    | 12V µl dosing                 |
| Pump reagent Mg   | GPIO16     | MOSFET    | 12V µl dosing                 |
| Pump flush        | GPIO17     | MOSFET    | 12V RO water                  |
| Status LED        | GPIO2      | Digital   | Built-in                      |

> ADC1 (GPIO32–39) reserved for Module 2. Photometer uses ADS1115 via I2C – no ADC conflict.

---

## Build Steps

### 1. Power stage
1. Connect 12V supply to LM2596 (IN+, IN–).
2. Set multimeter to DC V, range 20V.
3. Adjust potentiometer until OUT+ reads 5.0V ±0.1V.
4. Power off before connecting ESP32.
5. Connect: LM2596 OUT+ → ESP32 VIN, OUT– → GND.

### 2. ADS1115 wiring
- VDD → 3V3
- GND → GND
- SDA → GPIO21
- SCL → GPIO22
- ADDR → GND (sets I2C address 0x48)
- Add 100nF capacitor between VDD and GND, close to chip.

### 3. Optical block
- LED 650nm: anode (+) → GPIO25 via 100Ω resistor, cathode (–) → GND.
- LED 520nm: anode (+) → GPIO26 via 100Ω resistor, cathode (–) → GND.
- BPW34 photodiodes: anode → ADS1115 A0 (Ca) and A1 (Mg), cathode → GND.
- 10kΩ pull-up on each ADS1115 input channel.

### 4. Fluidic block
- MOSFET SIG → GPIO (per table above), VCC → 3V3, GND → GND.
- MOSFET OUT+ → pump (+), pump (–) → 12V GND.
- Route tubing: sample pump → cell input, reagent pumps → cell input (via Y-fitting), flush pump → cell input.
- Cell output → waste container.

### 5. Measurement cell
- Print PETG cell with 10mm optical path length.
- Mount LEDs and photodiodes facing each other through the cell.
- Seal tubing connections with silicone adhesive.
- Test water-tightness with RO water before introducing reagents.

---

## Measurement Protocol

### Ca sequence (Arsenazo III)
1. Sample pump: draw 2 ml aquarium water into cell.
2. Reagent pump: add 0.5 ml Arsenazo III (0.05% concentration).
3. Wait 60 s – colour reaction (solution turns blue proportional to Ca).
4. LED 650nm ON → read ADS1115 Ch0 → record absorbance A.
5. Calculate: `Ca [ppm] = (A – A_blank) / K_Ca`
6. Flush pump: 3× flush cell with RO water.
7. LED 650nm OFF.

### Mg sequence (Calmagite)
1. Sample pump: draw 2 ml aquarium water into cell.
2. Reagent pump: add 0.5 ml Calmagite + 0.1 ml pH 10 buffer.
3. Wait 45 s – colour reaction (solution turns red proportional to Mg).
4. LED 520nm ON → read ADS1115 Ch1 → record absorbance A.
5. Calculate: `Mg [ppm] = (A – A_blank) / K_Mg`
6. Flush pump: 3× flush cell with RO water.
7. LED 520nm OFF.

---

## Calibration

### 2-point calibration (recommended)

| Solution      | Ca [ppm] | Mg [ppm] | Source                         |
|---------------|----------|----------|--------------------------------|
| Standard LOW  | 300      | 1100     | Diluted NSW or artificial mix  |
| Standard HIGH | 500      | 1450     | Fresh NSW or enriched mix      |

1. Run measurement with Standard LOW → record A_low.
2. Run measurement with Standard HIGH → record A_high.
3. K = (C_high – C_low) / (A_high – A_low)
4. Enter K_Ca and K_Mg in sentinel_photometer.yaml.
5. Verify with NSW: Ca ~425 ppm, Mg ~1300 ppm expected.

Recalibrate every 3 months or after reagent replacement.
Arsenazo III shelf life: 6 months after opening. Calmagite: 12 months.

---

## Troubleshooting

### Ca/Mg result out of range (>15% deviation)
- Check reagent expiry – replace and recalibrate if expired.
- Increase flush cycles to 4× to clear residual reagent from cell.
- Verify cell is clean and optically clear.

### ADS1115 not detected (I2C error)
- Default address 0x48 (ADDR→GND); try 0x49 (ADDR→VCC) if not found.
- Verify SDA→GPIO21, SCL→GPIO22.
- Add 100nF decoupling capacitor near ADS1115.

### LED not lighting or dim
- Verify 100Ω series resistor on each LED.
- Check polarity: anode (+) → GPIO via resistor, cathode (–) → GND. Longer leg = anode.
- Measure voltage across LED: ~2.0V red (650nm), ~2.1V green (520nm).

---

## Firmware (ESPHome)

Flash sentinel_photometer.yaml via USB (12V disconnected).
Module connects to SentinelHub AP and pushes data via HTTP REST.
Schedule Ca/Mg tests via Hub panel or ESPHome API.

---

## Done Criteria
- LM2596 stable at 5.0V confirmed by multimeter
- ESP32 boot OK (LED blinks)
- ADS1115 visible in I2C scan logs (address 0x48)
- LED 650nm and 520nm respond to ESPHome commands
- All 3 pumps rotate in correct direction
- Cell water-tight (tested with RO water, no leaks)
- Ca test with NSW: result 380–480 ppm
- Mg test with NSW: result 1150–1400 ppm

---

## Related Documents
- `BOM_sentinel_photometer.md` *(to be created)*
- `BUILD_GUIDE_sentinel_view.md` – Module 4 build guide
- `firmware/sentinel_photometer.yaml` *(to be created)*

---

## Polish (PL)

# Reef Sentinel – BUILD GUIDE
## Module 3: Sentinel Photometer (Fotometr Ca/Mg)

> **Version:** 0.1 (szkielet)
> **Data:** 2026-03-07
> **Planowany start budowy:** Q3 2026
> **Czas montażu:** 5–8 godzin
> **Poziom trudności:** ★★★★☆ Zaawansowany
> **Wymaga:** Modułów 1 + 2 działających

---

## ZANIM ZACZNIESZ

### Co ten moduł robi

Sentinel Photometer mierzy wapń (Ca) i magnez (Mg) metodą fotometryczną – tą samą którą
używają profesjonalne laboratoria ICP-OES, ale dostosowaną do ESP32.

Zasada działania (prawo Beera-Lamberta): stężenie jonu jest proporcjonalne do absorpcji
światła przez roztwór barwnika z tym jonem. Jeden cykl pomiarowy trwa ~8 minut.
Moduł pracuje automatycznie (1× dziennie domyślnie) lub na żądanie z panelu Hub.

| Parametr | Barwnik       | LED                | Cel dokładności |
|----------|---------------|--------------------|-----------------|
| Ca       | Arsenazo III  | 650 nm (czerwony)  | ±10 ppm         |
| Mg       | Calmagite     | 520 nm (zielony)   | ±30 ppm         |

### Architektura modułu

```
Sentinel Hub (10.42.0.1)
       │
    WiFi AP SentinelHub
       │
  ESP32 Photometer   ← sentinel_photometer.yaml
       │
  ┌────┴──────────────────────────────────┐
  │  Blok optyczny                        │
  │  ADS1115 I2C ← BPW34 fotodiody       │
  │  LED 650nm (Ca) + LED 520nm (Mg)     │
  │                                       │
  │  Blok fluidyczny                      │
  │  Pompka próbki         (GPIO15)       │
  │  Pompka Arsenazo III   (GPIO4)        │
  │  Pompka Calmagite      (GPIO16)       │
  │  Pompka spłukiwania RO (GPIO17)       │
  └───────────────────────────────────────┘
```

> Komunikacja: ESP32 łączy się z AP `SentinelHub` i wysyła dane przez HTTP REST
> do Hub co 15 minut. Hub przekazuje je do reef-sentinel.com.

---

## ⚠️ ZASADY BEZPIECZEŃSTWA

```
┌──────────────────────────────────────────────────────────────┐
│                         NIGDY:                               │
│                                                              │
│  ✗ NIE podłączaj ESP32 do USB i LM2596 jednocześnie          │
│  ✗ NIE uruchamiaj pompek reagentów bez testu wodą RO         │
│  ✗ NIE pracuj z Arsenazo III bez rękawiczek nitrylowych      │
│    → Toksyczny: GHS06                                        │
│  ✗ NIE pracuj z Calmagite bez rękawiczek                     │
│    → Drażniący: GHS07                                        │
│  ✗ NIE przechowuj barwników w lodówce z żywnością            │
└──────────────────────────────────────────────────────────────┘
```

**Kolejność zawsze:**
1. Montaż (bez zasilania)
2. Ustawienie LM2596 na 5.0V (bez ESP32)
3. Test firmware bez pompek i reagentów
4. Test pompek z wodą RO – sprawdź szczelność
5. Dopiero potem wprowadzenie Arsenazo III i Calmagite

---

## LISTA KOMPONENTÓW

### Blok optyczny (~40 zł)

| Komponent | Spec | Ilość | Cena |
|-----------|------|-------|------|
| LED 650 nm | 5mm, narrow beam | 2 | ~4 zł |
| LED 520 nm | 5mm, narrow beam | 2 | ~4 zł |
| Fotodioda BPW34 | Silicon PIN, 400–1100nm | 2 | ~6 zł |
| ADS1115 | 16-bit ADC, I2C, 4-kanałowy | 1 | ~15 zł |
| Komora pomiarowa | Druk 3D PETG, 10mm ścieżka optyczna | 1 | ~10 zł |
| Rezystor 10 kΩ | Pull-up fotodioda | 2 | ~0.40 zł |
| Rezystor 100 Ω | Ogranicznik prądu LED | 4 | ~0.40 zł |

### Blok fluidyczny (~124 zł)

| Komponent | Spec | Ilość | Cena |
|-----------|------|-------|------|
| Pompka perystaltyczna 12V | ~1 ml/s | 3 | ~45 zł |
| MOSFET DFRobot Gravity | Sterownik 12V | 3 | ~54 zł |
| Rurka silikonowa 2mm | Chemoodporna | ~50 cm | ~8 zł |
| Złączka Y 2mm | Rozgałęzienie | 2 | ~4 zł |
| Butelka 50 ml + korek | Arsenazo III (ciemna!) | 1 | ~5 zł |
| Butelka 50 ml + korek | Calmagite (ciemna!) | 1 | ~5 zł |
| Butelka 500 ml | Woda RO – spłukiwanie | 1 | ~3 zł |

### Elektronika i zasilanie (~78 zł)

| Komponent | Spec | Ilość | Cena |
|-----------|------|-------|------|
| ESP32 DevKit 30-pin | ESP-WROOM-32 | 1 | ~25 zł |
| LM2596 Buck Converter | 12V→5V z wyświetlaczem | 1 | ~8 zł |
| Zasilacz 12V/2A | Dedykowany dla Photometer | 1 | ~25 zł |
| Kondensator 100µF/25V | Filtrowanie zasilania | 3 | ~3 zł |
| Kondensator 100nF | Decoupling | 5 | ~1 zł |
| Breadboard 400-hole | | 1 | ~8 zł |
| Przewody Dupont | M-F/F-F 20cm | 1 kpl | ~8 zł |

**KOSZT CAŁKOWITY: ~242 zł**

---

## MAPOWANIE GPIO

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

> GPIO32–39 (ADC1) zarezerwowane dla Modułu 2.
> Photometer używa ADS1115 przez I2C – brak konfliktu z WiFi ADC.

---

## ETAP 1 – ZASILANIE

1. Podłącz zasilacz 12V do LM2596 (IN+, IN–) – **ESP32 jeszcze niepodłączony**
2. Multimetr: czarna sonda → COM, czerwona → V/Ω, zakres DC V 20
3. Mierz OUT+ i OUT–, kręć potencjometrem do 5.0V ±0.1V
4. Wyłącz zasilacz
5. Podłącz ESP32: LM2596 OUT+ → VIN, OUT– → GND
6. Włącz zasilacz – ESP32 powinien świecić LED

```
✅ POPRAWNIE:  4.90–5.10V
⚠️ ZA MAŁO:  < 4.9V → ESP32 może nie startować
❌ ZA DUŻO:  > 5.1V → ryzyko uszkodzenia
```

---

## ETAP 2 – BLOK OPTYCZNY

### Dlaczego ADS1115 zamiast wbudowanego ADC ESP32?

- Wbudowany ADC ESP32 ma tylko 12-bit i jest nieliniowy przy aktywnym WiFi
- ADS1115 ma 16-bit (65536 kroków) i komunikuje się przez I2C – brak konfliktu z WiFi
- Dokładność absorpcji: ~0.001 AU vs ~0.01 AU → kluczowe dla ±10 ppm Ca

### Podłączenie ADS1115

```
ADS1115          ESP32
VDD    ────────► 3V3
GND    ────────► GND
SDA    ────────► GPIO21
SCL    ────────► GPIO22
ADDR   ────────► GND  (adres I2C = 0x48)
```

Kondensator 100nF między VDD a GND jak najbliżej układu ADS1115.

### Schemat optyczny (jeden kanał, drugi identyczny)

```
ESP32 GPIO25 ──[100Ω]──► LED 650nm ──► [komora 10mm] ──► BPW34 ──[10kΩ]──► ADS1115 A0
                                                          katoda → GND
```

- LED: dłuższa nóżka = anoda (+) → przez rezystor do GPIO
- BPW34: anoda → wejście ADS1115, katoda → GND, rezystor 10kΩ między anodą a 3V3

### Komora pomiarowa (druk 3D PETG)

Wymagania komory:
- Ścieżka optyczna dokładnie 10mm (od LED do fotodiody)
- Dwa otwory na rurkę 2mm (wlot + wylot cieczy)
- Otwory na LED i BPW34 naprzeciwko siebie, koncentryczne z ścieżką
- Po złożeniu: test szczelności wodą RO przy ciśnieniu roboczym pompki

---

## ETAP 3 – BLOK FLUIDYCZNY

### Schemat sterowania pompką (powtórz ×3)

```
ESP32 GPIO ──────────────────► MOSFET SIG
ESP32 3V3  ──────────────────► MOSFET VCC
ESP32 GND  ──────────────────► MOSFET GND
                                MOSFET OUT+ ──► Pompka (+)
Zasilacz 12V GND ────────────► Pompka (–)
Zasilacz 12V OUT+ ──────────── (zasila pompkę przez MOSFET OUT– nie jest połączony)
```

> ⚠️ Kierunek obrotów pompki peristaltycznej wyznacza polaryzacja silnika.
> Sprawdź kierunek przepływu cieczy przed podłączeniem reagentów.

### Routing rurek

| Pompka | GPIO | Od | Do |
|--------|------|----|----|
| Próbki | GPIO15 | Akwarium/sump | Wlot komory |
| Arsenazo III | GPIO4 | Butelka Ca | Wlot komory (Y-fitting) |
| Calmagite | GPIO16 | Butelka Mg | Wlot komory (Y-fitting) |
| Spłukiwania | GPIO17 | Butelka RO | Wlot komory → wymywa do odpadów |

> Przetestuj cały routing wodą RO zanim wprowadzisz barwniki.
> Sprawdź szczelność złączek Y pod ciśnieniem.

---

## ETAP 4 – PROTOKÓŁ POMIARU

### Sekwencja Ca (Arsenazo III)

1. Pompka próbki: pobierz 2 ml wody z akwarium do komory
2. Pompka Ca: dodaj 0.5 ml Arsenazo III (stężenie 0.05%)
3. Odczekaj 60 s – reakcja barwna (roztwór sinieje proporcjonalnie do Ca)
4. LED 650nm ON → odczyt ADS1115 Ch0 → zapisz absorpcję A
5. Oblicz: `Ca [ppm] = (A – A_blank) / K_Ca`
6. Pompka spłukiwania: 3× przepłucz komorę wodą RO
7. LED 650nm OFF

### Sekwencja Mg (Calmagite)

1. Pompka próbki: pobierz 2 ml wody z akwarium do komory
2. Pompka Mg: dodaj 0.5 ml Calmagite + 0.1 ml buforu pH 10
3. Odczekaj 45 s – reakcja barwna (roztwór czerwienieje proporcjonalnie do Mg)
4. LED 520nm ON → odczyt ADS1115 Ch1 → zapisz absorpcję A
5. Oblicz: `Mg [ppm] = (A – A_blank) / K_Mg`
6. Pompka spłukiwania: 3× przepłucz komorę wodą RO
7. LED 520nm OFF

---

## ETAP 5 – KALIBRACJA

### Kalibracja 2-punktowa

| Roztwór wzorcowy | Ca [ppm] | Mg [ppm] | Skąd wziąć |
|------------------|----------|----------|------------|
| Standard LOW | 300 | 1100 | Rozcieńczony NSW lub artificial |
| Standard HIGH | 500 | 1450 | Świeży NSW lub wzmocniony |

1. Zmierz Standard LOW → zapisz A_low
2. Zmierz Standard HIGH → zapisz A_high
3. Oblicz: `K = (C_high – C_low) / (A_high – A_low)`
4. Wpisz `K_Ca` i `K_Mg` do `sentinel_photometer.yaml`
5. Weryfikacja: zmierz NSW → Ca ~380–480 ppm, Mg ~1150–1400 ppm

> Kalibruj co 3 miesiące lub po wymianie barwnika.
> Arsenazo III: termin ważności 6 mies. od otwarcia.
> Calmagite: 12 mies. od otwarcia.

---

## ETAP 6 – FIRMWARE

Wgraj `sentinel_photometer.yaml` przez USB (12V odłączone).
Po wgraniu moduł łączy się z AP `SentinelHub` (adres DHCP).
Dane Ca i Mg wysyłane do Hub przez HTTP REST co 15 minut.
Testy uruchamialne z panelu Hub (`http://reef-sentinel.local`) lub przez ESPHome API.

---

## TROUBLESHOOTING

### Wynik Ca/Mg poza normą (>15% odchyłka)

```
Przyczyna 1: Zdegradowany barwnik
  → Arsenazo III: maksymalnie 6 mies. po otwarciu
  → Calmagite: maksymalnie 12 mies. po otwarciu
  → Wymień barwnik i przeprowadź kalibrację od nowa

Przyczyna 2: Resztkowy barwnik w komorze
  → Zwiększ liczbę cykli spłukiwania (4× zamiast 3×)
  → Sprawdź kierunek przepływu pompki RO

Przyczyna 3: Nieaktualna kalibracja
  → Przeprowadź kalibrację 2-punktową od nowa
  → Sprawdź daty i stężenia roztworów wzorcowych
```

### ADS1115 nie wykryty (błąd I2C)

```
Przyczyna 1: Zły adres I2C
  → Domyślnie 0x48 (ADDR→GND), alternatywnie 0x49 (ADDR→VCC)
  → Włącz i2c scan w ESPHome – pokaże znalezione adresy

Przyczyna 2: Złe przewody SDA/SCL
  → SDA → GPIO21, SCL → GPIO22 (sprawdź czy nie zamienione)

Przyczyna 3: Szumy zasilania
  → Dodaj kondensator 100nF między VDD a GND blisko ADS1115
```

### LED nie świeci lub świeci słabo

```
Przyczyna 1: Brak rezystora ograniczającego
  → Każdy LED MUSI mieć szeregowo 100Ω
  → Bez rezystora LED przepali się lub uszkodzi GPIO ESP32

Przyczyna 2: Zła polaryzacja
  → Anoda (+) = dłuższa nóżka → GPIO przez rezystor
  → Katoda (–) = krótsza nóżka → GND

Przyczyna 3: Napięcie poza zakresem
  → Multimetr na LED: ~2.0V dla 650nm, ~2.1V dla 520nm
```

---

## FINALNA CHECKLIST

```
HARDWARE:
☐ LM2596: 5.0V ±0.1V zmierzone multimetrem
☐ ESP32: boot OK (LED miga po włączeniu)
☐ ADS1115: widoczny w logach I2C (adres 0x48)
☐ LED 650nm: świeci na komendę z ESPHome
☐ LED 520nm: świeci na komendę z ESPHome
☐ Wszystkie 3 pompki: działają we właściwym kierunku (test wodą RO)
☐ Komora: szczelna (zero wycieków przy ciśnieniu roboczym)

OPROGRAMOWANIE:
☐ sentinel_photometer.yaml wgrany bez błędów
☐ WiFi: połączony z SentinelHub (adres IP w logach)
☐ K_Ca i K_Mg wpisane i zweryfikowane po kalibracji
☐ Test Ca z NSW: wynik 380–480 ppm
☐ Test Mg z NSW: wynik 1150–1400 ppm

STATUS: ✅ Sentinel Photometer gotowy!
        Następny krok: BUILD_GUIDE_sentinel_view.md
```

---

## NASTĘPNE KROKI

1. **Sentinel View (Module 4)** – Ca/Mg na dotykowym panelu Nextion
2. **Cloud** – AI insights dla dozowania Ca/Mg na reef-sentinel.com
3. **Kalibracja sezonowa** – co 3 miesiące lub po zmianie reagentów

---

## POWIĄZANE DOKUMENTY

- `BOM_sentinel_photometer.md` *(do stworzenia)*
- `BUILD_GUIDE_sentinel_view.md` – Instrukcja montażu Modułu 4
- `firmware/sentinel_photometer.yaml` *(do stworzenia)*

---

*Reef Sentinel Lab – Open-source aquarium controller*
*reef-sentinel.com | github.com/reef-sentinel*
*Ostatnia aktualizacja: 2026-03-07*
