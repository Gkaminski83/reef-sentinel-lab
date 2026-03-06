# Reef Sentinel Lab – WIRING DIAGRAM
## Kompletny schemat połączeń: Module 1 + Module 2

> **Version:** 1.0  
> **Data:** 2026-03-06  
> **Dotyczy:** Faza 1 – breadboard prototype  
> **Legenda:** 🔴 12V+ | 🟠 5V | ⚫ GND | 🟡 Sygnał/Data | 🔵 Analog | ⚪ 3V3

---

## 1. ARCHITEKTURA ZASILANIA – WIDOK OGÓLNY

```
  ┌───────────────────────┐         ┌───────────────────────┐
  │  ZASILACZ #1  12V/2A  │         │  ZASILACZ #2  12V/3A  │
  │  Sentinel Hub         │         │  Sentinel Chem/Mon.   │
  │  własne gniazdo 230V  │         │  własne gniazdo 230V  │
  └──────────┬────────────┘         └──────────┬────────────┘
             │ 12V DC                          │ 12V DC
             ▼                                 ▼
     ┌───────────────┐                ┌───────────────┐
     │  LM2596 #1    │                │  LM2596 #2    │
     │  12V → 5V     │                │  12V → 5V     │
     └───────┬───────┘                └───────┬───────┘
             │ 5V                             │ 5V
             ▼                                ▼
      ESP32 Hub                        ESP32 Chem
      + OLED                           + Sensory
      + SD card                        + MOSFET ×5
                                       + Pompki ×4
```

> ⚠️ Każdy moduł ma **własny zasilacz 12V i własny LM2596**. Moduły są elektrycznie niezależne.  
> ⚠️ Nigdy nie podłączaj ESP32 do USB i LM2596 jednocześnie.

---

## 2. SENTINEL HUB – MODULE 1

### 2.1 Schemat zasilania LM2596 #1

```
ZASILACZ #1 – 12V/2A (Sentinel Hub)
    (+) ────────────────────────────→ LM2596 #1 [IN+]
    (–) ────────────────────────────→ LM2596 #1 [IN–]

LM2596 #1 [OUT+] 5.0V ─────────────→ ESP32 Hub [VIN  pin 15]
LM2596 #1 [OUT–] GND  ─────────────→ ESP32 Hub [GND  pin 14]
```

### 2.2 ESP32 Hub – pełny pinout

```
                    ┌─────────────────┐
               3V3 ─┤ 1  ESP32   30  ├─ GND
               EN  ─┤ 2          29  ├─ GPIO23 ──→ SD MOSI
            VP/36  ─┤ 3          28  ├─ GPIO22 ──→ OLED SCL
            VN/39  ─┤ 4          27  ├─ GPIO21 ──→ OLED SDA
            GPIO34 ─┤ 5          26  ├─ GPIO19 ──→ SD MISO
            GPIO35 ─┤ 6          25  ├─ GPIO18 ──→ SD SCK
            GPIO32 ─┤ 7          24  ├─ GPIO5  ──→ SD CS
            GPIO33 ─┤ 8          23  ├─ GPIO17
            GPIO25 ─┤ 9          22  ├─ GPIO16
            GPIO26 ─┤ 10         21  ├─ GPIO4
            GPIO27 ─┤ 11         20  ├─ GPIO0
            GPIO14 ─┤ 12         19  ├─ GPIO2  ←── LED (built-in)
            GPIO12 ─┤ 13         18  ├─ GPIO15
               GND ─┤ 14         17  ├─ GND
     LM2596→   VIN ─┤ 15         16  ├─ 3V3
                    └────[USB]────────┘
```

### 2.3 OLED 0.96" (SSD1306, I2C)

```
OLED              ESP32 Hub
 VCC ────────────→ 3V3  (pin 1 lub 16)
 GND ────────────→ GND  (pin 14 lub 17)
 SDA ────────────→ GPIO21 (pin 27)
 SCL ────────────→ GPIO22 (pin 28)
```

> Adres I2C: 0x3C (typowy dla SSD1306). Jeśli OLED nie odpowiada, spróbuj 0x3D.

### 2.4 SD Card Reader (SPI)

```
SD Reader         ESP32 Hub
 VCC ────────────→ 5V   (VIN, pin 15)  ← SD może działać na 5V lub 3.3V
 GND ────────────→ GND  (pin 14)
 CS  ────────────→ GPIO5  (pin 24)
 MOSI ───────────→ GPIO23 (pin 29)
 MISO ───────────→ GPIO19 (pin 26)
 SCK ────────────→ GPIO18 (pin 25)
```

> Karta microSD: format FAT32, max 32GB, Class 10.

### 2.5 GPIO Hub – tabela zbiorcza

| GPIO | Pin | Funkcja | Urządzenie | Interfejs |
|------|-----|---------|-----------|-----------|
| GPIO21 | 27 | OLED SDA | OLED 0.96" SSD1306 | I2C |
| GPIO22 | 28 | OLED SCL | OLED 0.96" SSD1306 | I2C |
| GPIO5  | 24 | SD CS | SD Card Reader | SPI |
| GPIO18 | 25 | SD SCK | SD Card Reader | SPI |
| GPIO19 | 26 | SD MISO | SD Card Reader | SPI |
| GPIO23 | 29 | SD MOSI | SD Card Reader | SPI |
| GPIO2  | 19 | Status LED | Wbudowana dioda | Digital |
| WiFi | — | AP SentinelHub + STA home | — | 802.11b/g/n |

---

## 3. SENTINEL CHEM/MONITOR – MODULE 2

### 3.1 Schemat zasilania LM2596 #2

```
ZASILACZ #2 – 12V/3A (Sentinel Chem/Monitor)
    (+) ────────────────────────────→ LM2596 #2 [IN+]
    (–) ────────────────────────────→ LM2596 #2 [IN–]

LM2596 #2 [OUT+] 5.0V ─────────────→ ESP32 Chem [VIN  pin 15]
LM2596 #2 [OUT–] GND  ─────────────→ ESP32 Chem [GND  pin 14]
                                  └→ Wspólna magistrala GND
```

### 3.2 ESP32 Chem – pełny pinout

```
                    ┌─────────────────┐
               3V3 ─┤ 1  ESP32   30  ├─ GND
               EN  ─┤ 2          29  ├─ GPIO23
            VP/36  ─┤ 3          28  ├─ GPIO22
            VN/39  ─┤ 4          27  ├─ GPIO21
  pH analog  GPIO34─┤ 5          26  ├─ GPIO19
             GPIO35─┤ 6          25  ├─ GPIO18
  EC analog  GPIO32─┤ 7          24  ├─ GPIO5
  DS18B20    GPIO33─┤ 8          23  ├─ GPIO17
  Stirrer    GPIO25─┤ 9          22  ├─ GPIO16 ──→ MOSFET #4 SIG (Pompka RO)
             GPIO26─┤ 10         21  ├─ GPIO4  ──→ MOSFET #3 SIG (Pompka Odpad)
             GPIO27─┤ 11         20  ├─ GPIO0
             GPIO14─┤ 12         19  ├─ GPIO2  ──→ MOSFET #2 SIG (Pompka HCl) ⚠️LED
             GPIO12─┤ 13         18  ├─ GPIO15 ──→ MOSFET #1 SIG (Pompka Próbka)
               GND ─┤ 14         17  ├─ GND
     LM2596→   VIN ─┤ 15         16  ├─ 3V3
                    └────[USB]────────┘
```

### 3.3 DS18B20 × 3 (1-Wire, temperatura)

```
ESP32 3V3 (pin 1) ──┬────────────────────→ DS18B20 #1 VCC (czerwony)
                    ├────────────────────→ DS18B20 #2 VCC (czerwony)
                    ├────────────────────→ DS18B20 #3 VCC (czerwony)
                   [4.7kΩ]  ← REZYSTOR PULL-UP (konieczny!)
                    │
ESP32 GPIO33 (pin8) ┴────────────────────→ DS18B20 #1 DATA (żółty)
                         ───────────────→ DS18B20 #2 DATA (żółty)
                         ───────────────→ DS18B20 #3 DATA (żółty)

ESP32 GND (pin 14) ──────────────────────→ DS18B20 #1 GND (czarny)
                         ───────────────→ DS18B20 #2 GND (czarny)
                         ───────────────→ DS18B20 #3 GND (czarny)
```

**Rozmieszczenie sond:**

| Sonda | Lokalizacja |
|-------|-------------|
| DS18B20 #1 (index 0) | Akwarium (główny zbiornik) |
| DS18B20 #2 (index 1) | Sump |
| DS18B20 #3 (index 2) | Komora pomiarowa |

> Rezystor 4.7kΩ: paski **żółty–fioletowy–czerwony–złoty**.  
> Wstawiasz go między linię 3V3 a DATA (GPIO33) na breadboard.

### 3.4 pH Sensor (DFRobot Gravity pH V2)

```
pH Amplifier Board    ESP32 Chem
 VCC ────────────────→ 5V   (VIN, pin 15)
 GND ────────────────→ GND  (pin 14)
 AO  ────────────────→ GPIO34 (pin 5)  ← tylko INPUT!

Sonda pH → złącze BNC na amplifier board
```

**Kondensatory filtrujące (obowiązkowe!):**
```
100µF/25V elektrolit:   (+) → linia 5V przy module pH
                        (–) → GND
100nF ceramiczny:       jedna nóżka → linia AO (przed GPIO34)
                        druga nóżka → GND
```

> ⚠️ Kabel BNC prowadź z DALA od kabli 12V, MOSFET i GND zasilacza.  
> ⚠️ pH sonda między pomiarami: zanurzona w wodzie RO (nie zostawiaj w powietrzu).

### 3.5 EC / TDS Sensor (DFRobot Gravity TDS)

```
EC Sensor Module      ESP32 Chem
 VCC ────────────────→ 5V   (VIN, pin 15)
 GND ────────────────→ GND  (pin 14)
 AO  ────────────────→ GPIO32 (pin 7)

Kondensator 100µF/25V: (+) → 5V przy module EC, (–) → GND
```

### 3.6 MOSFET × 5 – sterowanie pompkami i mieszadełkiem

#### Zasada low-side switching

```
12V (+) ─────────────────────────────────────────→ Pompka (+)
12V (–) ──→ MOSFET [V+]    MOSFET [OUT–] ────────→ Pompka (–)
             MOSFET [SIG]  ←──────────────── GPIO ESP32
             MOSFET [GND]  ←──────────────── GND ESP32 ← KRYTYCZNE!
```

#### Common Ground – najważniejsza zasada

```
┌─────────────────────────────────────────────────────────────┐
│  GND ESP32 (pin 14 lub 17)                                  │
│       │                                                     │
│       └──→ magistrala GND na breadboard                     │
│               │         │         │         │         │    │
│               ▼         ▼         ▼         ▼         ▼    │
│           MOSFET#1  MOSFET#2  MOSFET#3  MOSFET#4  MOSFET#5 │
│           [GND/COM] [GND/COM] [GND/COM] [GND/COM] [GND/COM] │
│                                                             │
│  BEZ TEGO: pompki nie reagują na sygnał z ESP32             │
└─────────────────────────────────────────────────────────────┘
```

#### Schemat każdego MOSFET

```
MOSFET #1 – Pompka #1 (PRÓBKA z akwarium):
  [SIG] ←── GPIO15 (pin 18)   ESP32 Chem
  [GND] ←── GND               ESP32 Chem  ← MUSI BYĆ!
  [V+]  ←── 12V (+)           zasilacz
  [OUT–] ──→ Pompka #1 (–)
  12V (+) ──→ Pompka #1 (+)   zasilacz bezpośrednio

MOSFET #2 – Pompka #2 (HCl – TITRACJA):
  [SIG] ←── GPIO2  (pin 19)   ⚠️ Ten pin ma wbudowane LED
  [GND] ←── GND               ESP32 Chem
  [V+]  ←── 12V (+)
  [OUT–] ──→ Pompka #2 (–)
  12V (+) ──→ Pompka #2 (+)

MOSFET #3 – Pompka #3 (ODPAD):
  [SIG] ←── GPIO4  (pin 21)
  [GND] ←── GND               ESP32 Chem
  [V+]  ←── 12V (+)
  [OUT–] ──→ Pompka #3 (–)
  12V (+) ──→ Pompka #3 (+)

MOSFET #4 – Pompka #4 (WODA RO):
  [SIG] ←── GPIO16 (pin 22)
  [GND] ←── GND               ESP32 Chem
  [V+]  ←── 12V (+)
  [OUT–] ──→ Pompka #4 (–)
  12V (+) ──→ Pompka #4 (+)

MOSFET #5 – Mieszadełko (STIRRER PWM):
  [SIG] ←── GPIO25 (pin 9)    PWM output
  [GND] ←── GND               ESP32 Chem
  [V+]  ←── 12V (+)
  [OUT–] ──→ Silnik stirrer (–)
  12V (+) ──→ Silnik stirrer (+)
```

#### Tabela zbiorcza MOSFET

| MOSFET | GPIO | Pin ESP32 | Pompka/Urządzenie | Rola |
|--------|------|-----------|-------------------|------|
| #1 | GPIO15 | 18 | Pompka #1 | Pobieranie próbki z akwarium |
| #2 | GPIO2  | 19 | Pompka #2 | Dozowanie HCl ⚠️ |
| #3 | GPIO4  | 21 | Pompka #3 | Wypompowanie odpadu |
| #4 | GPIO16 | 22 | Pompka #4 | Woda RO (płukanie/storage pH) |
| #5 | GPIO25 | 9  | Silnik stirrer | Mieszadełko magnetyczne (PWM) |

### 3.7 GPIO Chem – tabela zbiorcza

| GPIO | Pin | Funkcja | Urządzenie | Typ sygnału |
|------|-----|---------|-----------|-------------|
| GPIO34 | 5  | pH analog IN | DFRobot pH V2 | ADC1 – tylko INPUT |
| GPIO32 | 7  | EC analog IN | DFRobot TDS | ADC1 – tylko INPUT |
| GPIO33 | 8  | DS18B20 1-Wire | DS18B20 ×3 | Digital (pull-up 4.7kΩ) |
| GPIO25 | 9  | Stirrer PWM | MOSFET #5 → Silnik | PWM output |
| GPIO15 | 18 | Pompka #1 | MOSFET #1 SIG | Digital output |
| GPIO2  | 19 | Pompka #2 (HCl) | MOSFET #2 SIG | Digital output ⚠️ LED |
| GPIO4  | 21 | Pompka #3 | MOSFET #3 SIG | Digital output |
| GPIO16 | 22 | Pompka #4 | MOSFET #4 SIG | Digital output |

---

## 4. SCHEMAT CAŁOŚCI – WIDOK SYSTEMOWY

```
╔═════════════════════════════════════════════════════════════════╗
║                    ZASILACZ 12V / 5A                            ║
╚═══════════════════════╤═════════════════════════════════════════╝
                        │ 12V
          ┌─────────────┴──────────────┐
          │                            │
   ┌──────┴──────┐               ┌─────┴──────┐
   │  LM2596 #1  │               │  LM2596 #2  │
   │  → 5.0V     │               │  → 5.0V     │
   └──────┬──────┘               └──────┬──────┘
          │ 5V                          │ 5V
          ▼                             ▼
   ┌─────────────────┐        ┌──────────────────────────┐
   │  SENTINEL HUB   │        │  SENTINEL CHEM/MONITOR   │
   │  ESP32          │        │  ESP32                   │
   │                 │        │                          │
   │  GPIO21 → OLED SDA       │  GPIO34 ← pH AO          │
   │  GPIO22 → OLED SCL       │  GPIO32 ← EC AO          │
   │  GPIO5  → SD CS  │        │  GPIO33 ← DS18B20 DATA   │
   │  GPIO18 → SD SCK │        │  GPIO25 → MOSFET#5 SIG  │
   │  GPIO19 → SD MISO│        │  GPIO15 → MOSFET#1 SIG  │
   │  GPIO23 → SD MOSI│        │  GPIO2  → MOSFET#2 SIG  │
   │                 │        │  GPIO4  → MOSFET#3 SIG  │
   │  WiFi AP        │        │  GPIO16 → MOSFET#4 SIG  │
   │  SentinelHub    │        │                          │
   │  10.42.0.1      │        │  WiFi Client             │
   │  MQTT :1883     │◄──────►│  → SentinelHub           │
   └─────────────────┘  WiFi  │  MQTT → 10.42.0.1        │
                              └──────────────────────────┘
                                         │
                         ┌───────────────┼───────────────┐
                         │               │               │
                    ┌────┴────┐    ┌─────┴────┐    ┌─────┴────┐
                    │pH Sensor│    │EC Sensor │    │DS18B20×3 │
                    │Gravity  │    │Gravity   │    │1-Wire    │
                    │V2       │    │TDS       │    │+ 4.7kΩ   │
                    └─────────┘    └──────────┘    └──────────┘

                    MOSFET×5 → Pompki×4 + Silnik stirrer
                    Wszystkie z 12V (bezpośrednio z zasilacza)
                    Common GND: GND ESP32 ↔ GND/COM każdy MOSFET
```

---

## 5. ZASILANIE – MAGISTRALE ZBIORCZE

### Magistrala 12V – Sentinel Hub (czerwona)

```
Zasilacz #1 (+12V) ──┬──→ LM2596 #1 [IN+]
```

### Magistrala 12V – Sentinel Chem/Monitor (czerwona)

```
Zasilacz #2 (+12V) ──┬──→ LM2596 #2 [IN+]
                  ├──→ MOSFET #1 [V+]
                  ├──→ MOSFET #2 [V+]
                  ├──→ MOSFET #3 [V+]
                  ├──→ MOSFET #4 [V+]
                  └──→ MOSFET #5 [V+]
                  (i bezpośrednio do (+) każdej pompki i silnika)
```

### Magistrala 5V (pomarańczowa)

```
LM2596 #1 [OUT+] ──→ ESP32 Hub [VIN]
                 └──→ OLED VCC
                 └──→ SD Card Reader VCC

LM2596 #2 [OUT+] ──→ ESP32 Chem [VIN]
                 └──→ pH Sensor VCC
                 └──→ EC Sensor VCC
```

### Magistrala 3.3V (biała)

```
ESP32 Chem [3V3 pin 1] ──→ DS18B20 ×3 VCC (przez magistralę breadboard)
                       └──→ (+) rezystora pull-up 4.7kΩ
```

### Magistrala GND (czarna) – dwa niezależne obwody

```
Zasilacz #1 (GND) ──→ LM2596 #1 [IN–] ──→ LM2596 #1 [OUT–] ──→ ESP32 Hub GND
                                        └──→ OLED GND
                                        └──→ SD Card GND

Zasilacz #2 (GND) ──→ LM2596 #2 [IN–] ──→ LM2596 #2 [OUT–] ──→ ESP32 Chem GND
                                        │         ← MAGISTRALA GND CHEM →
                                        ├──→ pH Sensor GND
                                        ├──→ EC Sensor GND
                                        ├──→ DS18B20 ×3 GND
                                        ├──→ MOSFET #1 [GND/COM]  ← KRYTYCZNE
                                        ├──→ MOSFET #2 [GND/COM]  ← KRYTYCZNE
                                        ├──→ MOSFET #3 [GND/COM]  ← KRYTYCZNE
                                        ├──→ MOSFET #4 [GND/COM]  ← KRYTYCZNE
                                        └──→ MOSFET #5 [GND/COM]  ← KRYTYCZNE
```

> Obwody GND modułu Hub i modułu Chem są od siebie **elektrycznie odizolowane**. Moduły komunikują się wyłącznie przez WiFi.

---

## 6. POŁĄCZENIA HYDRAULICZNE (RURKI/WĘŻE)

```
AKWARIUM
    │
    └──[wąż silikonowy]──→ Pompka #1 (próbka)
                                  │
                                  ▼
                      KOMORA POMIAROWA (~100ml)
                      ┌────────────────────────┐
                      │  ← pH sonda (BNC)      │
                      │  ← EC sonda            │
                      │  ← DS18B20 #3 (temp)   │
                      │  ← Stir bar PTFE       │
                      │                        │
                      │  ↑ Pompka #2 (HCl)     │  ← z butelki HCl 0.1M
                      └────────┬───────────────┘
                               │
                    ┌──────────┴─────────┐
                    │                    │
                    ▼                    ▼
             Pompka #3              Pompka #4
             (odpad)                (woda RO)
                    │                    │
                    ▼                    ▼
            Kanister 5L            Kanister 5L
            ODPAD                  WODA RO
                                        │
                                        └──→ (też przechowuje sondę pH
                                              między pomiarami!)

SILNIK STIRRER
    └── pod komorą pomiarową (odległość < 15mm od dna)
```

**Oznakowanie węży – obowiązkowe:**

| Wąż | Kolor etykiety | Nigdy nie mylić z |
|-----|---------------|-------------------|
| Pompka #1 – PRÓBKA | Niebieski | — |
| Pompka #2 – HCl ☠️ | Czerwony | Wszystkimi innymi! |
| Pompka #3 – ODPAD | Żółty | — |
| Pompka #4 – RO | Biały | — |

---

## 7. CHECKLIST PRZED PIERWSZYM WŁĄCZENIEM

### Zasilanie

```
☐ LM2596 #1 (Hub):   OUT+ = 5.0V ±0.1V (zmierzone multimetrem)
☐ LM2596 #2 (Chem):  OUT+ = 5.0V ±0.1V (zmierzone multimetrem)
☐ Zasilacz #1 i #2: ODŁĄCZONE podczas montażu
☐ ESP32 Hub: NIE podłączony do USB podczas pracy z zasilaczem
☐ ESP32 Chem: NIE podłączony do USB podczas pracy z zasilaczem
```

### Sentinel Hub

```
☐ OLED: VCC→3V3, GND→GND, SDA→GPIO21, SCL→GPIO22
☐ SD (jeśli używasz): VCC→5V, GND, CS→GPIO5, MOSI→GPIO23, MISO→GPIO19, SCK→GPIO18
☐ Brak zwarcia 3V3 ↔ GND (multimetr tryb ciągłości: cisza = OK)
```

### Sentinel Chem/Monitor

```
☐ DS18B20 ×3: VCC→3V3, DATA→GPIO33, GND→GND
☐ Rezystor 4.7kΩ: między 3V3 a GPIO33 (na breadboard)
☐ pH sensor: VCC→5V, GND→GND, AO→GPIO34
☐ pH kabel BNC: oddalony od kabli 12V i MOSFET
☐ EC sensor: VCC→5V, GND→GND, AO→GPIO32
☐ Kondensatory przy pH i EC (100µF/25V na 5V, 100nF na AO)
```

### Common Ground (KRYTYCZNE)

```
☐ GND ESP32 Chem → magistrala GND breadboard
☐ MOSFET #1 GND/COM → magistrala GND
☐ MOSFET #2 GND/COM → magistrala GND
☐ MOSFET #3 GND/COM → magistrala GND
☐ MOSFET #4 GND/COM → magistrala GND
☐ MOSFET #5 GND/COM → magistrala GND
```

### MOSFET i pompki

```
☐ MOSFET #1: SIG→GPIO15, V+→12V, OUT–→Pompka#1(–), 12V→Pompka#1(+)
☐ MOSFET #2: SIG→GPIO2,  V+→12V, OUT–→Pompka#2(–), 12V→Pompka#2(+)
☐ MOSFET #3: SIG→GPIO4,  V+→12V, OUT–→Pompka#3(–), 12V→Pompka#3(+)
☐ MOSFET #4: SIG→GPIO16, V+→12V, OUT–→Pompka#4(–), 12V→Pompka#4(+)
☐ MOSFET #5: SIG→GPIO25, V+→12V, OUT–→Silnik(–),   12V→Silnik(+)
☐ Pompki zasilane z 12V (nie 5V!)
☐ Węże oznaczone kolorami
☐ Pompka #2 (HCl): bez HCl podczas testu (najpierw woda RO)
```

### WiFi / MQTT

```
☐ ESP32 Hub: firmware wgrany, AP SentinelHub widoczny w sieci
☐ ESP32 Chem: firmware wgrany, łączy się z SentinelHub
☐ MQTT topics widoczne w HA:
   reef/chem/temp/aquarium
   reef/chem/temp/sump
   reef/chem/ph/value
   reef/chem/ec/value
   reef/chem/kh/value (po pierwszej titracji)
```

---

## 8. NAJCZĘSTSZE BŁĘDY OKABLOWANIA

| Błąd | Objaw | Rozwiązanie |
|------|-------|-------------|
| Brak common ground MOSFET | Pompki nie reagują na GPIO | GND ESP32 → GND/COM każdego MOSFET |
| USB + LM2596 jednocześnie | ESP32 przegrzany lub uszkodzony | Nigdy nie łącz dwóch źródeł zasilania |
| LM2596 > 5.2V | ESP32 uszkodzony przy bootowaniu | Ustaw 5.0V multimetrem PRZED podłączeniem ESP32 |
| Rezystor 10kΩ zamiast 4.7kΩ | DS18B20 daje –127°C | Sprawdź paski: żółty-fioletowy-czerwony = 4.7kΩ |
| TX/RX UART nie skrzyżowane | Nextion/inne urządzenie UART nie reaguje | TX ESP32→RX urządzenia, RX ESP32←TX urządzenia |
| BNC blisko kabli 12V | pH skacze, brak stabilizacji | Poprowadź BNC z dala od kabli mocy |
| Pompka podłączona do 5V | Pompka się nie kręci lub kręci słabo | V+ MOSFET musi iść do 12V, nie do LM2596 |
| Odwrócona pompka | Pompuje w złą stronę | Zamień (+) i (–) przy pompce |

---

## 9. KOLEJNOŚĆ URUCHAMIANIA

```
1. WSZYSTKO ODŁĄCZONE
   │
2. Ustaw LM2596 #1 na 5.0V (bez ESP32)
   │
3. Ustaw LM2596 #2 na 5.0V (bez ESP32)
   │
4. Odłącz zasilacz
   │
5. Podłącz ESP32 Hub do LM2596 #1
   (USB laptopa ODŁĄCZONE od ESP32)
   │
6. Wgraj firmware Hub przez USB
   ZASILACZ ODŁĄCZONY
   │
7. Po wgraniu – odłącz USB
   Podłącz zasilacz → sprawdź OLED
   │
8. Podłącz ESP32 Chem do LM2596 #2
   │
9. Wgraj firmware Chem przez USB
   ZASILACZ ODŁĄCZONY
   │
10. Po wgraniu – odłącz USB
    Podłącz zasilacz → sprawdź logi
    │
11. Testuj kolejno:
    DS18B20 → pH (na RO) → EC → pompki (na RO!) → mieszadełko
    │
12. Dopiero po wszystkich testach: HCl do pompki #2
```

---

## POWIĄZANE DOKUMENTY

- [`BUILD_GUIDE_sentinel_hub.md`](BUILD_GUIDE_sentinel_hub.md) – Instrukcja montażu Module 1
- [`BUILD_GUIDE_sentinel_chem_monitor.md`](BUILD_GUIDE_sentinel_chem_monitor.md) – Instrukcja montażu Module 2
- [`sentinel_hub_chem_monitor_BOM.md`](sentinel_hub_chem_monitor_BOM.md) – Lista komponentów
- [`firmware/sentinel_hub.yaml`](firmware/sentinel_hub.yaml) – ESPHome Module 1
- [`firmware/sentinel_chem.yaml`](firmware/sentinel_chem.yaml) – ESPHome Module 2

---

*Reef Sentinel Lab – Open-source aquarium controller*  
*reef-sentinel.com | github.com/reef-sentinel*  
*Ostatnia aktualizacja: 2026-03-06*
