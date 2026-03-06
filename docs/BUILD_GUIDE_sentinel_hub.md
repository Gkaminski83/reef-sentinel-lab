# Reef Sentinel – BUILD GUIDE
## Module 1: Sentinel Hub (Koordynator WiFi)

> **Version:** 1.0  
> **Data:** 2026-03-06  
> **Czas montażu:** 2–3 godziny  
> **Poziom trudności:** ⭐⭐☆☆☆ (Podstawowy)  
> **Status modułu:** 🟢 Możesz budować TERAZ – wszystkie komponenty dostępne

---

## ZANIM ZACZNIESZ

### Co ten moduł robi

Sentinel Hub to „mózg" całego systemu. Sam w sobie nie mierzy niczego w akwarium –
jego rola to:

- Tworzenie sieci WiFi (hotspot `SentinelHub`) do której łączą się pozostałe moduły
- Jednoczesne połączenie z Twoją siecią domową (dostęp do internetu)
- Zbieranie danych od wszystkich modułów i wysyłanie ich do reef-sentinel.com co 15 min
- Wyświetlanie statusu systemu na małym ekraniku OLED
- Opcjonalne zapisywanie danych na karcie SD (backup offline)

### Co będziesz potrzebował

**Komponenty (wszystkie już masz):**

| # | Komponent | Jak wygląda |
|---|-----------|-------------|
| 1 | ESP32 DevKit (30-pin) | Zielona płytka z metalową osłoną i portem USB-C/microUSB |
| 2 | LM2596 Buck Converter | Mała płytka z cewką i wyświetlaczem pokazującym napięcie |
| 3 | Grove OLED 0.96" | Mały czarny ekranik na białej płytce |
| 4 | SD Card Reader (moduł SPI) | Płytka z gniazdem na kartę microSD |
| 5 | Zasilacz 12V/5A | Czarna kostka z kablem i wtyczką DC |
| 6 | Breadboard 400-hole | Biała/przezroczysta płytka z otworkami |
| 7 | Przewody Dupont (zestaw) | Kolorowe kabelki z plastikowymi końcówkami |

**Narzędzia:**

| Narzędzie | Do czego | Czy masz? |
|-----------|---------|-----------|
| Multimetr | Ustawienie napięcia LM2596 | ✅ |
| Komputer z USB | Wgrywanie firmware ESPHome | ✅ |
| Kabel USB do ESP32 | Programowanie | Sprawdź jaki port ma Twój ESP32 |

---

## ⚠️ ZASADY BEZPIECZEŃSTWA

Przeczytaj to raz, zanim dotkniesz czegokolwiek.

```
┌─────────────────────────────────────────────────────┐
│                   NIGDY:                            │
│                                                     │
│  ✗ NIE podłączaj ESP32 do USB i LM2596 jednocześnie │
│    → przepięcie z dwóch źródeł = martwy ESP32       │
│                                                     │
│  ✗ NIE włączaj zasilacza 12V przed ustawieniem      │
│    LM2596 na 5.0V                                   │
│    → za wysokie napięcie = martwy ESP32             │
│                                                     │
│  ✗ NIE dotykaj metalowych elementów przy            │
│    podłączonym zasilaniu                            │
└─────────────────────────────────────────────────────┘
```

**Kolejność zawsze:**
1. Najpierw montaż (bez zasilania)
2. Sprawdzenie wszystkich połączeń (multimetr)
3. Ustawienie LM2596 na 5.0V (bez ESP32!)
4. Dopiero wtedy podłączenie ESP32

---

## ETAP 1 – ZASILANIE (LM2596)

### Co to jest LM2596 i po co nam to?

**Sentinel Hub ma własny, dedykowany zasilacz 12V.** Każdy moduł Reef Sentinel Lab ma swój osobny zasilacz – moduły są elektrycznie niezależne i komunikują się wyłącznie przez WiFi.

Zasilacz daje 12V. ESP32 i wszystkie komponenty Sentinel Hub potrzebują 5V.
LM2596 to przetwornica – zmienia 12V na 5V.
Ma małą śrubkę (potencjometr) którą ustawiasz dokładne napięcie wyjściowe.

### Schemat LM2596

```
Zasilacz 12V DC  ← dedykowany dla Sentinel Hub
     │
     ├── (+) ──→ LM2596 [IN+]
     └── (–) ──→ LM2596 [IN–]
                      │
                 [potencjometr] ← tu kręcisz śrubką
                      │
              LM2596 [OUT+] ──→ 5V dla ESP32
              LM2596 [OUT–] ──→ GND (wspólna masa)
```

### Krok 1.1 – Podłącz TYLKO zasilacz do LM2596

Na razie **nic więcej** – tylko zasilacz → LM2596.

```
Zasilacz (dedykowany Hub)     LM2596
  (+) ──────────────────────→ IN+
  (–) ──────────────────────→ IN–
```

> ESP32 jeszcze NIE podłączony!

### Krok 1.2 – Ustaw multimetr

1. Sondę czarną → gniazdo **COM**
2. Sondę czerwoną → gniazdo **V/Ω**
3. Pokrętło → **DC V** (napięcie stałe), zakres **20V**

### Krok 1.3 – Zmierz i ustaw napięcie

1. Włącz zasilacz 12V
2. Czarna sonda → **OUT–** na LM2596
3. Czerwona sonda → **OUT+** na LM2596
4. Odczytaj napięcie – będzie jakieś (np. 7.2V, 9.8V – cokolwiek, to normalne)
5. Małym śrubokrętem kręć potencjometrem (śrubkę widać na płytce):
   - Zgodnie z ruchem wskazówek → napięcie rośnie
   - Przeciwnie → napięcie maleje
6. Kręć powoli (po ćwierć obrotu), obserwuj wyświetlacz multimetru
7. Cel: **5.0V** (akceptowalne: 4.9–5.1V)

```
✅ POPRAWNIE:   5.00V  lub  5.01V  lub  4.99V
⚠️ ZA MAŁO:    < 4.9V   → ESP32 może nie bootować
❌ ZA DUŻO:    > 5.1V   → ryzyko uszkodzenia ESP32
```

8. Gdy masz 5.0V → **STOP, nie ruszaj śrubki**
9. Wyłącz zasilacz

### Checklist Etap 1

```
☐ LM2596 podłączony do zasilacza 12V (IN+, IN–)
☐ Multimetr pokazuje napięcie na OUT+/OUT–
☐ Napięcie ustawione na 5.0V ±0.1V
☐ Zasilacz wyłączony przed kolejnym etapem
```

---

## ETAP 2 – SCHEMAT POŁĄCZEŃ

### Mapa pinów ESP32 (30-pin DevKit)

```
                    ┌─────────────────┐
               3V3 ─┤ 1           30 ├─ GND
               EN  ─┤ 2           29 ├─ GPIO23  (SD MOSI)
             VP/36 ─┤ 3           28 ├─ GPIO22  (OLED SCL) ←
             VN/39 ─┤ 4           27 ├─ GPIO21  (OLED SDA) ←
             GPIO34─┤ 5           26 ├─ GPIO19  (SD MISO)
             GPIO35─┤ 6           25 ├─ GPIO18  (SD SCK)
             GPIO32─┤ 7           24 ├─ GPIO5   (SD CS)
             GPIO33─┤ 8           23 ├─ GPIO17
             GPIO25─┤ 9           22 ├─ GPIO16
             GPIO26─┤ 10          21 ├─ GPIO4
             GPIO27─┤ 11          20 ├─ GPIO0
             GPIO14─┤ 12          19 ├─ GPIO2   (LED)
             GPIO12─┤ 13          18 ├─ GPIO15
               GND ─┤ 14          17 ├─ GND
              VIN  ─┤ 15          16 ├─ 3V3
                    └──────[USB]──────┘
                           ↑ port USB (do komputera / programowania)
```

**Używane piny (zaznaczone ←):**

| GPIO | Funkcja | Urządzenie |
|------|---------|-----------|
| GPIO21 | SDA | OLED (dane I2C) |
| GPIO22 | SCL | OLED (zegar I2C) |
| GPIO5  | CS  | SD Card (chip select) |
| GPIO18 | SCK | SD Card (zegar SPI) |
| GPIO19 | MISO | SD Card (dane wejście) |
| GPIO23 | MOSI | SD Card (dane wyjście) |
| GPIO2  | LED | Wbudowana dioda statusu |
| VIN/5V | Zasilanie | Z LM2596 OUT+ |
| GND    | Masa | Z LM2596 OUT– |

---

### Schemat całości – Sentinel Hub

```
ZASILACZ 12V
     │
     ├──(+12V)──→ LM2596 [IN+]
     └──( GND)──→ LM2596 [IN–]
                       │
                  LM2596 [OUT+] 5V ──────────────────────────┐
                  LM2596 [OUT–] GND ─────────────────────┐   │
                                                         │   │
                                          ┌──────────────┘   │
                                          │                  │
                                     ESP32 DevKit            │
                                     ┌──────────┐            │
                              GND ───┤ GND      │            │
                              5V  ───┤ VIN      │←───────────┘
                                     │          │
                   OLED SDA  GPIO21 ─┤          │
                   OLED SCL  GPIO22 ─┤          │
                   SD CS     GPIO5  ─┤          │
                   SD SCK    GPIO18 ─┤          │
                   SD MISO   GPIO19 ─┤          │
                   SD MOSI   GPIO23 ─┤          │
                                     └──────────┘
                                          │   │
                    ┌─────────────────────┘   └──────────────────┐
                    │                                            │
              ┌─────┴──────┐                          ┌─────────┴────────┐
              │ GROVE OLED │                          │  SD CARD READER  │
              │  0.96" I2C │                          │    (SPI moduł)   │
              │            │                          │                  │
              │ VCC ──5V   │                          │ VCC ──── 3.3V    │
              │ GND ──GND  │                          │ GND ──── GND     │
              │ SDA ──GP21 │                          │ CS  ──── GPIO5   │
              │ SCL ──GP22 │                          │ SCK ──── GPIO18  │
              └────────────┘                          │ MISO─── GPIO19   │
                                                      │ MOSI─── GPIO23   │
                                                      └──────────────────┘
```

---

### Połączenia kabel po kablu

#### LM2596 → ESP32

| Z (LM2596) | Do (ESP32) | Kolor przewodu (sugerowany) |
|-----------|-----------|---------------------------|
| OUT+ (5V) | VIN | Czerwony |
| OUT– (GND) | GND (pin 14 lub 17) | Czarny |

#### Grove OLED → ESP32

Moduł Grove OLED ma 4-pinowy złącz Grove. Użyj przewodów Dupont F-F lub adaptera Grove.

| Z (OLED) | Do (ESP32) | Kolor |
|---------|-----------|-------|
| VCC | 3V3 (pin 1 lub 16) | Czerwony |
| GND | GND | Czarny |
| SDA | GPIO21 (pin 27) | Biały/Niebieski |
| SCL | GPIO22 (pin 28) | Żółty |

> ⚠️ OLED zasilamy z **3.3V**, nie z 5V! Sprawdź oznaczenie na swojej płytce.

#### SD Card Reader → ESP32

Moduł SD Reader ma 6 pinów. Sprawdź oznaczenia na swojej płytce.

| Z (SD Reader) | Do (ESP32) | Kolor |
|--------------|-----------|-------|
| VCC/3V3 | 3V3 (pin 1 lub 16) | Czerwony |
| GND | GND | Czarny |
| CS | GPIO5 (pin 24) | Pomarańczowy |
| SCK/CLK | GPIO18 (pin 25) | Żółty |
| MISO/DO | GPIO19 (pin 26) | Niebieski |
| MOSI/DI | GPIO23 (pin 29) | Zielony |

> ⚠️ SD Reader zasilamy z **3.3V**, nie z 5V!

---

## ETAP 3 – MONTAŻ NA BREADBOARD

### Krok 3.1 – Przygotuj breadboard

Breadboard ma rzędy ponumerowane (1–30 lub podobnie) i kolumny (a–j).
Każdy rząd (a–e) i (f–j) jest wewnętrznie połączony.
Boczne szyny (oznaczone + i –) to magistrale zasilania.

```
  +  –  a b c d e   f g h i j  +  –
  │  │  │ │ │ │ │   │ │ │ │ │  │  │
1 ○  ○  ○ ○ ○ ○ ○   ○ ○ ○ ○ ○  ○  ○
2 ○  ○  ○ ○ ○ ○ ○   ○ ○ ○ ○ ○  ○  ○
  ...
```

### Krok 3.2 – Umieść ESP32

Wsuń ESP32 na środek breadboard tak, żeby piny były po obu stronach rowka.
Nie dociskaj na siłę – wchodzi bez oporu.

```
Breadboard:
   a b c d e   f g h i j
1  ○ ○ ○ ○ ○   ○ ○ ○ ○ ○
2  ○ ○ [ESP32 tu, przez środek] ○ ○
3  ○ ○ ●─●─●   ●─●─●─● ○ ○   ← piny ESP32
...
```

### Krok 3.3 – Magistrale zasilania

Podłącz przewody Dupont do szyn + i – breadboard:

- **OUT+ (5V) z LM2596** → szyna **+** breadboard (czerwony)
- **OUT– (GND) z LM2596** → szyna **–** breadboard (czarny)

Teraz możesz korzystać z szyn breadboard zamiast wkładać wszystko bezpośrednio do ESP32.

### Krok 3.4 – Podłącz OLED

Wsuń OLED na wolną część breadboard (lub użyj przewodów Dupont bezpośrednio do ESP32):

```
OLED          Breadboard/ESP32
VCC  ────────→ 3V3 (pin 1 ESP32)
GND  ────────→ GND (szyna –)
SDA  ────────→ GPIO21 (pin 27 ESP32)
SCL  ────────→ GPIO22 (pin 28 ESP32)
```

### Krok 3.5 – Podłącz SD Card Reader

```
SD Reader     Breadboard/ESP32
VCC  ────────→ 3V3 (pin 1 ESP32)
GND  ────────→ GND (szyna –)
CS   ────────→ GPIO5  (pin 24 ESP32)
SCK  ────────→ GPIO18 (pin 25 ESP32)
MISO ────────→ GPIO19 (pin 26 ESP32)
MOSI ────────→ GPIO23 (pin 29 ESP32)
```

### Checklist Etap 3

```
☐ ESP32 wsunięty na breadboard (nie dotyka krawędzi)
☐ Magistrale +/– podłączone z LM2596
☐ OLED: VCC→3V3, GND→GND, SDA→GPIO21, SCL→GPIO22
☐ SD Reader: VCC→3V3, GND→GND, CS→5, SCK→18, MISO→19, MOSI→23
☐ Żaden metalowy element się nie styka z innym (wizualna kontrola)
```

---

## ETAP 4 – KONTROLA PRZED WŁĄCZENIEM

### Krok 4.1 – Kontrola wzrokowa

Sprawdź każdy przewód po kolei:
1. Śledzisz przewodem od źródła do celu
2. Upewniasz się że siedzi stabilnie w otworku breadboard
3. Sprawdzasz czy żaden przewód nie łączy przypadkowo dwóch sąsiednich rzędów

### Krok 4.2 – Pomiar multimetrem (tryb ciągłości)

Ustaw multimetr na **tryb ciągłości** (symbol diody lub Ω z dźwiękiem):

| Test | Czarna sonda | Czerwona sonda | Oczekiwany wynik |
|------|-------------|---------------|-----------------|
| Masa OK | GND ESP32 | GND OLED | Dźwięk / 0Ω |
| Masa OK | GND ESP32 | GND SD Reader | Dźwięk / 0Ω |
| Brak zwarcia | 5V linia | GND linia | Cisza / OL |
| SDA OK | GPIO21 ESP32 | SDA OLED | Dźwięk / 0Ω |
| SCL OK | GPIO22 ESP32 | SCL OLED | Dźwięk / 0Ω |

### Checklist Etap 4

```
☐ Kontrola wzrokowa: żadne metalowe elementy się nie stykają
☐ GND ESP32 ↔ GND OLED: ciągłość OK
☐ GND ESP32 ↔ GND SD Reader: ciągłość OK
☐ 5V ↔ GND: BRAK ciągłości (brak zwarcia)
☐ GPIO21 ↔ SDA OLED: ciągłość OK
☐ GPIO22 ↔ SCL OLED: ciągłość OK
☐ GPIO5 ↔ CS SD Reader: ciągłość OK
```

---

## ETAP 5 – PIERWSZE WŁĄCZENIE (bez firmware)

### Krok 5.1 – Włącz zasilacz

1. Upewnij się że ESP32 NIE jest podłączony do USB komputera
2. Włącz zasilacz 12V
3. Obserwuj przez 5 sekund:

```
✅ DOBRZE:  Nic się nie dzieje (cisza, brak zapachu)
            LED na LM2596 świeci (jeśli ma)
⚠️ PROBLEM: Dym / zapach spalenizny → NATYCHMIAST wyłącz zasilacz
⚠️ PROBLEM: LM2596 bardzo gorący po 5s → wyłącz, sprawdź zwarcie
```

### Krok 5.2 – Sprawdź napięcia pod obciążeniem

Z włączonym zasilaczem (ESP32 jeszcze bez firmware):

| Punkt pomiarowy | Oczekiwane napięcie |
|----------------|-------------------|
| LM2596 OUT+ → OUT– | 5.0V ±0.1V |
| VIN ESP32 → GND ESP32 | 5.0V ±0.1V |
| 3V3 ESP32 → GND ESP32 | 3.3V ±0.1V |
| VCC OLED → GND | 3.3V ±0.1V |

> Jeśli 3V3 ESP32 pokazuje 3.3V, oznacza to że wewnętrzny regulator ESP32 działa poprawnie.

### Krok 5.3 – Wyłącz zasilacz

Wyłącz zasilacz 12V. Teraz możesz podłączyć USB do komputera i przejść do firmware.

---

## ETAP 6 – FIRMWARE (ESPHome)

### Krok 6.1 – Instalacja ESPHome

Jeśli masz Home Assistant OS lub Supervised:
1. Wejdź w HA → **Add-ons** → szukaj **ESPHome**
2. Zainstaluj i uruchom
3. Otwórz ESPHome dashboard

Alternatywnie (bez HA, na komputerze):
```bash
pip install esphome
```

### Krok 6.2 – Konfiguracja YAML dla Sentinel Hub

Utwórz nowe urządzenie w ESPHome i wklej poniższy config:

```yaml
esphome:
  name: sentinel-hub
  friendly_name: "Sentinel Hub"

esp32:
  board: esp32dev
  framework:
    type: arduino

# Logi (do debugowania)
logger:
  level: INFO

# API dla Home Assistant
api:
  encryption:
    key: !secret api_encryption_key

# OTA updates (aktualizacje przez WiFi)
ota:
  password: !secret ota_password

# WiFi - dual mode: AP (dla modułów) + STA (do domu)
wifi:
  # Połączenie z Twoją siecią domową
  ssid: !secret wifi_ssid
  password: !secret wifi_password

  # Hotspot dla modułów Reef Sentinel
  ap:
    ssid: "SentinelHub"
    password: "reef1234"

  # Jeśli brak połączenia z domową siecią - AP działa zawsze
  ap_timeout: 30s

# Portal konfiguracyjny (gdy brak WiFi)
captive_portal:

# MQTT broker - wbudowany (dla komunikacji między modułami)
mqtt:
  broker: 127.0.0.1
  port: 1883
  discovery: true

# OLED 0.96" I2C (SSD1306)
i2c:
  sda: GPIO21
  scl: GPIO22
  scan: true
  id: bus_i2c

display:
  - platform: ssd1306_i2c
    model: "SSD1306 128x64"
    address: 0x3C
    id: oled_display
    pages:
      - id: page_status
        lambda: |-
          it.printf(0, 0, id(font_small), "Sentinel Hub");
          it.printf(0, 12, id(font_small), "IP: %s", id(wifi_ip).state.c_str());
          it.printf(0, 24, id(font_small), "AP: SentinelHub");
          it.printf(0, 36, id(font_small), "reef-sentinel.com");
          it.printf(0, 52, id(font_tiny), "v1.0 | OK");

# Czcionki dla OLED
font:
  - file: "gfonts://Roboto"
    id: font_small
    size: 10
  - file: "gfonts://Roboto"
    id: font_tiny
    size: 8

# Informacje sieciowe (do wyświetlania na OLED)
text_sensor:
  - platform: wifi_info
    ip_address:
      id: wifi_ip
      name: "Sentinel Hub IP"
    ssid:
      name: "Sentinel Hub SSID"

# Karta SD (SPI)
spi:
  clk_pin: GPIO18
  mosi_pin: GPIO23
  miso_pin: GPIO19

# Status LED (wbudowana w ESP32)
status_led:
  pin:
    number: GPIO2
    inverted: false

# Informacje o urządzeniu
sensor:
  - platform: wifi_signal
    name: "Sentinel Hub WiFi Signal"
    update_interval: 60s
  - platform: uptime
    name: "Sentinel Hub Uptime"
    update_interval: 60s

binary_sensor:
  - platform: status
    name: "Sentinel Hub Status"

# Restart button (przez HA)
button:
  - platform: restart
    name: "Sentinel Hub Restart"
```

**Plik secrets.yaml** (utwórz osobno w ESPHome):

```yaml
wifi_ssid: "NazwaTwojejSieci"
wifi_password: "HasloTwojejSieci"
api_encryption_key: "wygeneruj-losowy-32-znakowy-klucz"
ota_password: "dowolneHasloOTA"
```

### Krok 6.3 – Wgranie firmware

1. Podłącz ESP32 do komputera kablem USB
2. **Upewnij się że zasilacz 12V jest WYŁĄCZONY**
3. W ESPHome kliknij **Install** → **Plug into this computer**
4. Wybierz port COM/ttyUSB (ESP32)
5. Poczekaj na kompilację (~2–3 min) i wgrywanie (~30s)
6. Po wgraniu w logach zobaczysz:

```
[I][wifi:290]: WiFi connected to "NazwaTwojejSieci"
[I][wifi:290]: IP: 192.168.x.x
[I][mqtt:287]: MQTT connected to 127.0.0.1
```

### Krok 6.4 – Test bezprzewodowy

1. Odłącz USB od komputera
2. Włącz zasilacz 12V
3. ESP32 bootuje z zasilania LM2596
4. Na telefonie sprawdź sieci WiFi – powinna pojawić się sieć **SentinelHub**
5. Na OLED powinien pojawić się adres IP

---

## ETAP 7 – KALIBRACJA

### Co kalibrujemy w Sentinel Hub?

Sentinel Hub sam w sobie nie ma czujników wymagających kalibracji.
Jedyne co ustawiamy to:

**7.1 Napięcie LM2596** – zrobione w Etapie 1.

**7.2 Czas systemowy** – ESPHome automatycznie synchronizuje czas przez NTP gdy jest połączenie z internetem. Brak kalibracji ręcznej.

**7.3 Weryfikacja OLED** – sprawdź czy wyświetlane dane są poprawne:
- IP powinno zgadzać się z tym co widać w routerze
- Czas powinien być poprawny po ~1 min od pierwszego połączenia z WiFi

**7.4 Weryfikacja SD Card** (jeśli używasz):

W logach ESPHome po boocie powinieneś zobaczyć:
```
[I][sd_card:050]: Mounted SD card, size: 7633 MB
```
Jeśli zobaczysz błąd `Failed to mount`, sprawdź połączenia SPI i czy karta jest sformatowana (FAT32).

---

## ETAP 8 – INTEGRACJA Z HOME ASSISTANT

### Krok 8.1 – Dodaj urządzenie do HA

1. HA automatycznie wykryje Sentinel Hub przez ESPHome API
2. Idź w HA → **Ustawienia** → **Urządzenia i usługi**
3. Zobaczysz powiadomienie o nowym urządzeniu **Sentinel Hub**
4. Kliknij **Konfiguruj** i wpisz klucz szyfrowania (z secrets.yaml)

### Krok 8.2 – Weryfikacja encji

Po dodaniu powinieneś mieć w HA:

| Encja | Typ | Opis |
|-------|-----|------|
| `sensor.sentinel_hub_ip` | Sensor | Adres IP Sentinel Hub |
| `sensor.sentinel_hub_wifi_signal` | Sensor | Siła sygnału WiFi (dBm) |
| `sensor.sentinel_hub_uptime` | Sensor | Czas pracy (s) |
| `binary_sensor.sentinel_hub_status` | Binary | Online/Offline |
| `button.sentinel_hub_restart` | Przycisk | Restart zdalny |

### Krok 8.3 – Weryfikacja MQTT

Sprawdź czy broker MQTT na Sentinel Hub działa:

W HA → **Ustawienia** → **Automatyzacje** → **Narzędzia deweloperskie** → **MQTT**:
- Subskrybuj temat: `reef/#`
- Jeśli widzisz jakiekolwiek wiadomości → MQTT działa ✅

---

## TROUBLESHOOTING

### Problem: ESP32 nie bootuje (brak świecenia LED)

```
Przyczyna 1: Za niskie napięcie z LM2596
  → Sprawdź multimetrem napięcie na VIN ESP32
  → Powinno być 4.9–5.1V

Przyczyna 2: Zły kabel USB (przy programowaniu)
  → Spróbuj innego kabla (nie wszystkie kable USB mają linię danych!)
  → Kabel do ładowania ≠ kabel do transmisji danych

Przyczyna 3: ESP32 podłączony do USB i LM2596 jednocześnie
  → Odłącz jedno ze źródeł zasilania
```

### Problem: OLED nie świeci / nie wyświetla

```
Przyczyna 1: Zły adres I2C
  → Twój OLED może mieć adres 0x3D zamiast 0x3C
  → Zmień w YAML: address: 0x3D
  → Lub włącz scan: true i sprawdź logi (pokaże znaleziony adres)

Przyczyna 2: SDA/SCL odwrócone
  → Zamień przewody GPIO21 i GPIO22

Przyczyna 3: Zasilanie z 5V zamiast 3.3V
  → Sprawdź czy VCC OLED idzie do pinu 3V3 ESP32, nie 5V

Przyczyna 4: Zły model w YAML
  → Sprawdź czy Twój OLED to SSD1306 (najpopularniejszy)
  → Inne możliwe: SH1106 (zmień platform: ssd1306_i2c na sh1106_i2c)
```

### Problem: Karta SD nie wykryta

```
Przyczyna 1: Karta nie sformatowana w FAT32
  → Sformatuj kartę w systemie jako FAT32 (nie NTFS, nie exFAT)

Przyczyna 2: Złe połączenia SPI
  → Sprawdź CS → GPIO5, SCK → GPIO18, MISO → GPIO19, MOSI → GPIO23
  → Często myli się MISO z MOSI!

Przyczyna 3: Moduł SD zasilany z 5V
  → Większość modułów SD wymaga 3.3V!
  → Sprawdź oznaczenie "3V3" lub "VCC" na module

Przyczyna 4: Karta uszkodzona
  → Przetestuj kartę w czytniku USB w komputerze
```

### Problem: ESP32 nie łączy się z WiFi

```
Przyczyna 1: Błędne hasło/SSID w secrets.yaml
  → Sprawdź dwukrotnie (uwaga na polskie znaki, spacje!)

Przyczyna 2: Sieć 5GHz zamiast 2.4GHz
  → ESP32 obsługuje TYLKO WiFi 2.4GHz
  → Upewnij się że podajesz nazwę sieci 2.4GHz

Przyczyna 3: Za słaby sygnał WiFi
  → Zbliż ESP32 do routera podczas pierwszej konfiguracji
  → Docelowo: router max 10m od ESP32 bez ścian

Przyczyna 4: Firewall routera blokuje urządzenie
  → Sprawdź w panelu routera czy ESP32 dostało adres IP
  → Dodaj do białej listy MAC jeśli potrzeba
```

### Problem: Sentinel Hub nie pojawia się w Home Assistant

```
Przyczyna 1: HA i ESP32 w różnych sieciach
  → Oba muszą być w tej samej sieci lokalnej (192.168.x.x)

Przyczyna 2: Zły klucz szyfrowania API
  → W HA usuń integrację ESPHome i dodaj ponownie z poprawnym kluczem

Przyczyna 3: ESPHome Add-on nie uruchomiony
  → HA → Add-ons → ESPHome → sprawdź czy działa
```

### Problem: MQTT nie działa (moduły się nie widzą)

```
Przyczyna 1: Moduł nie łączy się z Sentinel Hub AP
  → Sprawdź czy sieć "SentinelHub" jest widoczna na telefonie
  → Sprawdź hasło: reef1234

Przyczyna 2: Zły adres brokera w module
  → Broker jest na 10.42.0.1 (adres AP Sentinel Hub)
  → W konfiguracji innych modułów: broker: 10.42.0.1

Przyczyna 3: Firewall ESP32 / kolizja portów
  → Domyślny port MQTT: 1883 (bez SSL)
  → Sprawdź czy nic innego nie używa portu 1883
```

---

## FINALNA CHECKLIST – REEF HUB GOTOWY

```
HARDWARE:
☐ LM2596: 5.0V ±0.1V zmierzone multimetrem
☐ ESP32: boot OK (LED miga po włączeniu)
☐ OLED: wyświetla IP i status
☐ SD Card: wykryta w logach (lub pomijamy opcjonalnie)
☐ Brak zapachu spalenizny / gorących elementów

OPROGRAMOWANIE:
☐ ESPHome firmware wgrany bez błędów
☐ WiFi: połączony z siecią domową (IP przypisane)
☐ WiFi AP: "SentinelHub" widoczny na telefonie (hasło: reef1234)
☐ MQTT broker: działa na 127.0.0.1:1883

INTEGRACJA:
☐ Sentinel Hub widoczny w Home Assistant
☐ Encje aktywne (sensor.sentinel_hub_status = Online)
☐ OTA update działa (można aktualizować przez WiFi)

STATUS: ✅ Sentinel Hub gotowy do pracy!
        Następny krok: BUILD_GUIDE_sentinel_chem_monitor.md
```

---

## NASTĘPNE KROKI

Po ukończeniu Sentinel Hub:

1. **Sentinel Chem/Monitor** – gdy przyjdą brakujące komponenty (~177 zł)
2. **Konfiguracja MQTT** – połączenie między modułami
3. **Cloud sync** – konfiguracja klucza API reef-sentinel.com
4. **Deploy na akwarium** – 24/7 monitoring

---

## POWIĄZANE DOKUMENTY

- [`sentinel_hub_chem_monitor_BOM.md`](./sentinel_hub_chem_monitor_BOM.md) – Lista komponentów
- [`BUILD_GUIDE_sentinel_chem_monitor.md`](./BUILD_GUIDE_sentinel_chem_monitor.md) *(następny)* – Instrukcja montażu Modułu 2
- [`sentinel_view_BOM.md`](./sentinel_view_BOM.md) – BOM dla Sentinel View (Faza 2)

---

*Reef Sentinel – Open-source aquarium controller*  
*reef-sentinel.com | github.com/reef-sentinel*  
*Ostatnia aktualizacja: 2026-03-06*
