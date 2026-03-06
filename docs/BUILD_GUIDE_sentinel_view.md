# Reef Sentinel Lab – BUILD GUIDE
## Module 4: Sentinel View (Wyświetlacz dotykowy)

> **Version:** 1.0  
> **Data:** 2026-03-06  
> **Status:** ⏸️ Faza 2 – Q2 2026  
> **Czas montażu:** 2–3 godziny (hardware) + 1–2 dni (UI w Nextion Editor)  
> **Poziom trudności:** ⭐⭐☆☆☆ (Podstawowy – hardware prostszy niż Module 2)  
> **Wymaganie wstępne:** Sentinel Hub + Sentinel Chem/Monitor działające na akwarium ✅

---

## CO TEN MODUŁ ROBI

Sentinel View to lokalny panel dotykowy systemu Reef Sentinel Lab.  
Wyświetla dane w czasie rzeczywistym bez potrzeby otwierania telefonu czy laptopa.

**Funkcje:**
- Aktualne odczyty pH, KH, temperatury, zasolenia – z kolorowym statusem
- Wykresy trendów (7/30 dni) bezpośrednio na wyświetlaczu
- Ręczne uruchamianie testów (przycisk "Start KH Test")
- Alerty wizualne + dźwiękowe gdy parametr poza normą
- Zegar/data (RTC wbudowany w Nextion – bez baterii zewnętrznej)
- Konfiguracja harmonogramu testów

---

## ARCHITEKTURA – JAK TO DZIAŁA

```
Sentinel Hub (10.42.0.1)
        │
     WiFi MQTT
        │
   ESP32 (bridge)
   ┌────┴────────────────────────────────┐
   │  Odbiera dane z MQTT               │
   │  Formatuje komendy Nextion          │
   │  Wysyła przez UART                  │
   └────┬────────────────────────────────┘
        │ UART (TX/RX)  ← tylko 4 przewody
        │
   ┌────▼────────────────────────────────┐
   │  Nextion NX8048P050-011C            │
   │  ┌──────────────────────────────┐   │
   │  │  Własny MCU Cortex-M4 200MHz │   │
   │  │  120MB Flash (UI + grafikA)  │   │
   │  │  Pojemnościowy touch 5.0"    │   │
   │  │  RTC wbudowany               │   │
   │  └──────────────────────────────┘   │
   └─────────────────────────────────────┘
```

**Dlaczego Nextion, a nie ESP32 + LCD IPS:**

| | Nextion NX8048P050 | ESP32-S3 + IPS LCD |
|--|-------------------|-------------------|
| Podłączenie | 4 przewody (UART) | 6–8 przewodów (SPI) |
| Projektowanie UI | Drag & drop (Nextion Editor) | Kod LVGL/GFX (trudne) |
| Czas wdrożenia UI | 3–5 dni | 2–4 tygodnie |
| RAM dla renderingu | 512KB własne | Dzielone z WiFi |
| RTC | Wbudowany | Zewnętrzny moduł |
| Cena | 295 zł | ~90 zł (ale więcej pracy) |

---

## KOMPONENTY (z BOM)

| Komponent | Model | Dostawca | Cena | Uwagi |
|-----------|-------|----------|------|-------|
| Nextion Display | NX8048P050-011C | elty.pl | 295 zł | 5.0", 800×480, touch |
| ESP32 DevKit | ESP-WROOM-32, 30-pin | Botland | 25 zł | WiFi bridge |
| LM2596 | 12V→5V buck | Botland | 8 zł | ⚠️ Ustaw 5.0V! |
| Buzzer pasywny | 5V, 85dB | Botland | 3 zł | Alerty dźwiękowe |
| Przewody Dupont | F-F 20cm | Botland | 1 zł | 4 szt. do Nextion |
| Obudowa 3D | PLA/PETG, custom | Printuj3D.pl | 20 zł | Pod wymiary Nextion |
| Magnesy N52 | 10×2mm | AliExpress | 2 zł | Mocowanie magnetyczne |
| Śruby M2.5 × 6mm | + wkładki mosiężne | AliExpress | 2 zł | |

---

## ETAP 1 – ZASILANIE (LM2596)

**Sentinel View ma własny, dedykowany zasilacz 12V** – osobny od pozostałych modułów. Moduły są elektrycznie niezależne i komunikują się wyłącznie przez WiFi.

Proces ustawiania LM2596 identyczny jak w poprzednich modułach.

### Krok 1.1 – Schemat zasilania

```
Zasilacz 12V (dedykowany Sentinel View)
    │
    ├─ (+12V) ──→ LM2596 [IN+]
    └─ ( GND) ──→ LM2596 [IN–]
                      │
               LM2596 [OUT+] 5V ───┬──→ ESP32 [VIN]
               LM2596 [OUT–] GND ──┴──→ ESP32 [GND]
                                   │
                                   └──→ Nextion [5V pin 4]
                                       Nextion [GND pin 3]
```

> ⚠️ Nextion i ESP32 dzielą jedno 5V z LM2596.  
> Łączny pobór: ~750mA (Nextion 500mA + ESP32 250mA) – LM2596 daje radę.

### Krok 1.2 – Ustaw LM2596 na 5.0V

Multimetr → DC V → 20V zakres → OUT+ i OUT– → kręć potencjometrem → cel 5.0V ±0.1V.  
Dopiero po ustawieniu podłącz ESP32 i Nextion.

### Checklist Etap 1

```
☐ LM2596 IN+ ← 12V, IN– ← GND
☐ Napięcie OUT+ / OUT– = 5.0V ±0.1V
☐ Zasilacz wyłączony przed podłączeniem ESP32 i Nextion
```

---

## ETAP 2 – POŁĄCZENIE ESP32 ↔ NEXTION (UART)

### Gdzie są piny na Nextion?

Nextion NX8048P050 ma 4-pinowy złącz po lewej stronie tylnej płyty (lub kabel z 4 przewodami w zestawie):

```
Nextion (tył modułu):
┌─────────────────────────────────────┐
│                                     │
│  [TX] [RX] [GND] [5V]              │
│   1    2    3     4                 │
│                                     │
└─────────────────────────────────────┘
Pin 1: TX  (wyjście danych z Nextion)
Pin 2: RX  (wejście danych do Nextion)
Pin 3: GND
Pin 4: 5V
```

### Schemat połączeń UART

```
ESP32                    Nextion
GPIO17 (TX2) ───────────→ RX  (pin 2)
GPIO16 (RX2) ←─────────── TX  (pin 1)
GND          ───────────→ GND (pin 3)
5V (VIN)     ───────────→ 5V  (pin 4)
```

> ⚠️ Uwaga na skrzyżowanie: TX ESP32 → RX Nextion, RX ESP32 ← TX Nextion.  
> To najczęstszy błąd przy UART – jeśli Nextion nie reaguje, zamień TX i RX.

> ✅ Nextion pracuje na poziomach TTL 3.3V/5V – bezpośrednie połączenie z ESP32 (3.3V logic) jest bezpieczne. Brak potrzeby konwertera poziomów.

### Kabel po kablu

| Z (ESP32) | Do (Nextion) | Kolor sugerowany |
|-----------|-------------|-----------------|
| GPIO17 (TX2) | Pin 2 (RX) | Żółty |
| GPIO16 (RX2) | Pin 1 (TX) | Biały |
| GND (pin 14) | Pin 3 (GND) | Czarny |
| VIN/5V (pin 15) | Pin 4 (5V) | Czerwony |

### Checklist Etap 2

```
☐ ESP32 TX (GPIO17) → Nextion RX (pin 2)
☐ ESP32 RX (GPIO16) ← Nextion TX (pin 1)
☐ ESP32 GND → Nextion GND (pin 3)
☐ ESP32 VIN/5V → Nextion 5V (pin 4)
☐ Nextion podświetlenie świeci po podłączeniu zasilania
```

---

## ETAP 3 – BUZZER (OPCJONALNIE)

Buzzer pasywny podłączony do ESP32 – alerty dźwiękowe gdy parametr poza normą.

```
ESP32 GPIO25 ──→ Buzzer (+)
ESP32 GND    ──→ Buzzer (–)
```

> Buzzer pasywny (nie aktywny) – sterowany PWM, pozwala regulować ton i czas trwania dźwięku.

---

## ETAP 4 – GPIO ASSIGNMENT

```
                    ┌─────────────────┐
               3V3 ─┤ 1           30 ├─ GND
               EN  ─┤ 2           29 ├─ GPIO23
             VP/36 ─┤ 3           28 ├─ GPIO22
             VN/39 ─┤ 4           27 ├─ GPIO21
             GPIO34─┤ 5           26 ├─ GPIO19
             GPIO35─┤ 6           25 ├─ GPIO18
             GPIO32─┤ 7           24 ├─ GPIO5
             GPIO33─┤ 8           23 ├─ GPIO17  ← Nextion RX
             GPIO25─┤ 9  ESP32    22 ├─ GPIO16  ← Nextion TX
  Buzzer     GPIO26─┤ 10          21 ├─ GPIO4
             GPIO27─┤ 11          20 ├─ GPIO0
             GPIO14─┤ 12          19 ├─ GPIO2   ← LED status
             GPIO12─┤ 13          18 ├─ GPIO15
               GND ─┤ 14          17 ├─ GND
         5V   VIN  ─┤ 15          16 ├─ 3V3
                    └──────[USB]──────┘
```

| GPIO | Funkcja | Uwagi |
|------|---------|-------|
| GPIO16 | Nextion TX (UART2 RX) | Odbiera dane z Nextion |
| GPIO17 | Nextion RX (UART2 TX) | Wysyła dane do Nextion |
| GPIO25 | Buzzer PWM | Alerty dźwiękowe |
| GPIO2 | Status LED | Wbudowana dioda |
| WiFi | SentinelHub MQTT | Połączenie z Hub |

**GPIO użytych: 3 – minimalistyczne ✅**

---

## ETAP 5 – FIRMWARE ESP32 (ESPHome)

### Konfiguracja YAML

```yaml
esphome:
  name: sentinel-view
  friendly_name: "Sentinel View"

esp32:
  board: esp32dev
  framework:
    type: arduino

logger:
  level: INFO
  # Wyłącz logi UART2 żeby nie zakłócały komunikacji z Nextion
  baud_rate: 0

api:
  encryption:
    key: !secret api_encryption_key

ota:
  password: !secret ota_password

wifi:
  ssid: "SentinelHub"
  password: "reef1234"

mqtt:
  broker: 10.42.0.1
  port: 1883
  topic_prefix: reef/view
  on_message:
    # Odbierz pH od Sentinel Chem i wyślij do Nextion
    - topic: reef/chem/ph/value
      then:
        - uart.write:
            id: nextion_uart
            data: !lambda |-
              std::string cmd = "pH.txt=\"" + x + "\"";
              cmd += "\xff\xff\xff";
              return std::vector<uint8_t>(cmd.begin(), cmd.end());
    # Odbiór temperatury
    - topic: reef/chem/temp/aquarium
      then:
        - uart.write:
            id: nextion_uart
            data: !lambda |-
              std::string cmd = "temp1.txt=\"" + x + "°C\"";
              cmd += "\xff\xff\xff";
              return std::vector<uint8_t>(cmd.begin(), cmd.end());
    # Odbiór KH
    - topic: reef/chem/kh/value
      then:
        - uart.write:
            id: nextion_uart
            data: !lambda |-
              std::string cmd = "kh.txt=\"" + x + " dKH\"";
              cmd += "\xff\xff\xff";
              return std::vector<uint8_t>(cmd.begin(), cmd.end());

# UART2 do komunikacji z Nextion
uart:
  id: nextion_uart
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 9600

# Buzzer
output:
  - platform: ledc
    pin: GPIO25
    id: buzzer_output

rtttl:
  output: buzzer_output
  id: buzzer

# Status LED
status_led:
  pin:
    number: GPIO2
    inverted: false

binary_sensor:
  - platform: status
    name: "Sentinel View Status"

button:
  - platform: restart
    name: "Sentinel View Restart"
  # Przycisk testu dźwięku (z HA)
  - platform: template
    name: "Sentinel View Buzzer Test"
    on_press:
      - rtttl.play: "alert:d=8,o=5,b=180:c,e,g"
```

### Krok 5.1 – Wgraj firmware

1. Podłącz ESP32 do komputera USB (**zasilacz 12V wyłączony**)
2. ESPHome → Install → Plug into this computer
3. Poczekaj na kompilację i wgranie
4. W logach po boocie:
```
[I][wifi:290]: Connected to SentinelHub
[I][mqtt:287]: MQTT connected to 10.42.0.1
```

---

## ETAP 6 – UI W NEXTION EDITOR

### Krok 6.1 – Pobierz Nextion Editor

- Adres: `nextion.com/nextion-editor`
- Wersja: v1.67 lub nowsza
- System: Windows (lub Wine na Linux/Mac)
- Koszt: bezpłatny

### Krok 6.2 – Utwórz nowy projekt

1. Uruchom Nextion Editor
2. **New Project** → wybierz model: `NX8048P050_011C` (P-series, 5.0")
3. Orientacja: **Landscape** (pozioma)
4. Encoding: **UTF-8**

### Krok 6.3 – Planowane ekrany

```
┌─────────────────────────────────────────────────────────┐
│                   SENTINEL VIEW                         │
│              Mapa ekranów (5 stron)                     │
│                                                         │
│  [HOME] ←→ [CHARTS] ←→ [TESTS] ←→ [SETTINGS] ←→ [ALERTS] │
│     0          1           2           3           4    │
│                                                         │
│  Nawigacja: pasek dolny z ikonami (stały na wszystkich) │
└─────────────────────────────────────────────────────────┘
```

**Strona 0 – HOME (główny widok):**
```
┌────────────────────────────────────────┐
│  🐠 Reef Sentinel   14:32  ●●●WiFi    │  ← header
├────────────┬────────────┬──────────────┤
│    pH      │    KH      │    TEMP      │
│   8.15     │  8.2 dKH   │   25.3°C    │
│   ●OK      │   ●OK      │    ●OK      │
├────────────┴────────────┴──────────────┤
│   Zasolenie: 35.1 ppt    EC: 52.8 mS  │
├────────────────────────────────────────┤
│  Ostatni test KH: dziś 08:00           │
│  Następny: jutro 08:00                 │
├────────────────────────────────────────┤
│  [⌂ Home] [📈 Charts] [🧪 Tests] [⚙️] │  ← nawigacja
└────────────────────────────────────────┘
```

**Strona 1 – CHARTS:**
```
┌────────────────────────────────────────┐
│  📈 Wykresy trendów        [7d] [30d]  │
├────────────────────────────────────────┤
│  pH ────────────────────────────────── │
│   8.20 ┤                               │
│   8.10 ┤  ~~~~~~~~~~~~~~~~~~~~         │
│   8.00 ┤                               │
├────────────────────────────────────────┤
│  KH ────────────────────────────────── │
│   8.5 ┤    ●    ●    ●    ●    ●      │
│   8.0 ┤                               │
├────────────────────────────────────────┤
│  [⌂ Home] [📈 Charts] [🧪 Tests] [⚙️] │
└────────────────────────────────────────┘
```

**Strona 2 – TESTS:**
```
┌────────────────────────────────────────┐
│  🧪 Testy ręczne                       │
├────────────────────────────────────────┤
│                                        │
│  ┌──────────────────────────────────┐  │
│  │   ▶ START TEST KH               │  │
│  └──────────────────────────────────┘  │
│                                        │
│  ┌──────────────────────────────────┐  │
│  │   ▶ START POMIAR pH             │  │
│  └──────────────────────────────────┘  │
│                                        │
│  Status: Oczekiwanie...               │
│  Postęp: [░░░░░░░░░░░░░░░░░░░░]  0%   │
├────────────────────────────────────────┤
│  [⌂ Home] [📈 Charts] [🧪 Tests] [⚙️] │
└────────────────────────────────────────┘
```

**Strona 3 – SETTINGS:**
```
┌────────────────────────────────────────┐
│  ⚙️ Ustawienia                         │
├────────────────────────────────────────┤
│  Test KH: co [3] dni  [–] [+]         │
│  pH monit: co [4] godz  [–] [+]       │
│  Alert pH min: [7.9]    [–] [+]       │
│  Alert pH max: [8.4]    [–] [+]       │
│  Alert KH min: [7.0]    [–] [+]       │
│  Alert temp max: [27.0]  [–] [+]      │
│                                        │
│  [ZAPISZ]         [RESET DO DOMYŚLNYCH]│
├────────────────────────────────────────┤
│  [⌂ Home] [📈 Charts] [🧪 Tests] [⚙️] │
└────────────────────────────────────────┘
```

### Krok 6.4 – Upload UI na Nextion

**Metoda 1 – przez UART (kabel USB-TTL):**
1. Podłącz USB-TTL adapter: TX→RX Nextion, RX→TX Nextion, GND→GND, 5V→5V
2. W Nextion Editor: **Upload** → wybierz port COM → kliknij Upload
3. Poczekaj na transfer (~1–2 min przy 9600 baud)

**Metoda 2 – przez kartę microSD (szybsza):**
1. W Nextion Editor: **File** → **TFT file output** → zapisz plik `.tft`
2. Skopiuj plik `.tft` na kartę microSD (FAT32, max 32GB)
3. Włóż kartę do gniazda microSD w Nextion
4. Włącz zasilanie – Nextion automatycznie wykryje i zainstaluje UI
5. Po instalacji wyciągnij kartę (Nextion nie bootuje z kartą w środku)

> ✅ Metoda microSD jest szybsza i wygodniejsza przy kolejnych aktualizacjach UI.

### Checklist Etap 6

```
☐ Nextion Editor zainstalowany
☐ Projekt skonfigurowany dla modelu NX8048P050_011C
☐ Strona HOME: etykiety pH, KH, temp, zasolenie
☐ Strona TESTS: przyciski start testu
☐ Strona SETTINGS: pola konfiguracji
☐ UI wgrany na Nextion (UART lub microSD)
☐ Nextion wyświetla UI po włączeniu
```

---

## ETAP 7 – OBUDOWA

### Wymiary Nextion NX8048P050-011C

```
┌──────────────────────────────────────┐
│         132.0 mm                     │
│  ┌──────────────────────────────┐    │  82.8mm
│  │  Obszar wyświetlacza         │    │
│  │  122.0 × 72.6 mm             │    │
│  └──────────────────────────────┘    │
└──────────────────────────────────────┘
Grubość modułu: 12.9 mm
```

### Projekt obudowy

```
Widok z przodu:                    Widok z tyłu:
┌────────────────────┐             ┌────────────────────┐
│  ┌──────────────┐  │             │  [ESP32]           │
│  │              │  │             │  [LM2596]          │
│  │   NEXTION    │  │             │  [Buzzer]          │
│  │   5.0"       │  │             │                    │
│  │              │  │             │  ○ M2.5  ○ M2.5   │
│  └──────────────┘  │             │                    │
└────────────────────┘             │  ○ M2.5  ○ M2.5   │
                                   └────────────────────┘
```

**Wskazówki do druku 3D:**
- Materiał: PETG (lepszy niż PLA przy akwarium – wilgoć, temperatura)
- Kolor: Biały lub czarny (czarny lepiej maskuje złącza)
- Otwór na wyświetlacz: dokładnie 122.0 × 72.6 mm + 0.3mm tolerancja
- Otwory montażowe Nextion: wg datasheetu (2× M3 po bokach)
- Przejście kabla: od tyłu, z opaską kablową

**Podstawka magnetyczna:**
```
[Obudowa Sentinel View]
        │
  [Przegub M6]   ← regulowany kąt (0–90°)
        │
  [Podstawka z magnesami N52]
   🧲 🧲 🧲 🧲
```
Magnesy N52 10×2mm przyklejone epoksydem do podstawki → montaż na boku szafki akwaryjnej.

### Checklist Etap 7

```
☐ Obudowa wydrukowana lub zamówiona
☐ Nextion wklejony/przykręcony do frontu
☐ ESP32 i LM2596 umieszczone z tyłu
☐ Kabel UART przeprowadzony wewnętrznie
☐ Podstawka magnetyczna: magnesy przyklejone epoksydem
☐ Przegub M6 zamontowany
```

---

## TROUBLESHOOTING

### Problem: Nextion nie wyświetla nic po włączeniu

```
Przyczyna 1: Brak zasilania 5V
  → Sprawdź multimetrem napięcie na pin 4 (5V) Nextion

Przyczyna 2: UI nie wgrany
  → Nowy Nextion wyświetla ekran demonstracyjny bez UI
  → Wgraj projekt przez microSD lub UART

Przyczyna 3: UI wgrany dla złego modelu
  → Projekt MUSI być stworzony dla NX8048P050_011C
  → Plik .tft z innego modelu nie zadziała
```

### Problem: Nextion wyświetla UI ale dane się nie aktualizują

```
Przyczyna 1: TX i RX odwrócone
  → Zamień GPIO16 ↔ GPIO17 w fizycznym połączeniu
  → To najczęstszy błąd UART!

Przyczyna 2: ESP32 nie łączy się z SentinelHub MQTT
  → Sprawdź logi ESPHome – czy widać "MQTT connected"

Przyczyna 3: Błędna nazwa etykiety w YAML
  → Nazwa w YAML (np. pH.txt) musi dokładnie odpowiadać
    nazwie komponentu na stronie Nextion Editor
```

### Problem: Nextion się zawiesza lub restartuje

```
Przyczyna 1: Niedomagające zasilanie
  → Sprawdź czy LM2596 daje stabilne 5V pod obciążeniem
  → Nextion pobiera do 500mA – upewnij się że LM2596 ma
    rezerwę (używaj modelu 3A, nie 1A)

Przyczyna 2: Przekroczenie limitu danych UART
  → Zbyt częste wysyłanie komend (częściej niż co 100ms)
  → Dodaj delay między komendami w YAML
```

### Problem: Dotyk nie reaguje

```
Przyczyna 1: Folia ochronna na ekranie
  → NX8048P050-011C może mieć folię – zdejmij ją!

Przyczyna 2: Wilgoć na ekranie
  → Zetrzyj, poczekaj aż wyschnie

Przyczyna 3: Zły model w projekcie Nextion Editor
  → Pojemnościowy touch wymaga prawidłowego modelu w projekcie
```

---

## FINALNA CHECKLIST – SENTINEL VIEW GOTOWY

```
HARDWARE:
☐ LM2596: 5.0V ±0.1V
☐ Nextion podświetlenie świeci
☐ UART TX/RX: dane płyną (nie odwrócone)
☐ Buzzer: reaguje na komendę testową z HA

OPROGRAMOWANIE:
☐ ESPHome firmware wgrany
☐ Połączenie z SentinelHub MQTT aktywne
☐ UI Nextion wgrany (wszystkie 5 stron)
☐ Dane z Sentinel Chem/Monitor aktualizują się na ekranie

MECHANIKA:
☐ Obudowa zamontowana
☐ Podstawka magnetyczna trzyma pewnie
☐ Kąt widzenia wygodny (przegub ustawiony)

STATUS: ✅ Sentinel View gotowy!
```

---

## POWIĄZANE DOKUMENTY

- [`sentinel_view_BOM.md`](./sentinel_view_BOM.md) – Lista komponentów
- [`NEXTION_UI_GUIDE.md`](./NEXTION_UI_GUIDE.md) *(do stworzenia)* – Szczegółowy przewodnik po Nextion Editor
- [`BUILD_GUIDE_sentinel_hub.md`](./BUILD_GUIDE_sentinel_hub.md) – Module 1

---

*Reef Sentinel Lab – Open-source aquarium controller*  
*reef-sentinel.com | github.com/reef-sentinel*  
*Ostatnia aktualizacja: 2026-03-06*
