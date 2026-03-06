# 🐠 Reef Sentinel Lab

**Open-source reef aquarium controller. Mierzy KH, Ca, Mg automatycznie. Nie stanie się cegłą.**

[![License: MIT](https://img.shields.io/badge/License--MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![ESPHome](https://img.shields.io/badge/Firmware-ESPHome-blue)](https://esphome.io)
[![Home Assistant](https://img.shields.io/badge/Integration-Home%20Assistant-41BDF5)](https://www.home-assistant.io)
[![Status](https://img.shields.io/badge/Status-Active%20Development-brightgreen)]()

---

## Dlaczego Reef Sentinel?

Pamiętasz **Reef Factory**? Urządzenia za 2500–6000 zł. Upadłość w 2024. Tysiące akwarystów z cegłami zamiast kontrolerów.

**Z Reef Sentinel to się nie może zdarzyć.**

Cały kod, schematy i dokumentacja są publiczne. Nawet jeśli projekt przestanie być rozwijany – Ty masz wszystko. Działa lokalnie, bez chmury, bez subskrypcji.

| | Reef Sentinel Lab | Neptune Apex + Trident | GHL ProfiLux |
|--|-------------------|----------------------|--------------|
| KH / Ca / Mg auto | ✅ | ✅ | Częściowo |
| Open-source | ✅ | ❌ | ❌ |
| Lokalny (bez chmury) | ✅ | ❌ | ❌ |
| Vendor lock-in | ❌ | ✅ | ✅ |
| Home Assistant native | ✅ | ❌ | ❌ |

---

## Co robi Reef Sentinel Lab?

### Automatyczne testy chemii (BIG THREE)

| Parametr | Moduł | Metoda | Częstotliwość | Dokładność |
|----------|-------|--------|---------------|-----------|
| **KH** (zasadowość) | Sentinel Chem | Titracja HCl 0.1M | 1–7× dziennie | ±0.2 dKH |
| **pH** | Sentinel Chem | Elektroda potencjometryczna | co 15 min | ±0.05 |
| **Ca** (wapń) | Sentinel Photometer | Fotometria Arsenazo III | 1× dziennie | ±10 ppm |
| **Mg** (magnez) | Sentinel Photometer | Fotometria Calmagite | 1× dziennie | ±30 ppm |

### Monitoring ciągły 24/7

| Parametr | Moduł | Czujnik |
|----------|-------|---------|
| Temperatura × 3 | Sentinel Monitor | DS18B20 (akwarium, sump, komora) |
| Zasolenie/EC | Sentinel Monitor | DFRobot Gravity TDS |

### Triki które wyróżniają ten projekt

**Sonda pH w wodzie RO** – między pomiarami sonda przechowywana jest w wodzie RO zamiast w wodzie morskiej. Żywotność elektrody: **18–36 miesięcy** zamiast 4–6 miesięcy. Koszt eksploatacji: ~20 zł/rok zamiast ~120 zł/rok.

**Modularność** – każdy moduł to niezależny ESP32 z własnym firmware. Awaria Sentinel Chem/Monitor nie wpływa na Sentinel View i odwrotnie. Sentinel Hub pełni rolę koordynatora – jego awaria wstrzymuje agregację danych, ale moduły pomiarowe kontynuują pracę lokalnie.

**Integracja HA** – przez ESPHome API. Po dodaniu urządzenia w Home Assistant encje sensorów pojawiają się automatycznie – nie trzeba ręcznie definiować każdego czujnika w `configuration.yaml`.

---

## Architektura systemu

```
                    Internet
                       │
              reef-sentinel.com  ←── AI Insights (GPT-4)
                       │              Dashboard, historia
                  co 15 min
                       │
              ┌────────┴────────┐
              │  Sieć domowa    │
              │  (WiFi router)  │
              └────────┬────────┘
                       │
           ┌───────────┴──────────────┐
           │     SENTINEL HUB         │
           │     (Module 1)           │
           │  IP: 10.42.0.1           │
           │  AP: SentinelHub         │
           │  MQTT Broker: :1883      │
           │  OLED status display     │
           └────┬──────┬──────┬───────┘
         WiFi  │      │      │  WiFi
               │      │      │
    ┌──────────┘   ┌───┘   └──────────────┐
    │              │                       │
┌───┴──────────┐ ┌─┴─────────────┐  ┌────┴──────────┐
│ SENTINEL     │ │ SENTINEL      │  │ SENTINEL      │
│ CHEM/MONITOR │ │ PHOTOMETER    │  │ VIEW          │
│ (Module 2)   │ │ (Module 3)    │  │ (Module 4)    │
│              │ │               │  │               │
│ pH + KH auto │ │ Ca + Mg foto  │  │ Nextion 5.0"  │
│ Temp × 3     │ │ LED 650/520nm │  │ Touch display │
│ EC/zasolenie │ │ Fotodioda     │  │ Real-time UI  │
│ Pompki × 4   │ │ ADS1115       │  │               │
│ Mieszadełko  │ │               │  │               │
┌──────────────┘ └───────────────┘  └───────────────┘
   Faza 1 ✅        Faza 2 (Q3 2026)   Faza 2 (Q2 2026)

           ┌──────────────────────────────────┐
           │  SENTINEL CONNECTOR  (Module 5)  │
           │  Standalone – nie wymaga Hub     │
           │  Integracja: Apex / GHL / Hydros │
           │  → reef-sentinel.com + HA        │
           └──────────────────────────────────┘
                      TBD
```

---

## Moduły

### Module 1 – Sentinel Hub ✅ Gotowy do budowy
WiFi koordynator całego systemu. Tworzy sieć `SentinelHub` (10.42.0.1), zbiera dane ze wszystkich modułów, synchronizuje z reef-sentinel.com co 15 minut.

**Komponenty:** ESP32 + LM2596 + OLED 0.96" + SD card reader  
**Koszt:** ~102–117 zł

### Module 2 – Sentinel Chem/Monitor ⚠️ 60% gotowy
Automatyczna titracja KH, monitoring pH 24/7, temperatura × 3, zasolenie. Fizycznie jeden ESP32, marketingowo dwa produkty.

**Komponenty:** ESP32 + LM2596 + DFRobot pH V2 + EC sensor + DS18B20 × 3 + MOSFET × 5 + pompki × 4 + mieszadełko magnetyczne  
**Koszt:** ~450 zł

### Module 3 – Sentinel Photometer ⏸️ Q3 2026
Fotometryczny pomiar Ca i Mg. 2-kanałowy: LED 650nm (Ca, arsenazo III) + LED 520nm (Mg, calmagite). ADS1115 16-bit ADC dla precyzji.

**Planowany koszt:** ~400 zł (+60 zł/50 testów reagenty DIY)

### Module 4 – Sentinel View ⏸️ Q2 2026
Dotykowy panel wyświetlający dane w czasie rzeczywistym. Nextion NX8048P050-011C (5.0", 800×480, własny MCU 200MHz, 120MB flash). Połączenie przez UART – tylko 4 przewody.

### Module 5 – Sentinel Connector ⏸️ TBD
Moduł dla właścicieli istniejącego sprzętu (Neptune Apex, GHL ProfiLux, Hydros i in.). Działa niezależnie od pozostałych modułów Reef Sentinel Lab – nie wymaga Sentinel Hub. Łączy dane z zewnętrznego kontrolera z reef-sentinel.com i Home Assistant, dodając automatyczną chemię (KH, Ca, Mg) bez wymiany całego systemu.

**Przypadek użycia:** masz Apex za 3000 zł i nie chcesz go wyrzucać – Sentinel Connector dodaje automatyczny pomiar KH/Ca/Mg jako uzupełnienie.

---

## Specyfikacja techniczna

| | Wartość |
|--|---------|
| Mikrokontroler | ESP32-WROOM-32 (każdy moduł) |
| Firmware | ESPHome |
| Komunikacja wewnętrzna | WiFi 2.4GHz + MQTT |
| Protokół danych | JSON over MQTT |
| Integracja HA | Natywna (ESPHome API) |
| Cloud | reef-sentinel.com (REST API) |
| Zasilanie | 12V DC / 5A |
| Konwersja napięcia | LM2596 (12V→5V, per moduł) |
| Obudowa docelowa | Takachi SWN11-3-14G (IP67) |
| PCB | Planowane Q2 2026 (JLCPCB) |

---

## Szybki start

### Wymagania wstępne

- Home Assistant (opcjonalnie, ale zalecane)
- ESPHome (add-on do HA lub standalone)
- Konto na reef-sentinel.com (3 miesiące gratis, następnie plan płatny)
- Multimetr (do ustawienia LM2596 na 5.0V – obowiązkowe!)

### Kolejność budowy

```
Krok 1 ── Sentinel Hub (Module 1)
           └── Czas: 2–3h | Koszt: ~117 zł | Trudność: ⭐⭐☆☆☆

Krok 2 ── Sentinel Chem/Monitor (Module 2) – Faza A (sensory)
           └── Czas: 2–3h | Koszt: ~200 zł | Trudność: ⭐⭐⭐☆☆

Krok 3 ── Sentinel Chem/Monitor (Module 2) – Faza B (pompki + mechanika)
           └── Czas: 3–4h | Koszt: ~250 zł | Trudność: ⭐⭐⭐☆☆

Krok 4 ── Integracja i testy na akwarium
           └── Czas: 1–2h

Krok 5 ── Sentinel View (Module 4) – opcjonalny
           └── Czas: 2–3h + 1–2 dni UI | Koszt: ~365 zł | Trudność: ⭐⭐☆☆☆
```

### Dokumentacja

| Dokument | Opis |
|----------|------|
| [`docs/BOM_sentinel_hub_chem_monitor.md`](docs/BOM_sentinel_hub_chem_monitor.md) | Lista komponentów Module 1+2 |
| [`docs/BOM_sentinel_view.md`](docs/BOM_sentinel_view.md) | Lista komponentów Module 4 |
| [`docs/BUILD_GUIDE_sentinel_hub.md`](docs/BUILD_GUIDE_sentinel_hub.md) | Instrukcja montażu Module 1 |
| [`docs/BUILD_GUIDE_sentinel_chem_monitor.md`](docs/BUILD_GUIDE_sentinel_chem_monitor.md) | Instrukcja montażu Module 2 |
| [`docs/BUILD_GUIDE_sentinel_photometer.md`](docs/BUILD_GUIDE_sentinel_photometer.md) | Szkielet Module 3 (Q3 2026) |
| [`docs/BUILD_GUIDE_sentinel_view.md`](docs/BUILD_GUIDE_sentinel_view.md) | Instrukcja montażu Module 4 |
| [`docs/WIRING_DIAGRAM.md`](docs/WIRING_DIAGRAM.md) | Schematy połączeń całości |
| [`firmware/sentinel_hub.yaml`](firmware/sentinel_hub.yaml) | ESPHome – Module 1 |
| [`firmware/sentinel_chem.yaml`](firmware/sentinel_chem.yaml) | ESPHome – Module 2 |

---

## Roadmap

```
Q1 2026  ──  ✅ Projekt i dokumentacja (faza obecna)
             ✅ Sentinel Hub – gotowy do budowy
             ⚠️ Sentinel Chem/Monitor – 60% gotowy

Q2 2026  ──  Sentinel Chem/Monitor – 100% + deploy na akwarium
             Sentinel View – pierwsza wersja UI (Nextion)
             Beta testing (5–10 użytkowników)
             Strona www + GitHub public launch

Q3 2026  ──  Sentinel Photometer v0.1 (Ca + Mg)
             DIY Kit – pierwsze sprzedaże
             PCB design (KiCad + JLCPCB)

Q4 2026  ──  Sentinel Photometer v1.0 (stabilny)
             Gotowe urządzenia (plug-and-play)
             Ekspansja EU (DE, UK)

2027     ──  Dozownik 4-kanałowy (v3)
             ML predictions (cloud, reef-sentinel.com PRO)
             Społeczność 200+ użytkowników
```

---

## Model biznesowy i licencja

### Licencja

| Co | Licencja |
|----|---------|
| Hardware (schematy, PCB, BOM) | CERN-OHL-P v2 (Permissive) |
| Firmware (ESPHome YAML, kod) | MIT |
| Dokumentacja | CC BY 4.0 |
| Marka „Reef Sentinel" | ™ Trademark (chronione) |

**Wolno Ci:**  
✅ Zbudować własne urządzenie (DIY)  
✅ Modyfikować schematy i kod  
✅ Sprzedawać urządzenia oparte na tym projekcie  
✅ Tworzyć fork projektu  

**Nie wolno:**  
❌ Używać nazwy „Reef Sentinel" bez zgody  
❌ Używać logo projektu  

### Jak zarabiamy (jak Arduino, jak Prusa)

Kod i schematy są darmowe. Zarabiamy na:
- **Gotowych urządzeniach** – złożone, skalibrowane, z gwarancją
- **DIY Kitach** – wszystkie komponenty w jednej paczce
- **Wsparciu premium** – konfiguracja, instalacja on-site
- **Cloud PRO** – ML predictions na reef-sentinel.com

---

## Bezpieczeństwo

> ⚠️ **WAŻNE:** Reef Sentinel to urządzenie DIY. Używasz go na własną odpowiedzialność.

Kluczowe zasady bezpieczeństwa:
- LM2596 MUSI być ustawiony na **dokładnie 5.0V** multimetrem przed podłączeniem ESP32
- Nigdy nie podłączaj ESP32 do USB i LM2596 jednocześnie (dwa źródła zasilania = uszkodzenie)
- HCl (kwas solny) – praca wyłącznie w rękawicach i okularach ochronnych
- Przy pierwszym teście pompek używaj wody RO, nie HCl

---

## Status projektu

| Moduł | Status | Gotowość |
|-------|--------|---------|
| Sentinel Hub | 🟢 Gotowy do budowy | 90% |
| Sentinel Chem/Monitor | 🟡 W budowie | 60% |
| Sentinel Photometer | ⏸️ Q3 2026 | Szkielet |
| Sentinel View | ⏸️ Q2 2026 | Zaplanowany |
| Sentinel Connector | ⏸️ TBD | Koncepcja |
| PCB (wszystkie moduły) | ⏸️ Q2 2026 | Planowanie |
| reef-sentinel.com | 🟢 Live | Operacyjny |

---

## Społeczność i wsparcie

- 🌐 **Strona:** [reef-sentinel.com](https://reef-sentinel.com)
- 💬 **Forum:** forum.reef-sentinel.com *(wkrótce)*
- 📧 **Kontakt:** opensource@reefsentinel.pl
- 🐛 **Bugi:** GitHub Issues
- 💡 **Propozycje:** GitHub Discussions

---

## Podziękowania

Projekt czerpie inspirację z:
- [reef-pi](https://reef2reef.com/oss/reef-pi/) – pionier open-source reef controllera
- [ReefManager](https://reefmanager.eu) – DE community, świetna dokumentacja
- [ESPHome](https://esphome.io) – bez którego ten projekt byłby 10× trudniejszy
- Wszystkich akwarystów którzy testowali i zgłaszali bugi

---

*Reef Sentinel Lab – Open-source aquarium controller*  
*reef-sentinel.com | github.com/reef-sentinel*  
*Last updated: 2026-03-06*
