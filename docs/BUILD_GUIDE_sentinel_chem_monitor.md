# Reef Sentinel Lab – BUILD GUIDE
## Module 2: Sentinel Chem + Sentinel Monitor

> **Version:** 1.0  
> **Data:** 2026-03-06  
> **Czas montażu:** 3–5 godzin (w 2–3 sesjach)  
> **Poziom trudności:** ⭐⭐⭐☆☆ (Średni)  
> **Wymaganie wstępne:** Sentinel Hub (Module 1) zmontowany i działający ✅

---

## CO TEN MODUŁ ROBI

Fizycznie to **jedno urządzenie** (jeden ESP32), które marketingowo pełni dwie role:

**Sentinel Monitor** – ciągły monitoring 24/7:
- Temperatura × 3 (akwarium, sump, komora pomiarowa) – DS18B20
- Zasolenie/EC (z kompensacją temperatury) – ciągłe

**Sentinel Chem** – automatyczne testy chemiczne:
- pH monitoring co 15 min (z kompensacją temperatury)
- Automatyczna titracja KH (HCl 0.1M, ~10 min, 1–7× dziennie)
- Magnetyczne mieszadełko (PWM)
- Sonda pH przechowywana w wodzie RO między pomiarami → żywotność 18–36 mies.

---

## PLAN MONTAŻU – ETAPY

Ten moduł budujemy **w dwóch fazach**, bo nie wszystkie komponenty są jeszcze dostępne:

```
FAZA A (TERAZ – komponenty dostępne):
  ├─ Etap 1: Zasilanie (LM2596)
  ├─ Etap 2: ESP32 + czujniki pasywne (DS18B20 × 3, EC)
  ├─ Etap 3: Sensor pH (gdy dojedzie)
  └─ Etap 4: Firmware + pierwsze odczyty

FAZA B (po dostawie brakujących części ~177 zł):
  ├─ Etap 5: MOSFET × 5 + pompki × 4
  ├─ Etap 6: Mieszadełko magnetyczne (DIY)
  └─ Etap 7: Komora pomiarowa + pojemniki
```

---

## ⚠️ ZASADY BEZPIECZEŃSTWA

```
┌──────────────────────────────────────────────────────────────┐
│                        NIGDY:                                │
│                                                              │
│  ✗ NIE podłączaj ESP32 do USB i LM2596 jednocześnie          │
│    → dwa źródła zasilania = uszkodzony ESP32                 │
│                                                              │
│  ✗ NIE włączaj pompki #2 (HCl) bez upewnienia się,          │
│    że przewód jest skierowany w odpowiednie miejsce          │
│    → HCl na obwodach elektronicznych = awaria               │
│                                                              │
│  ✗ NIE testuj z HCl zanim nie przetestujesz z wodą RO        │
│    → najpierw mechanika, potem chemia                        │
│                                                              │
│  ✗ NIE zamieniaj GND ESP32 i COM MOSFET                      │
│    → bez wspólnej masy pompki nie działają                   │
│                                                              │
│  ✗ BNC kabla pH NIE prowadź obok kabli 230V                  │
│    → zakłócenia = błędne odczyty pH                         │
└──────────────────────────────────────────────────────────────┘
```

**Praca z HCl – bezwzględne zasady:**
- Rękawice lateksowe lub nitrylowe zawsze przy pracy z kwasem
- Okulary ochronne
- Praca w wentylowanym miejscu
- Przechowywanie w zamkniętym pojemniku, z dala od dzieci
- Przy rozlaniu – duża ilość wody, nie wycierać ściereczką

---

## MAPA PINÓW ESP32 – MODULE 2

```
                    ┌─────────────────┐
               3V3 ─┤ 1           30 ├─ GND
               EN  ─┤ 2           29 ├─ GPIO23
             VP/36 ─┤ 3           28 ├─ GPIO22
             VN/39 ─┤ 4           27 ├─ GPIO21
    pH (ADC) GPIO34─┤ 5           26 ├─ GPIO19
             GPIO35─┤ 6           25 ├─ GPIO18
    EC (ADC) GPIO32─┤ 7           24 ├─ GPIO5
  DS18B20    GPIO33─┤ 8           23 ├─ GPIO17
  Stirrer    GPIO25─┤ 9           22 ├─ GPIO16  Pompka #4 (RO)
             GPIO26─┤ 10          21 ├─ GPIO4   Pompka #3 (odpad)
             GPIO27─┤ 11          20 ├─ GPIO0
             GPIO14─┤ 12          19 ├─ GPIO2   Pompka #2 (HCl) ⚠️LED
             GPIO12─┤ 13          18 ├─ GPIO15  Pompka #1 (próbka)
               GND ─┤ 14          17 ├─ GND
              VIN  ─┤ 15          16 ├─ 3V3
                    └──────[USB]──────┘
```

**Tabela GPIO:**

| GPIO | Funkcja | Urządzenie | Typ sygnału | Uwagi |
|------|---------|-----------|-------------|-------|
| GPIO34 | pH sensor | DFRobot Gravity pH V2 | ADC1 analog input | Tylko INPUT! |
| GPIO32 | EC sensor | DFRobot Gravity TDS | ADC1 analog input | Tylko INPUT! |
| GPIO33 | DS18B20 (1-Wire) | 3× DS18B20 na jednym pin | Digital 1-Wire | Pull-up 4.7kΩ! |
| GPIO25 | Stirrer PWM | Silnik mieszadełka | PWM output | 0–100% duty |
| GPIO15 | Pompka #1 | Próbka z akwarium | Digital output | MOSFET SIG |
| GPIO2  | Pompka #2 | HCl (titracja) | Digital output | ⚠️ Ma wbudowane LED |
| GPIO4  | Pompka #3 | Odpad/wypompowanie | Digital output | MOSFET SIG |
| GPIO16 | Pompka #4 | Woda RO (płukanie) | Digital output | MOSFET SIG |

> **Dlaczego GPIO34 i GPIO32 dla czujników analogowych?**  
> GPIO34–39 na ESP32 to piny wyłącznie wejściowe (INPUT only) – nie możesz ich przez pomyłkę ustawić jako wyjście. Idealne dla delikatnych sensorów analogowych.

> **GPIO2 i wbudowane LED:**  
> GPIO2 ma wbudowaną niebieską diodę. Podczas bootowania może migać – to normalne. Przy sterowaniu pompką #2 dioda będzie świecić gdy pompka pracuje. To przydatna wizualna informacja.

---

## ETAP 1 – ZASILANIE (LM2596)

**Sentinel Chem/Monitor ma własny, dedykowany zasilacz 12V/3A** – osobny od Sentinel Hub i pozostałych modułów. Moduły są elektrycznie niezależne i komunikują się wyłącznie przez WiFi.

Proces ustawiania LM2596 identyczny jak w Sentinel Hub. Jeśli już go robiłeś – wiesz jak.

### Krok 1.1 – Podłącz zasilacz → LM2596 (bez ESP32!)

```
Zasilacz 12V/3A (dedykowany Sentinel Chem/Monitor)
   (+) ─────────────→ LM2596 IN+
   (–) ─────────────→ LM2596 IN–
```

### Krok 1.2 – Ustaw napięcie na 5.0V

1. Multimetr: DC V, zakres 20V
2. Czarna sonda → OUT–, czerwona → OUT+
3. Kręć potencjometrem aż wyświetli **5.0V ±0.1V**
4. Wyłącz zasilacz

### Checklist Etap 1

```
☐ LM2596 IN+ ← zasilacz (+)
☐ LM2596 IN– ← zasilacz (–)
☐ Napięcie OUT+/OUT– = 5.0V ±0.1V (zmierzone multimetrem)
☐ Zasilacz wyłączony przed kolejnym etapem
```

---

## ETAP 2 – DS18B20 × 3 (TEMPERATURA)

### Czym jest DS18B20 i dlaczego trzy sztuki?

DS18B20 to wodoodporna sonda temperaturowa. Masz trzy, bo mierzymy w trzech miejscach:
- **Sonda #1** – w akwarium (główny zbiornik)
- **Sonda #2** – w sumpie
- **Sonda #3** – w komorze pomiarowej (do kompensacji temperatury pH/EC)

Wszystkie trzy podłączone do **jednego pinu GPIO33** – protokół 1-Wire pozwala na to.

### Co to jest rezystor pull-up 4.7kΩ i dlaczego jest konieczny?

Protokół 1-Wire wymaga żeby linia DATA była „podciągnięta" do VCC przez rezystor.
Bez niego sonda nie odpowiada lub daje losowe odczyty.
DFRobot Gravity adapter ma rezystor 10kΩ – za duży dla DS18B20. Dlatego potrzebny jest osobny rezystor 4.7kΩ.

### Schemat DS18B20

```
ESP32 3V3 ────┬──────────────────────────── VCC (czerwony) DS18B20 #1
              │                             VCC (czerwony) DS18B20 #2
              │                             VCC (czerwony) DS18B20 #3
             [4.7kΩ]  ← rezystor pull-up
              │
ESP32 GPIO33 ─┴──────────────────────────── DATA (żółty)  DS18B20 #1
                                            DATA (żółty)  DS18B20 #2
                                            DATA (żółty)  DS18B20 #3

ESP32 GND ───────────────────────────────── GND (czarny)  DS18B20 #1
                                            GND (czarny)  DS18B20 #2
                                            GND (czarny)  DS18B20 #3
```

Wszystkie trzy sondy łączą się **równolegle** na tych samych liniach.

### Montaż na breadboard krok po kroku

**Krok 2.1 – Przygotuj rezystor 4.7kΩ**

Rezystor wygląda tak (kolorowe paski na małej rurce):
```
  ╔════════════╗
──╢ żółty-fioletowy-czerwony-złoty ╟──
  ╚════════════╝
```
Paski: żółty = 4, fioletowy = 7, czerwony = ×100, złoty = ±5% → razem 4700Ω = 4.7kΩ.
Rezystor nie ma kierunku – możesz go wstawić w dowolną stronę.

**Krok 2.2 – Umieść rezystor na breadboard**

```
Breadboard (widok z góry):
         a   b   c   d   e       f   g   h   i   j
    1  [ ][ ][ ][ ][ ]     [ ][ ][ ][ ][ ]
    2  [ ][ ][ ][ ][ ]     [ ][ ][ ][ ][ ]
    3  [R]─────────────────────────────[R]   ← rezystor 4.7kΩ między rzędami a3 i e3
    4  [ ][ ][ ][ ][ ]     [ ][ ][ ][ ][ ]
    5  [ ][ ][ ][ ][ ]     [ ][ ][ ][ ][ ]
```

Wbij jedną nóżkę rezystora w rząd `a3`, drugą w `e3` (przez środkową przerwę).

**Krok 2.3 – Magistrale zasilania na breadboard**

```
Szyna (+) breadboard ← przewód ← 3V3 ESP32 (pin 1 lub 16)
Szyna (–) breadboard ← przewód ← GND ESP32 (pin 14 lub 17)
```

**Krok 2.4 – Podłącz rezystor do magistral**

```
Rząd a3 (nóżka 1 rezystora) → przewód → szyna (+) [3V3]
Rząd e3 (nóżka 2 rezystora) → przewód → GPIO33 ESP32 (pin 8)
```

**Krok 2.5 – Podłącz DS18B20 #1**

| Przewód sondy | Kolor | Dokąd na breadboard |
|--------------|-------|---------------------|
| VCC | Czerwony | Szyna (+) [3V3] |
| DATA | Żółty | Rząd e3 (tam gdzie nóżka rezystora i GPIO33) |
| GND | Czarny | Szyna (–) [GND] |

**Krok 2.6 – Podłącz DS18B20 #2 i #3**

Identycznie jak #1 – VCC do szyny (+), DATA do rzędu e3 (lub bezpośrednio do GPIO33), GND do szyny (–).

```
┌────────────────────────────────────────────────┐
│              BREADBOARD                        │
│  (+)───3V3───────────────── VCC DS18B20 #1     │
│  (+)───3V3───────────────── VCC DS18B20 #2     │
│  (+)───3V3───────────────── VCC DS18B20 #3     │
│                                                │
│  (+)───[4.7kΩ]───GPIO33 ─── DATA DS18B20 #1   │
│                  GPIO33 ─── DATA DS18B20 #2    │
│                  GPIO33 ─── DATA DS18B20 #3    │
│                                                │
│  (–)───GND───────────────── GND DS18B20 #1     │
│  (–)───GND───────────────── GND DS18B20 #2     │
│  (–)───GND───────────────── GND DS18B20 #3     │
└────────────────────────────────────────────────┘
```

### Checklist Etap 2

```
☐ Rezystor 4.7kΩ wbity na breadboard (między szyna 3V3 a GPIO33)
☐ DS18B20 #1: VCC→3V3, DATA→GPIO33, GND→GND
☐ DS18B20 #2: VCC→3V3, DATA→GPIO33, GND→GND
☐ DS18B20 #3: VCC→3V3, DATA→GPIO33, GND→GND
☐ Brak zwarcia 3V3 ↔ GND (sprawdź multimetrem, tryb ciągłości: cisza = OK)
```

---

## ETAP 3 – EC/TDS SENSOR (ZASOLENIE)

### Schemat DFRobot Gravity TDS

Moduł ma 3 przewody (złącze Grove lub Dupont):

```
DFRobot TDS        ESP32
  VCC ────────────→ 5V (VIN, pin 15)
  GND ────────────→ GND
  AO  ────────────→ GPIO32 (pin 7)
```

> ⚠️ EC sensor zasilamy z **5V** (nie 3.3V) – sprawdź oznaczenie VCC na module.

### Kondensatory filtrujące (ważne dla dokładności!)

Bez kondensatorów odczyty ADC będą niestabilne i skaczące.

```
Kondensator 100µF/25V:
  (+) → linia 5V (jak najbliżej pinu VCC modułu)
  (–) → GND

Kondensator 100nF (ceramiczny):
  (dowolna strona) → między linią AO a GND
```

Na breadboard – wstaw kondensatory w bezpośrednim sąsiedztwie modułu EC.

### Checklist Etap 3

```
☐ EC sensor: VCC→5V, GND→GND, AO→GPIO32
☐ Kondensator 100µF/25V: między linią 5V a GND (blisko modułu)
☐ Kondensator 100nF: między AO a GND (blisko GPIO32)
```

---

## ETAP 4 – pH SENSOR (DFRobot Gravity V2)

> Wykonaj gdy sensor dojedzie.

### Zestaw DFRobot Gravity pH V2 zawiera:
- Płytkę wzmacniacza (amplifier board)
- Sondę pH ze złączem BNC
- Bufor kalibracyjny pH 4.0 i pH 7.0 (małe saszetki lub buteleczki)

### Schemat

```
DFRobot pH V2        ESP32
  VCC ─────────────→ 5V (VIN, pin 15)
  GND ─────────────→ GND
  AO  ─────────────→ GPIO34 (pin 5)

Sonda pH → złącze BNC na amplifier board
```

> ⚠️ Kabel BNC (od sondy do amplifier board) prowadź z DALA od innych przewodów, szczególnie od linii 12V i GND MOSFET. Zakłócenia elektromagnetyczne fałszują odczyty pH.

### Kondensator filtrujący pH

```
Kondensator 100µF/25V:
  (+) → 5V linia (blisko modułu pH)
  (–) → GND
```

### Checklist Etap 4

```
☐ pH amplifier board: VCC→5V, GND→GND, AO→GPIO34
☐ Sonda podłączona do złącza BNC na amplifier board
☐ Kabel BNC oddalony od kabli 12V i MOSFET
☐ Kondensator 100µF/25V przy zasilaniu modułu
☐ Sonda zanurzona w wodzie RO (przechowywanie między pomiarami)
```

---

## ETAP 5 – MOSFET × 5 + POMPKI × 4

### Jak działa MOSFET DFRobot Gravity (low-side switching)

MOSFET to elektroniczny przełącznik. Pompka 12V jest za ciężka dla ESP32 (max 40mA na pin). MOSFET pozwala ESP32 sterować dużymi obciążeniami małym sygnałem.

**Low-side switching** – MOSFET przerywa obwód od strony masy (–):

```
Zasilacz 12V (+) ────────────────────→ (+) Pompka
Zasilacz 12V (–) ───→ MOSFET [OUT–] ──→ (–) Pompka
                       MOSFET [SIG]  ←── GPIO ESP32  (sygnał sterujący)
                       MOSFET [GND]  ←── GND ESP32   ← KRYTYCZNE!
                       MOSFET [V+]   ←── 12V zasilacz (strona mocy)
```

### ⚠️ COMMON GROUND – najważniejsza zasada

```
┌─────────────────────────────────────────────────────────┐
│  GND ESP32  MUSI być połączony z GND/COM każdego MOSFET │
│                                                         │
│  Bez tego: ESP32 i MOSFET „nie rozumieją się"           │
│  → pompki nie reagują na sygnał                         │
│  → możliwe uszkodzenie ESP32                            │
│                                                         │
│  Zrób to JEDNYM przewodem od szyny GND breadboard       │
│  do COM każdego MOSFET (lub połącz COM-y ze sobą)       │
└─────────────────────────────────────────────────────────┘
```

### Schemat wszystkich 5 MOSFET

```
                    ESP32
                      │
   ┌──────────────────┼──────────────────────────────┐
   │                  │ GND (wspólna masa)           │
   │          ┌───────┼───────────────────────┐      │
   │          │       │                       │      │
   │         GND     GND                     GND    GND
   │          │       │                       │      │
  SIG       COM     COM                     COM    COM
GPIO15   MOSFET#1  MOSFET#2               MOSFET#3 MOSFET#4
  │         │         │                       │      │
 MOSFET#1  OUT      OUT                     OUT    OUT
  │         │         │                       │      │
Pompka#1  Pompka#1  Pompka#2              Pompka#3 Pompka#4
(próbka)   (–)      HCl (–)              odpad(–)  RO (–)
```

Uproszczony schemat jednego MOSFET + pompki:

```
12V (+) ──────────────────────────────────→ Pompka (+)
12V (–) ──→ [MOSFET V+]   [MOSFET OUT–] ──→ Pompka (–)
             [MOSFET SIG] ←── GPIO ESP32
             [MOSFET GND] ←── GND ESP32  ← MUSI BYĆ!
```

### Połączenia kabel po kablu – każdy MOSFET

| Z | Do | Kolor |
|---|----|----|
| MOSFET SIG | GPIO ESP32 (wg tabeli poniżej) | Żółty/biały |
| MOSFET GND/COM | GND ESP32 (szyna –) | Czarny |
| MOSFET V+ | 12V (+) zasilacz | Czerwony (gruby) |
| MOSFET OUT– | (–) pompki | Czarny (gruby) |
| 12V (+) zasilacz | (+) pompki | Czerwony (gruby) |

### Przypisanie GPIO → MOSFET → Pompka

| MOSFET # | GPIO | Pompka | Rola | Kiedy aktywna |
|---------|------|--------|------|--------------|
| #1 | GPIO15 | Pompka #1 | Pobieranie próbki z akwarium | Test pH/KH start |
| #2 | GPIO2 | Pompka #2 | Dozowanie HCl | Titracja KH |
| #3 | GPIO4 | Pompka #3 | Wypompowanie odpadu | Po teście |
| #4 | GPIO16 | Pompka #4 | Wpompowanie wody RO | Przechowywanie sondy |
| #5 | GPIO25 | Stirrer (silnik) | Mieszadełko magnetyczne | Podczas titracji |

> Mieszadełko (MOSFET #5 / GPIO25) sterowane jest PWM – nie jest to zwykłe ON/OFF, ale płynna regulacja prędkości przez zmianę wypełnienia sygnału.

### Checklist Etap 5

```
☐ MOSFET #1 (GPIO15): SIG→GPIO15, GND→GND, V+→12V, OUT–→Pompka#1(–), 12V→Pompka#1(+)
☐ MOSFET #2 (GPIO2):  SIG→GPIO2,  GND→GND, V+→12V, OUT–→Pompka#2(–), 12V→Pompka#2(+)
☐ MOSFET #3 (GPIO4):  SIG→GPIO4,  GND→GND, V+→12V, OUT–→Pompka#3(–), 12V→Pompka#3(+)
☐ MOSFET #4 (GPIO16): SIG→GPIO16, GND→GND, V+→12V, OUT–→Pompka#4(–), 12V→Pompka#4(+)
☐ MOSFET #5 (GPIO25): SIG→GPIO25, GND→GND, V+→12V, OUT–→Stirrer(–),  12V→Stirrer(+)
☐ Common Ground: GND ESP32 ↔ GND/COM każdego MOSFET (KRYTYCZNE!)
☐ Pompki zasilane z 12V (nie 5V!)
☐ Brak zwarcia 12V ↔ GND (multimetr, tryb ciągłości: cisza = OK)
```

---

## ETAP 6 – MIESZADEŁKO MAGNETYCZNE (DIY)

### Co to jest i po co?

Podczas titracji KH próbka musi być mieszana, żeby kwas HCl równomiernie reagował z wodą. Mieszadełko magnetyczne kręci małym prętem PTFE w komorze pomiarowej za pomocą obracających się magnesów napędzanych silnikiem.

### Budowa wirnika magnetycznego

**Potrzebujesz:**
- Silnik gearmotor 12V, 60 RPM
- 2 magnesy neodymowe N52 10×5mm
- Klej epoksydowy 2-składnikowy

**Kroki:**

1. Wymieszaj klej epoksydowy (proporcje wg instrukcji, zwykle 1:1)
2. Nanieś klej na oś silnika lub na specjalny krążek
3. Przyklej dwa magnesy **biegunami naprzeciwlegle** (N i S po przeciwnych stronach osi) – to tworzy pole magnetyczne które „ciągnie" stir bar
4. Odczekaj czas utwardzania kleju wg instrukcji (zazwyczaj 24h dla pełnej wytrzymałości, 30 min wstępne)
5. Sprawdź czy magnesy trzymają się pewnie – stir bar będzie się kręcił z dużą prędkością

```
Widok z góry silnika z magnesami:

        [S]
         │
    ─────●───── oś silnika
         │
        [N]

Magnesy po przeciwnych stronach = pole magnetyczne
które obraca stir bar PTFE w komorze
```

### Pozycjonowanie silnika pod komorą

Silnik musi być umieszczony **pod komorą pomiarową**, możliwie blisko dna:

```
  ┌──────────────────┐
  │  KOMORA 100ml    │  ← próbka wody + HCl
  │   ~~~~stirbar~~~ │  ← stir bar PTFE (kręci się)
  └────────┬─────────┘
           │ ~5–10mm
    ┌──────┴──────┐
    │   SILNIK    │  ← magnesy na osi kręcą się
    │   12V 60rpm │
    └─────────────┘
```

> Odległość silnik–dno komory: im mniejsza tym lepiej, max ~15mm. Przy większej odległości pole magnetyczne jest za słabe.

### Checklist Etap 6

```
☐ Magnesy przyklejone do osi silnika epoksydem
☐ Klej w pełni utwardzony (min. 2h, najlepiej 24h)
☐ Magnesy trzymają się pewnie (brak luzu)
☐ Silnik pod komorą, odległość < 15mm od dna
☐ Stir bar PTFE umieszczony w komorze testowej
☐ Stir bar reaguje na ruch silnika (sprawdź ręcznie kręcąc osią)
```

---

## ETAP 7 – KOMORA POMIAROWA I POJEMNIKI

### Rozmieszczenie komponentów hydraulicznych

```
                    ┌─────────────────────────────┐
                    │   AKWARIUM (źródło próbki)  │
                    └──────────────┬──────────────┘
                                   │
                              Pompka #1 (próbka)
                                   │
          ┌────────────────────────▼────────────────────────┐
          │              KOMORA POMIAROWA (~100ml)           │
          │  ┌─────────────────────────────────────────┐    │
          │  │  pH sonda (BNC) ← cały czas tu          │    │
          │  │  Stir bar PTFE                          │    │
          │  │  EC sonda                               │    │
          │  │  DS18B20 #3 (temp komory)               │    │
          │  └─────────────────────────────────────────┘    │
          └──┬─────────────────────────────────────────┬────┘
             │                                         │
        Pompka #3                                 Pompka #4
        (odpad)                                   (woda RO)
             │                                         │
    ┌────────▼────────┐                    ┌───────────▼──────────┐
    │  KANISTER 5L    │                    │   KANISTER 5L        │
    │  ODPAD          │                    │   WODA RO            │
    └─────────────────┘                    └──────────────────────┘
                                                       ↑
                                               (tu też trafia sonda
                                                pH między pomiarami)

          Pompka #2 (HCl) → wąż → komora pomiarowa
          ↑
    Butelka HCl 0.1M (~250ml – mała, bezpieczna pojemność)
```

### Wskazówki praktyczne

**Komora pomiarowa (pojemnik ~100ml):**
- Wybierz przezroczysty – możesz wizualnie kontrolować proces
- Szklanka 100ml z Ikea lub mała szklana zlewka
- Musi mieć otwory/przejścia na węże 4 pomp i kable sond
- Otwory uszczelnij silikonem neutralnym (nie octowym – niszczy metale)

**Węże pompek:**
- Każda pompka perystaltyczna ma własny wąż wewnętrzny (nie wymień!)
- Na wejściu/wyjściu: wąż silikonowy 3–4mm wewnętrzny
- Pompka #2 (HCl): użyj węża z tworzywa odpornego na kwas (PE lub PTFE)
- Oznacz węże kolorami lub etykietami – **nie wolno** pomylić wężyka HCl z innymi

**Etykietowanie – obowiązkowe:**
```
Pompka #1: PRÓBKA  (niebieski)
Pompka #2: HCl ☠️  (czerwony)
Pompka #3: ODPAD   (żółty)
Pompka #4: RO/pH   (biały)
```

### Checklist Etap 7

```
☐ Komora pomiarowa: przezroczysta, ~100ml, stabilna podstawa
☐ Węże pompek podłączone i oznaczone kolorami/etykietami
☐ Pompka #2 (HCl): wąż odporny na kwas (PE lub PTFE)
☐ Silnik mieszadełka pod komorą (odległość < 15mm)
☐ pH sonda zanurzona w komorze (lub w wodzie RO w kanisterze)
☐ EC sonda w komorze
☐ DS18B20 #3 w komorze
☐ Kanister odpadu: min. 3L pojemności
☐ Kanister RO: min. 3L pojemności (woda RO/DI)
```

---

## ETAP 8 – FIRMWARE (ESPHome)

### Konfiguracja YAML – Sentinel Chem/Monitor

```yaml
esphome:
  name: sentinel-chem
  friendly_name: "Sentinel Chem/Monitor"

esp32:
  board: esp32dev
  framework:
    type: arduino

logger:
  level: INFO

api:
  encryption:
    key: !secret api_encryption_key

ota:
  password: !secret ota_password

# Łączy się z siecią SentinelHub (AP Sentinel Hub)
wifi:
  ssid: "SentinelHub"
  password: "reef1234"

mqtt:
  broker: 10.42.0.1      # Adres IP Sentinel Hub (AP)
  port: 1883
  topic_prefix: reef/chem
  discovery: true

# ──────────────────────────────────────────
# SENTINEL MONITOR – czujniki pasywne
# ──────────────────────────────────────────

# DS18B20 × 3 (temperatura)
dallas:
  - pin: GPIO33
    update_interval: 15min
    id: dallas_bus

sensor:
  # Temperatury (adresy 1-Wire wykryje automatycznie po pierwszym boocie)
  - platform: dallas
    index: 0
    name: "Temp Akwarium"
    id: temp_aquarium
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    filters:
      - filter_out: nan
      - sliding_window_moving_average:
          window_size: 3
          send_every: 1

  - platform: dallas
    index: 1
    name: "Temp Sump"
    id: temp_sump
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    filters:
      - filter_out: nan

  - platform: dallas
    index: 2
    name: "Temp Komora"
    id: temp_chamber
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    filters:
      - filter_out: nan

  # EC / Zasolenie (GPIO32)
  - platform: adc
    pin: GPIO32
    name: "EC Raw"
    id: ec_raw
    attenuation: 11db
    update_interval: 15min
    filters:
      - multiply: 3.3        # Przeliczenie na napięcie
      - sliding_window_moving_average:
          window_size: 5
          send_every: 5

  # pH (GPIO34)
  - platform: adc
    pin: GPIO34
    name: "pH Raw Voltage"
    id: ph_raw
    attenuation: 11db
    update_interval: 15min
    filters:
      - multiply: 3.3
      - sliding_window_moving_average:
          window_size: 5
          send_every: 5

# ──────────────────────────────────────────
# SENTINEL CHEM – sterowanie pompkami
# ──────────────────────────────────────────

# Pompki (MOSFET low-side)
switch:
  - platform: gpio
    pin: GPIO15
    id: pump_sample
    name: "Pompka #1 Próbka"
    restore_mode: ALWAYS_OFF

  - platform: gpio
    pin: GPIO2
    id: pump_hcl
    name: "Pompka #2 HCl"
    restore_mode: ALWAYS_OFF

  - platform: gpio
    pin: GPIO4
    id: pump_waste
    name: "Pompka #3 Odpad"
    restore_mode: ALWAYS_OFF

  - platform: gpio
    pin: GPIO16
    id: pump_ro
    name: "Pompka #4 RO"
    restore_mode: ALWAYS_OFF

# Mieszadełko (PWM)
output:
  - platform: ledc
    pin: GPIO25
    id: stirrer_pwm
    frequency: 1000Hz

fan:
  - platform: speed
    output: stirrer_pwm
    name: "Mieszadełko"
    id: stirrer

# ──────────────────────────────────────────
# STATUS
# ──────────────────────────────────────────
binary_sensor:
  - platform: status
    name: "Sentinel Chem Status"

button:
  - platform: restart
    name: "Sentinel Chem Restart"
```

### Krok 8.1 – Wgraj firmware

1. Podłącz ESP32 do komputera USB (**zasilacz 12V wyłączony!**)
2. ESPHome → Install → Plug into this computer
3. Wybierz port COM/ttyUSB
4. Poczekaj na kompilację i wgranie
5. W logach po boocie:
```
[I][wifi:290]: Connected to SentinelHub
[I][mqtt:287]: MQTT connected to 10.42.0.1
[I][dallas:084]: Found Dallas sensors: 3
```

### Krok 8.2 – Identyfikacja adresów DS18B20

Po pierwszym boocie w logach ESPHome zobaczysz coś takiego:
```
[W][dallas:082]: Dallas sensor 0x28FF123456789012 has no index configured
[W][dallas:082]: Dallas sensor 0x28FF987654321098 has no index configured
[W][dallas:082]: Dallas sensor 0x28FFABCDEF012345 has no index configured
```

Każdy sensor ma unikalny adres. Żeby wiedzieć który to który:
1. Zanurz tylko **jedną sondę** w ciepłej wodzie, pozostałe w zimnej
2. Sprawdź w HA który sensor pokazuje wyższy odczyt → to ta sonda
3. Oznacz ją (taśmą, markerem) i przypisz do właściwego `index`

---

## ETAP 9 – KALIBRACJA

### 9.1 Kalibracja pH (DFRobot Gravity V2)

pH sonda wymaga kalibracji dwupunktowej. DFRobot V2 ma wbudowany układ kalibracji.

**Potrzebujesz:**
- Bufor pH 4.0 (żółty, zazwyczaj w zestawie)
- Bufor pH 7.0 (zielony, zazwyczaj w zestawie)
- Woda RO do płukania między buforami

**Procedura:**

```
Krok 1: Zanurz sondę w buforze pH 7.0
         Poczekaj 30s na stabilizację
         Zanotuj surowe napięcie z GPIO34 (widoczne w HA jako "pH Raw Voltage")
         → np. 1.738V przy pH 7.0

Krok 2: Opłucz sondę wodą RO (delikatnie, bez tarcia)
         Zanurz w buforze pH 4.0
         Poczekaj 30s
         Zanotuj napięcie
         → np. 2.031V przy pH 4.0

Krok 3: Oblicz współczynniki kalibracji:
         slope    = (pH4.0 – pH7.0) / (V_pH4 – V_pH7)
                  = (4.0 – 7.0) / (2.031 – 1.738)
                  = -3.0 / 0.293
                  = –10.24 pH/V

         intercept = pH7.0 – slope × V_pH7
                   = 7.0 – (–10.24 × 1.738)
                   = 7.0 + 17.80
                   = 24.80

Krok 4: Dodaj przelicznik do YAML ESPHome:
         - platform: adc
           pin: GPIO34
           name: "pH"
           filters:
             - multiply: 3.3
             - lambda: return -10.24 * x + 24.80;
```

> Kalibrację powtarzaj co 4–6 tygodni lub gdy odczyty wydają się błędne.
> Nowe bufory kupuj tylko od sprawdzonych dostawców – stare lub zanieczyszczone bufory dają złą kalibrację.

### 9.2 Kalibracja EC (zasolenie)

EC sensor daje surowe napięcie, które przeliczasz na przewodność i zasolenie.

**Metoda kalibracji jednorazowej:**

```
Krok 1: Przygotuj wodę z akwarium o ZNANYCH parametrach
         (zmierz refraktometrem – np. 35 ppt / 1.026 sg)

Krok 2: Zanurz EC sensor, odczytaj surowe napięcie z GPIO32

Krok 3: Oblicz współczynnik:
         factor = 35 ppt / V_zmierzone

Krok 4: Dodaj przelicznik w YAML:
         filters:
           - multiply: 3.3          # napięcie
           - multiply: factor       # ppt
```

> EC nie wymaga częstej rekalibracji – raz po montażu wystarczy.
> Sprawdzaj raz na miesiąc refraktometrem czy wartości się zgadzają.

### 9.3 Kalibracja pompek (objętościowa)

Przed titrację KH musisz wiedzieć ile ml pompka pobiera na sekundę.

**Metoda:**
```
Krok 1: Podłącz wężyk pompki do pojemnika z wodą RO
Krok 2: W HA włącz pompkę na DOKŁADNIE 10 sekund
         (użyj automation lub przycisku z timerem)
Krok 3: Zmierz zmierzoną objętość (strzykawka lub menzurka)
         → np. 8.5ml w 10s = 0.85 ml/s
Krok 4: Zanotuj dla każdej pompki osobno (różnią się!)
Krok 5: Użyj tej wartości w algorytmie titracji
```

**Tabela kalibracji pompek:**

| Pompka | Rola | ml/10s | ml/s |
|--------|------|--------|------|
| #1 | Próbka | ___ | ___ |
| #2 | HCl | ___ | ___ |
| #3 | Odpad | ___ | ___ |
| #4 | RO | ___ | ___ |

> Wypełnij tabelę po zmierzeniu. Pompki perystaltyczne mogą różnić się o 10–20% między sobą.

---

## ETAP 10 – PIERWSZY TEST SEKWENCJI (NA SUCHO)

Zanim użyjesz HCl, przetestuj całą mechanikę z **wodą RO**.

### Test 10.1 – Test pojedynczej pompki

W HA → Developer Tools → Services:
```yaml
service: switch.turn_on
target:
  entity_id: switch.pompka_1_probka
```
Obserwuj: czy pompka się kręci, czy wąż transportuje wodę, czy nie ma wycieków.
Następnie wyłącz i powtórz dla każdej pompki.

### Test 10.2 – Test mieszadełka

W HA → encja fan.mieszadelko → ustaw prędkość 50%.
Obserwuj: czy stir bar się kręci w komorze, czy silnik nie grzeje się za mocno.

### Test 10.3 – Sekwencja testowa (bez chemii)

Ręcznie wykonaj sekwencję titracji KH ale z wodą RO zamiast HCl:

```
1. Pompka #4 ON na 35s  → woda RO do komory (symulacja płukania)
2. Pompka #4 OFF
3. Pompka #1 ON na 20s  → próbka z akwarium do komory
4. Pompka #1 OFF
5. Mieszadełko 60% na 10s
6. Odczyt pH i temp (sprawdź w HA)
7. Pompka #3 ON na 40s  → wypompowanie do odpadu
8. Pompka #3 OFF
9. Pompka #4 ON na 35s  → woda RO (płukanie komory)
10. Pompka #4 OFF
```

Sprawdź: brak wycieków, sondy mają kontakt z wodą, odczyty mają sens.

### Checklist Etap 10

```
☐ Pompka #1: pobiera próbkę z akwarium do komory (wizualnie OK)
☐ Pompka #2: (testuj z wodą RO!) – przepływ OK, brak wycieków
☐ Pompka #3: wypompowuje komorę do odpadu
☐ Pompka #4: pompuje wodę RO do komory / zbiornika pH
☐ Mieszadełko: stir bar kręci się w komorze
☐ pH sonda: odczyty stabilne w wodzie RO (ok. 6.5–7.5)
☐ Temperatura: wszystkie 3 sondy DS18B20 dają sensowne wartości
☐ EC: odczyty w wodzie RO bliskie 0
☐ Brak wycieków z żadnego węża przez minimum 5 minut
```

---

## TROUBLESHOOTING

### Problem: DS18B20 – brak odczytów lub wartość –127°C

```
Przyczyna 1: Brak rezystora pull-up 4.7kΩ
  → Sprawdź czy rezystor jest między 3V3 a DATA (GPIO33)

Przyczyna 2: Zły rezystor – 10kΩ zamiast 4.7kΩ
  → Sprawdź paski kolorów (żółty-fioletowy-czerwony = 4.7kΩ)
  → 10kΩ: brązowy-czarny-pomarańczowy

Przyczyna 3: Przewód DATA nie podłączony do GPIO33
  → Sprawdź ciągłość multimetrem (DATA sonda ↔ GPIO33)

Przyczyna 4: Sonda uszkodzona
  → Przetestuj każdą sondę osobno
```

### Problem: pH – skaczące odczyty, brak stabilizacji

```
Przyczyna 1: Brak kondensatora filtrującego
  → Dodaj 100µF/25V na linii VCC modułu pH

Przyczyna 2: Kabel BNC blisko kabli zasilania
  → Odsuń BNC od linii 12V, MOSFET i GND zasilacza

Przyczyna 3: Sonda nie skondycjonowana
  → Nowa sonda wymaga 24h zanurzenia w buforze pH 7.0 przed pierwszym użyciem

Przyczyna 4: Sonda wysychała (brak czapki ochronnej lub RO)
  → Zanurz w buforze pH 7.0 na 24h (regeneracja)

Przyczyna 5: Stara kalibracja nieaktualna
  → Przeprowadź kalibrację 2-punktową od nowa
```

### Problem: EC – nieprawidłowe odczyty

```
Przyczyna 1: Brak kompensacji temperatury
  → Odczyty EC zmieniają się z temperaturą – upewnij się że firmware
    uwzględnia temp z DS18B20 #3 (komora)

Przyczyna 2: Elektroda EC w powietrzu
  → Elektroda musi być całkowicie zanurzona

Przyczyna 3: Błędny współczynnik kalibracji
  → Przerakalibruj z refraktometrem
```

### Problem: Pompka nie działa (brak ruchu)

```
Przyczyna 1: Brak common ground
  → GND ESP32 NIE jest połączony z COM MOSFET
  → To najczęstszy błąd! Sprawdź każdy MOSFET osobno

Przyczyna 2: Zły GPIO w YAML
  → Sprawdź czy switch.pompka_X odpowiada właściwemu GPIO

Przyczyna 3: MOSFET uszkodzony
  → Zmierz napięcie na OUT– gdy SIG = HIGH (powinno być ~0V gdy aktywny)
  → Jeśli ~12V przy aktywnym SIG → MOSFET nie przełącza = uszkodzony

Przyczyna 4: Pompka podłączona do 5V zamiast 12V
  → Pompki 12V na 5V kręcą się słabo lub wcale
  → Sprawdź że V+ MOSFET idzie do 12V zasilacza, nie do LM2596

Przyczyna 5: Odwrócona polaryzacja pompki
  → Zamień (+) i (–) przy pompce (kręci się w przeciwną stronę)
```

### Problem: Mieszadełko nie kręci stir bara

```
Przyczyna 1: Za duża odległość silnik–komora
  → Zmniejsz do < 15mm

Przyczyna 2: Za mała prędkość PWM
  → Zwiększ do 70–80% (niektóre stir bary wymagają wyższej prędkości startowej)

Przyczyna 3: Magnesy źle przyklejone (N-N lub S-S naprzeciwko)
  → Sprawdź orientację biegunów (powinny być N-S naprzeciwko)

Przyczyna 4: Stir bar zbyt ciężki lub niemagnetyczny
  → Upewnij się że to PTFE stir bar z metalowym rdzeniem (ferrytowy/neodym)
```

### Problem: Sentinel Chem nie łączy się z Sentinel Hub

```
Przyczyna 1: Sentinel Hub nie nadaje sieci SentinelHub
  → Sprawdź czy Sentinel Hub działa (LED, OLED)
  → Sprawdź w telefonie czy sieć SentinelHub jest widoczna

Przyczyna 2: Złe hasło w YAML
  → Hasło AP Sentinel Hub: reef1234

Przyczyna 3: Zły adres brokera MQTT
  → Broker jest na 10.42.0.1 (adres AP Sentinel Hub)
```

---

## FINALNA CHECKLIST – SENTINEL CHEM/MONITOR GOTOWY

```
FAZA A – SENSORY:
☐ LM2596: 5.0V ±0.1V zmierzone multimetrem
☐ ESP32: boot OK
☐ DS18B20 × 3: wszystkie widoczne w HA z sensownymi wartościami
☐ EC sensor: widoczny w HA, reaguje na zmianę zasolenia
☐ pH sensor: skalibrowany (2-punktowo pH 4.0 + pH 7.0), stabilny odczyt
☐ Sentinel Chem/Monitor widoczny w Sentinel Hub (MQTT)

FAZA B – MECHANIKA:
☐ MOSFET × 5: common ground podłączony
☐ Pompki × 4: każda przetestowana osobno z wodą RO
☐ Mieszadełko: stir bar kręci się w komorze
☐ Komora pomiarowa: bez wycieków przez 5 min testu
☐ Węże oznaczone kolorami/etykietami
☐ Kalibracja pompek: ml/s zanotowane dla każdej

INTEGRACJA:
☐ Wszystkie encje aktywne w HA
☐ Dane spływają do reef-sentinel.com (przez Sentinel Hub)
☐ Sekwencja testowa (z wodą RO) zakończona sukcesem

STATUS: ✅ Sentinel Chem/Monitor gotowy!
        Następny krok: Pierwsza titracja KH z HCl 0.1M
        (pamiętaj: rękawice + okulary)
```

---

## NASTĘPNE KROKI

Po zakończeniu Modułu 2:

1. **Pierwsza titracja KH** – z HCl 0.1M, małe ilości (50ml próbki)
2. **Porównanie z testem manualnym** (Salifert/Red Sea) – weryfikacja dokładności
3. **Konfiguracja harmonogramu** – jak często testować KH (preset w HA)
4. **Sentinel View** – wyświetlacz (Q2 2026)

---

## POWIĄZANE DOKUMENTY

- [`sentinel_hub_chem_monitor_BOM.md`](./sentinel_hub_chem_monitor_BOM.md) – Lista komponentów
- [`BUILD_GUIDE_sentinel_hub.md`](./BUILD_GUIDE_sentinel_hub.md) – Instrukcja Modułu 1 (wymaganie wstępne)
- [`sentinel_view_BOM.md`](./sentinel_view_BOM.md) – BOM dla Sentinel View (Faza 2)

---

*Reef Sentinel Lab – Open-source aquarium controller*  
*reef-sentinel.com | github.com/reef-sentinel*  
*Ostatnia aktualizacja: 2026-03-06*
