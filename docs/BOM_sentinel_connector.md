# Reef Sentinel – Bill of Materials
## Module 5: Sentinel Connector (Bridge dla zewnętrznych kontrolerów)

> **Version:** 0.1
> **Date:** 2026-03-07
> **Status projektu:** ⏸️ TBD – koncepcja (planowany start: Q4 2026)
> **Autor:** reef-sentinel.com | github.com/reef-sentinel

---

## 📦 STATUS KOMPONENTÓW – PODSUMOWANIE

| Kategoria | Co masz | Czego brakuje | Koszt zakupu |
|-----------|---------|---------------|--------------|
| Elektronika (ESP32, LM2596, zasilacz) | ❌ Brak – osobny moduł | ESP32, LM2596, zasilacz 12V/1A | ~48 zł |
| Obudowa i montaż | ❌ Brak | Mała obudowa ABS lub druk 3D | ~10 zł |
| **RAZEM** | | | **~58 zł** |

> To **najtańszy moduł** w całym systemie Reef Sentinel Lab.
> Jeśli masz już ESP32 i LM2596 z poprzedniego projektu – koszt wynosi ~15 zł (sam zasilacz).

---

## FUNKCJE MODUŁU

### Sentinel Connector – bridge dla zewnętrznych kontrolerów
- Odpytuje API istniejącego kontrolera (Apex, GHL, Hydros) co 60 sekund
- Normalizuje dane do ujednoliconego formatu Reef Sentinel JSON
- Przekazuje dane do reef-sentinel.com co 15 minut (HTTPS)
- Udostępnia dane przez ESPHome API → Home Assistant (opcjonalnie)
- Przekazuje dane przez MQTT (opcjonalnie, jeśli broker dostępny w sieci)

### Kluczowe cechy
- **Standalone** – jedyny moduł RS Lab który NIE wymaga Sentinel Hub
- Łączy się z **siecią domową** (nie z AP SentinelHub)
- Brak czujników, brak pompek – tylko WiFi i logika integracji
- Obsługuje stopniową migrację: działa obok istniejącego kontrolera

### Obsługiwane kontrolery (planowane)

| Kontroler | Protokół | Wersja wsparcia | Termin |
|-----------|----------|----------------|--------|
| Neptune Apex | REST API (LAN) | v0.5 | Q4 2026 |
| GHL ProfiLux | GHL HTTP API | v0.7 | Q1 2027 |
| Hydros (CoralVue) | REST API (cloud) | v0.9 | Q1 2027 |
| Generic MQTT | MQTT | v1.x | 2027+ |
| Generic HTTP | HTTP polling | v1.x | 2027+ |
| Raspberry Pi | MQTT / HTTP | v1.x | 2027+ |

---

## 🔌 ELEKTRONIKA

### Główny kontroler

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **ESP32 DevKit** | 30-pin ESP-WROOM-32 | Botland / Kamami | 1 | 25 zł | 25 zł | 🔴 KUP | WiFi-only – brak dodatkowych GPIO potrzebnych |

### Zasilanie

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **Zasilacz DC** | 12V/1A, gniazdo 5.5/2.1mm | Botland / Allegro | 1 | 15 zł | 15 zł | 🔴 KUP | Lekki zasilacz – brak pompek. ESP32 pobiera ~250 mA. |
| **LM2596 Buck Converter** | 12V→5V z wyświetlaczem | Botland / AliExpress | 1 | 8 zł | 8 zł | 🔴 KUP | ⚠️ Ustaw DOKŁADNIE 5.0V multimetrem! |

> **Bilans prądowy:**
> ESP32 aktywne WiFi: ~250 mA → zasilacz 12V/1A = margines 4× ✅
> Nie ma powodu używać zasilacza 2A ani 3A – Connector nie ma żadnych aktuatorów.

---

## 📌 GPIO Assignment – Sentinel Connector

| Funkcja | GPIO | Interfejs | Uwagi |
|---------|------|-----------|-------|
| WiFi | Wbudowane | – | Łączy z siecią DOMOWĄ (nie SentinelHub!) |
| Status LED | GPIO2 (built-in) | Digital | Wbudowana – miga przy połączeniu |
| (Opcja) Dioda statusu ext. | GPIO4 | Digital | Zewnętrzna dioda – online/offline |

**GPIO użytych: 1 (built-in LED) ✅ – minimalistyczny moduł**

> Connector nie steruje żadnymi czujnikami ani aktuatorami.
> Wszystkie dane pobiera przez WiFi z zewnętrznego kontrolera.

---

## 🔧 MECHANIKA I OBUDOWA

### Opcja A – Mała obudowa ABS (zalecana dla montażu przy kontrolerze)

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **Obudowa ABS** | Np. Hammond 1591XXSSBK lub podobna | TME / Botland | 1 | 10 zł | 10 zł | 🔴 KUP | Zmieści ESP32 + LM2596 |
| **Taśma dwustronna 3M VHB** | Piankowa | Sklep lokalny | 1 pasek | 2 zł | 2 zł | 🔴 KUP | Montaż ESP32 i LM2596 w obudowie |

### Opcja B – Montaż na szynie DIN (dla szaf sterowniczych)

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **Uchwyt DIN** | Do ESP32 DevKit, druk 3D lub gotowy | AliExpress / Thingiverse | 1 | 5 zł | 5 zł | 🟢 OPCJONALNY | Dla użytkowników Apex/GHL montujących w szafie |

### Opcja C – Druk 3D (najmniejsza obudowa)

| Komponent | Model/Spec | Dostawca | Ilość | Cena jedn. | Razem | Status | Uwagi |
|-----------|-----------|----------|-------|-----------|-------|--------|-------|
| **Obudowa drukowana 3D** | PLA/PETG, minimalna – tylko ESP32+LM2596 | Własna / Printuj3D.pl | 1 | 5 zł | 5 zł | 🟢 OPCJONALNY | Najprostsza opcja |

---

## 💻 KONFIGURACJA – WYMAGANIA

### Neptune Apex

| Wymaganie | Wartość |
|-----------|---------|
| Model | Apex Classic, Apex Jr., Apex EL, Apex (2016+) |
| Połączenie | LAN (Ethernet lub WiFi) |
| Endpoint | `http://{apex_ip}/cgi-bin/status.json` |
| Autoryzacja | Bearer token z panelu Apex |
| Klucz API | Apex panel → System → Network → API Key |
| Format odpowiedzi | JSON z tablicą `status[]` |

### GHL ProfiLux

| Wymaganie | Wartość |
|-----------|---------|
| Model | ProfiLux 4 lub nowszy |
| Moduł sieciowy | GHL IF-USB-Wifi lub LAN |
| Endpoint | `http://{ghl_ip}/api/v1/profilux/read` |
| Format odpowiedzi | JSON (dokumentacja GHL API) |
| Uwaga | API słabiej udokumentowane – może wymagać analizy pakietów |

### Hydros (CoralVue)

| Wymaganie | Wartość |
|-----------|---------|
| Model | Hydros Control (dowolna generacja) |
| Połączenie | Przez Internet (brak lokalnego API!) |
| Endpoint | `https://api.hydrosapp.com/v2/devices/{id}/readings` |
| Autoryzacja | Bearer token |
| Klucz API | hydrosapp.com → Settings → Developer API |
| Uwaga | ⚠️ Zależność od chmury – vendor lock-in |

### secrets.yaml – wymagane wpisy

```yaml
# Sieć domowa (nie SentinelHub!)
wifi_home_ssid: "NazwaTwojejSieci"
wifi_home_password: "HasloTwojejSieci"

# Reef Sentinel cloud
reef_sentinel_api_key: "rs_placeholder"
tank_id: "tank_001"

# Wybierz JEDEN z kontrolerów:

# Opcja A – Neptune Apex
controller_api_url: "http://192.168.1.100/cgi-bin/status.json"
controller_api_key: "Bearer TWOJ-KLUCZ-APEX"

# Opcja B – GHL ProfiLux
# controller_api_url: "http://192.168.1.101/api/v1/profilux/read"
# controller_api_key: ""  # GHL może nie wymagać klucza w LAN

# Opcja C – Hydros
# controller_api_url: "https://api.hydrosapp.com/v2/devices/DEVICE_ID/readings"
# controller_api_key: "Bearer TWOJ-KLUCZ-HYDROS"
```

---

## 💰 PODSUMOWANIE KOSZTÓW

### Wersja podstawowa (ESP32 + LM2596 + obudowa)

| Kategoria | Koszt |
|-----------|-------|
| ESP32 DevKit 30-pin | 25 zł |
| Zasilacz 12V/1A | 15 zł |
| LM2596 Buck Converter | 8 zł |
| Obudowa ABS (opcja A) | 10 zł |
| Taśma VHB | 2 zł |
| **RAZEM** | **~60 zł** |

> Dla porównania: Sentinel Hub (~117 zł), Sentinel Chem (~456 zł już wydanych + ~177 zł brakuje),
> Sentinel View (~375 zł), Sentinel Photometer (~323 zł).
> **Connector to 3× tańszy niż Hub i 10× tańszy niż Chem.**

### Szacowany koszt urządzenia gotowego (przy sprzedaży)

| Kategoria | Koszt |
|-----------|-------|
| Komponenty | ~60 zł |
| Firmware konfiguracja (czas) | ~15 zł |
| QA + pakowanie | ~15 zł |
| **KOSZT CAŁKOWITY** | **~90 zł** |

**Sugerowana cena sprzedaży:** 149 zł
**Marża:** ~59 zł (39.6%)

> Connector wyceniony nisko celowo – bariera wejścia dla użytkowników Apex/GHL
> którzy chcą wypróbować reef-sentinel.com bez wymiany całego systemu.

---

## 🛒 LISTA ZAKUPÓW – GDZIE KUPIĆ

### Botland (botland.com.pl)
- ESP32 DevKit 30-pin (ESP-WROOM-32)
- LM2596 Buck Converter z wyświetlaczem

### Allegro / Botland
- Zasilacz 12V/1A (gniazdo 5.5/2.1mm)

### TME (tme.eu) lub Botland
- Obudowa ABS (np. Hammond 1591, Maszczyk Z-BOX lub podobna)

### AliExpress (opcjonalnie)
- Uchwyt DIN do ESP32 DevKit (szukaj: "ESP32 DIN rail mount")
- Zapas LM2596 ×2 (warto mieć)

---

## ⚠️ KRYTYCZNE UWAGI

```
PRZED PIERWSZYM WŁĄCZENIEM – CHECKLIST:

☐ LM2596 ustawiony na DOKŁADNIE 5.0V (multimetrem!)
   → Standard dla wszystkich modułów RS Lab

☐ WiFi: sieć DOMOWA, NIE SentinelHub
   → Connector musi mieć dostęp do LAN z kontrolerem (Apex/GHL)
   → Lub do Internetu (Hydros cloud API)

☐ secrets.yaml: klucz API kontrolera i reef_sentinel_api_key uzupełnione
   → Bez tego moduł bootuje ale nic nie wysyła

☐ Test pollingu: sprawdź logi ESPHome – dane z kontrolera widoczne?
   → Błąd HTTP 401 → zły klucz API
   → Błąd HTTP 404 → zły URL lub IP kontrolera
   → Timeout → zły IP lub kontroler niedostępny w LAN

☐ secrets.yaml NIE commituj do publicznego repo
   → Klucze API to wrażliwe dane
```

---

## 📋 PORÓWNANIE MODUŁÓW – KOSZT CAŁEGO SYSTEMU

| Moduł | Status | Koszt komponentów |
|-------|--------|-------------------|
| Module 1 – Sentinel Hub | ✅ Gotowy | ~117 zł |
| Module 2 – Sentinel Chem/Monitor | 🟡 W budowie | ~633 zł (już wydane + do kupienia) |
| Module 3 – Sentinel Photometer | ⏸️ Q3 2026 | ~323 zł |
| Module 4 – Sentinel View | ⏸️ Q2 2026 | ~375 zł |
| Module 5 – Sentinel Connector | ⏸️ TBD | ~60 zł |
| **RAZEM (pełny system)** | | **~1508 zł** |

> Module 5 jest alternatywą, nie uzupełnieniem modułów 1–4.
> Użytkownicy z Apex/GHL zazwyczaj **nie** budują modułów 1–4 – kupują tylko Connector.

---

## 🔗 POWIĄZANE DOKUMENTY

- `BUILD_GUIDE_sentinel_connector.md` – instrukcja konfiguracji krok po kroku
- `reef_hub_chem_monitor_BOM.md` – BOM dla Modułów 1+2 (dla nowych użytkowników RS)
- `BOM_sentinel_view.md` – BOM dla Modułu 4
- `BOM_sentinel_photometer.md` – BOM dla Modułu 3
- `firmware/sentinel_connector.yaml` *(do stworzenia)*

---

*Reef Sentinel Lab – Open-source aquarium controller*
*reef-sentinel.com | github.com/reef-sentinel*
*Ostatnia aktualizacja: 2026-03-07*
