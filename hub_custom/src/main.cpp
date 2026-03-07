#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <time.h>
#include <deque>

namespace {
WebServer server(80);
Preferences prefs;
constexpr int OLED_WIDTH = 128;
constexpr int OLED_HEIGHT = 64;
constexpr uint8_t OLED_ADDR = 0x3C;
constexpr int OLED_SDA_PIN = 21;
constexpr int OLED_SCL_PIN = 22;
Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
bool oled_ready = false;
uint32_t last_oled_update_ms = 0;
uint32_t wifi_disconnected_since_ms = 0;
uint32_t last_cloud_sync_ms = 0;
uint32_t last_cloud_sync_try_ms = 0;
bool last_cloud_sync_ok = false;
String last_cloud_sync_message = "never";
uint32_t last_cloud_enqueue_ms = 0;

struct CloudQueueItem {
  String payload;
  uint8_t retry_step{0};  // 0->+1m, 1->+5m, 2->+15m
  uint32_t next_try_ms{0};
};
std::deque<CloudQueueItem> cloud_queue;
const size_t CLOUD_QUEUE_MAX = 500;

struct ChemState {
  float ph{-1.0f};
  float kh{-1.0f};
  float temp_aq{-20.0f};
  float temp_sump{-20.0f};
  float temp_room{-20.0f};
  float ec{-1.0f};
  bool kh_test_active{false};
  String last_kh_test{"brak danych"};
  uint32_t last_update_ms{0};
} chem;

struct HubConfig {
  String chem_host{"10.42.0.2"};
  uint16_t chem_port{80};
  int chem_publish_interval_s{30};
  int chem_kh_interval_h{1};
  float ph_cal_liquid1{7.00f};
  float ph_cal_liquid2{4.00f};
  String reef_webhook_url{"https://reef-sentinel.com/api/integrations/webhook"};
  String reef_api_key{""};
  String reef_tank_id{""};
  String reef_device_id{"hub_unknown"};
  int cloud_sync_interval_min{15};
} cfg;

void load_config() {
  prefs.begin("reefhub", true);
  cfg.chem_host = prefs.getString("chem_host", "10.42.0.2");
  cfg.chem_port = static_cast<uint16_t>(prefs.getUShort("chem_port", 80));
  cfg.chem_publish_interval_s = prefs.getInt("chem_pub_s", 30);
  cfg.chem_kh_interval_h = prefs.getInt("chem_kh_h", 1);
  cfg.ph_cal_liquid1 = prefs.getFloat("ph_l1", 7.00f);
  cfg.ph_cal_liquid2 = prefs.getFloat("ph_l2", 4.00f);
  cfg.reef_webhook_url = prefs.getString("reef_webhook", "https://reef-sentinel.com/api/integrations/webhook");
  cfg.reef_api_key = prefs.getString("reef_api_key", "");
  cfg.reef_tank_id = prefs.getString("reef_tank_id", "");
  cfg.reef_device_id = prefs.getString("reef_dev_id", "hub_unknown");
  cfg.cloud_sync_interval_min = prefs.getInt("cloud_int_m", 15);
  prefs.end();
}

void save_config() {
  prefs.begin("reefhub", false);
  prefs.putString("chem_host", cfg.chem_host);
  prefs.putUShort("chem_port", cfg.chem_port);
  prefs.putInt("chem_pub_s", cfg.chem_publish_interval_s);
  prefs.putInt("chem_kh_h", cfg.chem_kh_interval_h);
  prefs.putFloat("ph_l1", cfg.ph_cal_liquid1);
  prefs.putFloat("ph_l2", cfg.ph_cal_liquid2);
  prefs.putString("reef_webhook", cfg.reef_webhook_url);
  prefs.putString("reef_api_key", cfg.reef_api_key);
  prefs.putString("reef_tank_id", cfg.reef_tank_id);
  prefs.putString("reef_dev_id", cfg.reef_device_id);
  prefs.putInt("cloud_int_m", cfg.cloud_sync_interval_min);
  prefs.end();
}

String chem_url(const String &path_and_query) {
  return "http://" + cfg.chem_host + ":" + String(cfg.chem_port) + path_and_query;
}

bool http_post_plain(const String &url, const String &body = "") {
  HTTPClient http;
  if (!http.begin(url)) {
    return false;
  }
  int code = http.POST(body);
  http.end();
  return code >= 200 && code < 300;
}

bool forward_chem_command(const String &path) { return http_post_plain(chem_url(path)); }

bool forward_chem_number(const String &id, const String &value) {
  return http_post_plain(chem_url("/number/" + id + "/set?value=" + value));
}

String mask_api_key(const String &key) {
  if (key.length() <= 8) return "****";
  return key.substring(0, 4) + "..." + key.substring(key.length() - 4);
}

String now_iso_utc() {
  time_t now = time(nullptr);
  if (now < 1700000000) {
    return "";
  }
  struct tm tmbuf {};
  gmtime_r(&now, &tmbuf);
  char out[25];
  strftime(out, sizeof(out), "%Y-%m-%dT%H:%M:%SZ", &tmbuf);
  return String(out);
}

String default_device_id() {
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  mac.toLowerCase();
  return "hub_" + mac;
}

String short_val(float v, uint8_t digits) {
  if (isnan(v)) return "--";
  return String(static_cast<double>(v), static_cast<unsigned int>(digits));
}

void oled_init() {
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN, 400000U);
  oled_ready = oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  if (!oled_ready) {
    Serial.println("[reef-hub] OLED init failed (SSD1306 0x3C)");
    return;
  }
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  oled.println("Reef Sentinel Hub");
  oled.println("OLED ready");
  oled.display();
}

void oled_render() {
  if (!oled_ready) return;
  const uint32_t now = millis();
  if (now - last_oled_update_ms < 1200U) return;
  last_oled_update_ms = now;

  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0, 0);
  oled.println("Sentinel Hub");

  oled.setCursor(0, 10);
  oled.print("IP: ");
  if (WiFi.status() == WL_CONNECTED) oled.println(WiFi.localIP().toString());
  else oled.println("offline");

  oled.setCursor(0, 20);
  oled.print("WiFi: ");
  oled.print(WiFi.RSSI());
  oled.println(" dBm");

  oled.setCursor(0, 32);
  oled.print("pH:");
  oled.print(short_val(chem.ph, 2));
  oled.print(" KH:");
  oled.println(short_val(chem.kh, 1));

  oled.setCursor(0, 42);
  oled.print("T:");
  oled.print(short_val(chem.temp_aq, 1));
  oled.print(" EC:");
  oled.println(short_val(chem.ec, 0));

  oled.display();
}

bool build_cloud_payload(String *payload_out, String *err_out = nullptr) {
  if (cfg.reef_api_key.isEmpty() || cfg.reef_tank_id.isEmpty()) {
    if (err_out) *err_out = "Missing reef_api_key or reef_tank_id";
    return false;
  }
  JsonDocument doc;
  doc["tank_id"] = cfg.reef_tank_id;
  doc["device_id"] = cfg.reef_device_id;
  String iso = now_iso_utc();
  if (iso.length()) doc["timestamp"] = iso;
  doc["timestamp_unix"] = static_cast<int64_t>(time(nullptr));
  JsonArray arr = doc["measurements"].to<JsonArray>();

  auto add_measurement = [&](const char *p, float v, const char *u) {
    JsonObject m = arr.add<JsonObject>();
    m["parameter"] = p;
    m["value"] = v;
    m["unit"] = u;
  };

  add_measurement("ph", chem.ph, "");
  add_measurement("kh", chem.kh, "dKH");
  add_measurement("temp_aquarium", chem.temp_aq, "C");
  add_measurement("temp_sump", chem.temp_sump, "C");
  add_measurement("temp_room", chem.temp_room, "C");
  add_measurement("ec", chem.ec, "uS/cm");

  serializeJson(doc, *payload_out);
  return true;
}

bool send_cloud_payload(const String &payload, String *err_out = nullptr) {
  if (WiFi.status() != WL_CONNECTED) {
    if (err_out) *err_out = "No Wi-Fi";
    return false;
  }
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  if (!http.begin(client, cfg.reef_webhook_url)) {
    if (err_out) *err_out = "HTTP begin failed";
    return false;
  }
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-API-Key", cfg.reef_api_key);
  int code = http.POST(payload);
  String body = http.getString();
  http.end();

  if (code >= 200 && code < 300) {
    if (err_out) *err_out = "HTTP " + String(code);
    return true;
  }
  if (err_out) {
    *err_out = "HTTP " + String(code);
    if (body.length()) *err_out += " " + body.substring(0, 120);
  }
  return false;
}

void enqueue_cloud_payload(const String &payload) {
  if (cloud_queue.size() >= CLOUD_QUEUE_MAX) {
    cloud_queue.pop_front();  // oldest-drop
  }
  CloudQueueItem it;
  it.payload = payload;
  it.retry_step = 0;
  it.next_try_ms = millis();
  cloud_queue.push_back(it);
}

void enqueue_cloud_retry_after_fail(const String &payload) {
  if (cloud_queue.size() >= CLOUD_QUEUE_MAX) {
    cloud_queue.pop_front();  // oldest-drop
  }
  CloudQueueItem it;
  it.payload = payload;
  it.retry_step = 1;  // first retry slot after immediate failure is +1m
  it.next_try_ms = millis() + 60U * 1000U;
  cloud_queue.push_back(it);
}

void schedule_retry(CloudQueueItem &it) {
  static const uint32_t backoff_ms[] = {60U * 1000U, 5U * 60U * 1000U, 15U * 60U * 1000U};
  if (it.retry_step < 3) {
    it.next_try_ms = millis() + backoff_ms[it.retry_step];
    it.retry_step++;
  } else {
    it.next_try_ms = millis() + backoff_ms[2];
  }
}

void process_cloud_queue_once() {
  if (cloud_queue.empty()) return;
  if (WiFi.status() != WL_CONNECTED) return;
  CloudQueueItem &front = cloud_queue.front();
  if (millis() < front.next_try_ms) return;

  String err;
  last_cloud_sync_try_ms = millis();
  bool ok = send_cloud_payload(front.payload, &err);
  last_cloud_sync_ok = ok;
  last_cloud_sync_message = err;
  if (ok) {
    last_cloud_sync_ms = millis();
    cloud_queue.pop_front();
  } else {
    schedule_retry(front);
  }
}

bool connect_saved_wifi(uint32_t timeout_ms) {
  Serial.println("[reef-hub] trying saved Wi-Fi...");
  WiFi.begin();
  const uint32_t t0 = millis();
  while (millis() - t0 < timeout_ms) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("[reef-hub] connected to saved Wi-Fi, IP: ");
      Serial.println(WiFi.localIP());
      return true;
    }
    delay(400);
  }
  Serial.println("[reef-hub] saved Wi-Fi connect timeout");
  return false;
}

const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="pl">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Reef Sentinel Hub</title>
  <style>
    :root { color-scheme: dark; }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      font-family: "Segoe UI", Tahoma, sans-serif;
      color: #eaf8ff;
      background: radial-gradient(circle at 55% 0%, #0a2b63 0%, #081a3a 38%, #030916 100%);
    }
    .wrap { max-width: 1240px; margin: 0 auto; padding: 20px; }
    .head { display:flex; align-items:center; justify-content:space-between; gap:10px; margin-bottom: 12px; flex-wrap: wrap; }
    .brand { display:flex; align-items:center; gap: 12px; }
    .brand img {
      width: 72px;
      height: 72px;
      object-fit: contain;
      object-position: center;
      border-radius: 12px;
      filter: drop-shadow(0 0 10px rgba(74, 210, 255, 0.4));
    }
    h1 { margin:0; font-size: 38px; font-weight: 800; }
    .sub { color:#9dddf3; margin-top:4px; }
    .pill {
      border:1px solid rgba(41,212,255,.45);
      border-radius: 999px;
      padding: 8px 12px;
      color:#caf4ff;
      font-weight: 700;
    }
    .grid { display:grid; gap:12px; grid-template-columns: repeat(auto-fit,minmax(210px,1fr)); margin: 16px 0; }
    .card {
      border:1px solid rgba(41,212,255,.24);
      border-radius:14px;
      background: rgba(4,13,28,.78);
      padding: 12px;
    }
    .panel { margin-top: 12px; }
    .panel h2 { margin: 0 0 10px; font-size: 18px; }
    .tabs { display:flex; gap:8px; flex-wrap:wrap; margin: 8px 0 14px; }
    .tab-btn {
      border:1px solid rgba(41,212,255,.38);
      border-radius: 12px;
      background: rgba(6,22,45,.65);
      color:#bdefff;
      padding: 8px 12px;
      font-weight: 700;
      cursor: pointer;
    }
    .tab-btn.active {
      background: rgba(35,176,235,.35);
      color: #ffffff;
      border-color: rgba(87,225,255,.7);
    }
    .tab-panel { display: none; }
    .tab-panel.active { display: block; }
    .row { display:flex; gap: 10px; flex-wrap: wrap; align-items: center; margin: 8px 0; }
    .row label { min-width: 220px; color:#9bcde2; }
    .row input {
      width: 140px;
      background: rgba(3,11,25,.8);
      border: 1px solid rgba(41,212,255,.28);
      color: #eaf8ff;
      border-radius: 8px;
      padding: 8px 10px;
    }
    .btn {
      border:1px solid rgba(41,212,255,.5);
      border-radius:10px;
      background: rgba(8,28,58,.8);
      color:#d7f7ff;
      padding: 8px 12px;
      font-weight: 700;
      cursor: pointer;
    }
    .btn.secondary { border-color: rgba(114,160,255,.5); }
    .chart-grid { display:grid; gap:10px; grid-template-columns: repeat(auto-fit,minmax(260px,1fr)); }
    .chart-card { border:1px solid rgba(41,212,255,.2); border-radius:12px; background: rgba(3,11,25,.7); padding:8px; }
    .msg { color:#9de2f7; min-height: 20px; }
    .k { color:#9bcde2; font-size:13px; margin:0; }
    .v { font-size:30px; font-weight:800; margin: 2px 0 0; }
  </style>
</head>
<body>
  <div class="wrap">
    <div class="head">
      <div class="brand">
        <img alt="Reef Sentinel" src="data:image/png;base64,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" />
        <div>
          <h1>Reef Sentinel Hub</h1>
        </div>
      </div>
      <div class="pill" id="host-pill">---</div>
    </div>
    <nav class="tabs" id="tabs">
      <button class="tab-btn active" data-tab="tab-dashboard">Dashboard</button>
      <button class="tab-btn" data-tab="tab-settings">Chem Settings</button>
      <button class="tab-btn" data-tab="tab-phcal">Kalibracja pH</button>
      <button class="tab-btn" data-tab="tab-pumps">Pompy i Testy</button>
      <button class="tab-btn" data-tab="tab-charts">Wykresy</button>
      <button class="tab-btn" data-tab="tab-cloud">Cloud</button>
    </nav>

    <section class="card panel tab-panel active" id="tab-dashboard">
      <h2>Parametry główne</h2>
      <section class="grid" id="cards_tab"></section>
    </section>

    <section class="card panel tab-panel" id="tab-settings">
      <h2>Chem Module Settings</h2>
      <div class="row"><label>Chem host/IP</label><input id="chem_host" /></div>
      <div class="row"><label>Chem publish interval [s]</label><input id="chem_pub_s" type="number" min="10" max="3600" /></div>
      <div class="row"><label>Chem KH interval [h]</label><input id="chem_kh_h" type="number" min="0" max="24" /></div>
      <div class="row"><button class="btn" id="save_settings">Zapisz ustawienia</button><button class="btn secondary" id="apply_settings">Wyślij do Chem (Apply)</button></div>
    </section>

    <section class="card panel tab-panel" id="tab-phcal">
      <h2>Kalibracja pH (2 płyny)</h2>
      <div class="row"><label>Liquid 1 pH</label><input id="ph_l1" type="number" step="0.01" /><button class="btn" id="capture_l1">Capture Liquid 1</button></div>
      <div class="row"><label>Liquid 2 pH</label><input id="ph_l2" type="number" step="0.01" /><button class="btn" id="capture_l2">Capture Liquid 2</button></div>
    </section>

    <section class="card panel tab-panel" id="tab-pumps">
      <h2>Kalibracja pomp dozujących</h2>
      <div class="row">
        <button class="btn" id="pump_hcl">Run HCL</button>
        <button class="btn" id="pump_waste">Run Waste</button>
        <button class="btn" id="pump_ro">Run RO</button>
        <button class="btn" id="pump_sample">Run Sample</button>
      </div>
      <div class="row">
        <button class="btn secondary" id="start_kh_test">Start Test KH</button>
      </div>
      <div class="msg" id="msg"></div>
    </section>

    <section class="card panel tab-panel" id="tab-charts">
      <h2>Podstawowe wykresy</h2>
      <div class="chart-grid" id="charts"></div>
    </section>

    <section class="card panel tab-panel" id="tab-cloud">
      <h2>Integracja reef-sentinel.com</h2>
      <div class="row"><label>Webhook URL</label><input id="reef_webhook_url" style="width:420px" /></div>
      <div class="row"><label>API Key</label><input id="reef_api_key" type="password" style="width:420px" /></div>
      <div class="row"><label>Tank ID</label><input id="reef_tank_id" style="width:220px" /></div>
      <div class="row"><label>Device ID</label><input id="reef_device_id" style="width:220px" /></div>
      <div class="row"><label>Cloud sync interval [min]</label><input id="cloud_sync_interval_min" type="number" min="1" max="240" /></div>
      <div class="row">
        <button class="btn" id="save_cloud">Zapisz cloud</button>
        <button class="btn secondary" id="sync_now">Wyslij teraz</button>
      </div>
      <div class="msg" id="cloud_msg"></div>
    </section>
  </div>

  <script>
    const cards = [
      ["pH", "ph", "", 2, "#47d6ff"],
      ["KH", "kh", "dKH", 1, "#86e7ff"],
      ["Temperatura Akwarium", "temp_aq", "C", 1, "#82b9ff"],
      ["Temperatura Sump", "temp_sump", "C", 1, "#629fff"],
      ["Temperatura Komora", "temp_room", "C", 1, "#4b84e3"],
      ["EC Zasolenie", "ec", "", 0, "#46efca"]
    ];
    const hist = {};

    function msg(text) {
      const el = document.getElementById("msg");
      if (el) el.textContent = text || "";
    }

    function cloudMsg(text) {
      const el = document.getElementById("cloud_msg");
      if (el) el.textContent = text || "";
    }

    async function postJson(url, body) {
      const r = await fetch(url, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(body || {})
      });
      return await r.json();
    }

    function spark(values, color) {
      const w = 220, h = 64;
      if (!values.length) return `<svg viewBox="0 0 ${w} ${h}" width="100%" height="${h}"></svg>`;
      let min = Math.min(...values), max = Math.max(...values);
      if (min === max) { min -= 1; max += 1; }
      const pts = values.map((v, i) => {
        const x = (i / Math.max(values.length - 1, 1)) * (w - 8) + 4;
        const y = h - 4 - ((v - min) / (max - min)) * (h - 8);
        return `${x.toFixed(1)},${y.toFixed(1)}`;
      }).join(" ");
      return `<svg viewBox="0 0 ${w} ${h}" width="100%" height="${h}"><polyline fill="none" stroke="${color}" stroke-width="2.2" points="${pts}"></polyline></svg>`;
    }

    function render(data) {
      document.getElementById("host-pill").textContent = data.host || "reef-sentinel.local";
      const rootTab = document.getElementById("cards_tab");
      if (rootTab) rootTab.innerHTML = "";
      const charts = document.getElementById("charts");
      charts.innerHTML = "";
      for (const [label, key, unit, precision, color] of cards) {
        const value = Number(data[key]);
        const valText = Number.isFinite(value) ? value.toFixed(precision) + (unit ? " " + unit : "") : "--";
        if (!hist[key]) hist[key] = [];
        if (Number.isFinite(value)) hist[key].push(value);
        if (hist[key].length > 120) hist[key].shift();
        const el = document.createElement("article");
        el.className = "card";
        el.innerHTML = "<p class='k'>" + label + "</p><p class='v' style='color:" + color + "'>" + valText + "</p>";
        if (rootTab) rootTab.appendChild(el);

        const ch = document.createElement("article");
        ch.className = "chart-card";
        ch.innerHTML = "<p class='k'>" + label + " trend</p>" + spark(hist[key], color);
        charts.appendChild(ch);
      }
      document.getElementById("chem_host").value = data.chem_host || "";
      document.getElementById("chem_pub_s").value = data.chem_publish_interval_s ?? 30;
      document.getElementById("chem_kh_h").value = data.chem_kh_interval_h ?? 1;
      document.getElementById("ph_l1").value = data.ph_cal_liquid1 ?? 7.0;
      document.getElementById("ph_l2").value = data.ph_cal_liquid2 ?? 4.0;
      document.getElementById("reef_webhook_url").value = data.reef_webhook_url || "https://reef-sentinel.com/api/integrations/webhook";
      document.getElementById("reef_api_key").value = "";
      document.getElementById("reef_api_key").placeholder = data.reef_api_key_masked || "";
      document.getElementById("reef_tank_id").value = data.reef_tank_id || "";
      document.getElementById("reef_device_id").value = data.reef_device_id || "";
      document.getElementById("cloud_sync_interval_min").value = data.cloud_sync_interval_min ?? 15;
      const q = Number(data.cloud_queue_size || 0);
      const nextRetry = Number(data.cloud_next_retry_s || 0);
      cloudMsg(`Ostatni sync: ${data.last_cloud_sync_status || "never"} (${data.last_cloud_sync_message || ""}) | kolejka: ${q}${q > 0 ? " | retry za: " + nextRetry + "s" : ""}`);
    }

    async function saveSettings(sendToChem) {
      try {
        const body = {
          chem_host: document.getElementById("chem_host").value.trim(),
          chem_publish_interval_s: Number(document.getElementById("chem_pub_s").value),
          chem_kh_interval_h: Number(document.getElementById("chem_kh_h").value),
          ph_cal_liquid1: Number(document.getElementById("ph_l1").value),
          ph_cal_liquid2: Number(document.getElementById("ph_l2").value),
          push_to_chem: !!sendToChem
        };
        const res = await postJson("/api/module/chem/settings", body);
        msg(res.ok ? "Ustawienia zapisane" : "Błąd zapisu ustawień");
      } catch (e) {
        msg("Błąd połączenia");
      }
    }

    async function sendCommand(cmd) {
      try {
        const res = await postJson("/api/module/chem/command", { cmd });
        msg(res.ok ? ("Wykonano: " + cmd) : ("Nieudane: " + cmd));
      } catch (_) {
        msg("Błąd połączenia");
      }
    }

    async function saveCloud() {
      try {
        const body = {
          reef_webhook_url: document.getElementById("reef_webhook_url").value.trim(),
          reef_api_key: document.getElementById("reef_api_key").value.trim(),
          reef_tank_id: document.getElementById("reef_tank_id").value.trim(),
          reef_device_id: document.getElementById("reef_device_id").value.trim(),
          cloud_sync_interval_min: Number(document.getElementById("cloud_sync_interval_min").value)
        };
        const res = await postJson("/api/cloud/settings", body);
        cloudMsg(res.ok ? "Cloud config zapisana" : ("Blad: " + (res.message || "unknown")));
      } catch (_) {
        cloudMsg("Blad polaczenia");
      }
    }

    async function syncNow() {
      try {
        const res = await postJson("/api/cloud/sync_now", {});
        cloudMsg(res.ok ? ("Sync OK: " + (res.message || "")) : ("Sync FAILED: " + (res.message || "")));
      } catch (_) {
        cloudMsg("Blad polaczenia");
      }
    }

    async function tick() {
      try {
        const r = await fetch("/api/status");
        const data = await r.json();
        render(data);
      } catch (_) {}
    }

    function wire() {
      document.querySelectorAll(".tab-btn").forEach((btn) => {
        btn.onclick = () => {
          const tabId = btn.getAttribute("data-tab");
          document.querySelectorAll(".tab-btn").forEach((b) => b.classList.remove("active"));
          document.querySelectorAll(".tab-panel").forEach((p) => p.classList.remove("active"));
          btn.classList.add("active");
          const panel = document.getElementById(tabId);
          if (panel) panel.classList.add("active");
        };
      });
      document.getElementById("save_settings").onclick = () => saveSettings(false);
      document.getElementById("apply_settings").onclick = () => saveSettings(true);
      document.getElementById("capture_l1").onclick = async () => {
        await saveSettings(true);
        await sendCommand("ph_capture_1");
      };
      document.getElementById("capture_l2").onclick = async () => {
        await saveSettings(true);
        await sendCommand("ph_capture_2");
      };
      document.getElementById("pump_hcl").onclick = () => sendCommand("pump_cal_hcl");
      document.getElementById("pump_waste").onclick = () => sendCommand("pump_cal_waste");
      document.getElementById("pump_ro").onclick = () => sendCommand("pump_cal_ro");
      document.getElementById("pump_sample").onclick = () => sendCommand("pump_cal_sample");
      document.getElementById("start_kh_test").onclick = () => sendCommand("start_kh_test");
      document.getElementById("save_cloud").onclick = () => saveCloud();
      document.getElementById("sync_now").onclick = () => syncNow();
    }

    wire();
    tick();
    setInterval(tick, 3000);
  </script>
</body>
</html>
)HTML";

void send_json_ok(bool ok, const String &msg = "") {
  JsonDocument doc;
  doc["ok"] = ok;
  if (msg.length()) {
    doc["message"] = msg;
  }
  String payload;
  serializeJson(doc, payload);
  server.send(200, "application/json", payload);
}

void setup_routes() {
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html; charset=utf-8", INDEX_HTML);
  });

  server.on("/api/status", HTTP_GET, []() {
    JsonDocument doc;
    doc["host"] = "reef-sentinel.local";
    doc["ip"] = WiFi.localIP().toString();
    doc["wifi_rssi"] = WiFi.RSSI();
    doc["uptime_s"] = millis() / 1000;
    doc["ph"] = chem.ph;
    doc["kh"] = chem.kh;
    doc["temp_aq"] = chem.temp_aq;
    doc["temp_sump"] = chem.temp_sump;
    doc["temp_room"] = chem.temp_room;
    doc["ec"] = chem.ec;
    doc["kh_test_active"] = chem.kh_test_active;
    doc["last_kh_test"] = chem.last_kh_test;
    doc["chem_host"] = cfg.chem_host;
    doc["chem_port"] = cfg.chem_port;
    doc["chem_publish_interval_s"] = cfg.chem_publish_interval_s;
    doc["chem_kh_interval_h"] = cfg.chem_kh_interval_h;
    doc["ph_cal_liquid1"] = cfg.ph_cal_liquid1;
    doc["ph_cal_liquid2"] = cfg.ph_cal_liquid2;
    doc["reef_webhook_url"] = cfg.reef_webhook_url;
    doc["reef_api_key_masked"] = mask_api_key(cfg.reef_api_key);
    doc["reef_tank_id"] = cfg.reef_tank_id;
    doc["reef_device_id"] = cfg.reef_device_id;
    doc["cloud_sync_interval_min"] = cfg.cloud_sync_interval_min;
    doc["last_cloud_sync_status"] = last_cloud_sync_ok ? "ok" : "failed";
    doc["last_cloud_sync_message"] = last_cloud_sync_message;
    doc["last_cloud_sync_try_ms"] = last_cloud_sync_try_ms;
    doc["last_cloud_sync_ms"] = last_cloud_sync_ms;
    doc["cloud_queue_size"] = cloud_queue.size();
    if (!cloud_queue.empty()) {
      uint32_t now = millis();
      uint32_t due_in_ms = 0;
      if (cloud_queue.front().next_try_ms > now) {
        due_in_ms = cloud_queue.front().next_try_ms - now;
      }
      doc["cloud_next_retry_s"] = due_in_ms / 1000U;
    } else {
      doc["cloud_next_retry_s"] = 0;
    }

    String payload;
    serializeJson(doc, payload);
    server.send(200, "application/json", payload);
  });

  server.on("/api/module/chem/report", HTTP_POST, []() {
    if (!server.hasArg("plain")) {
      send_json_ok(false, "Missing JSON body");
      return;
    }
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, server.arg("plain"));
    if (err) {
      send_json_ok(false, "Invalid JSON");
      return;
    }
    if (doc["ph"].is<float>()) chem.ph = doc["ph"].as<float>();
    if (doc["kh"].is<float>()) chem.kh = doc["kh"].as<float>();
    if (doc["temp_aq"].is<float>()) chem.temp_aq = doc["temp_aq"].as<float>();
    if (doc["temp_sump"].is<float>()) chem.temp_sump = doc["temp_sump"].as<float>();
    if (doc["temp_room"].is<float>()) chem.temp_room = doc["temp_room"].as<float>();
    if (doc["ec"].is<float>()) chem.ec = doc["ec"].as<float>();
    if (doc["kh_test_active"].is<bool>()) chem.kh_test_active = doc["kh_test_active"].as<bool>();
    if (doc["last_kh_test"].is<const char *>()) chem.last_kh_test = doc["last_kh_test"].as<const char *>();
    chem.last_update_ms = millis();
    send_json_ok(true);
  });

  server.on("/api/module/chem/settings", HTTP_POST, []() {
    if (!server.hasArg("plain")) {
      send_json_ok(false, "Missing JSON body");
      return;
    }
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, server.arg("plain"));
    if (err) {
      send_json_ok(false, "Invalid JSON");
      return;
    }

    if (doc["chem_host"].is<const char *>()) cfg.chem_host = doc["chem_host"].as<const char *>();
    if (doc["chem_port"].is<uint16_t>()) cfg.chem_port = doc["chem_port"].as<uint16_t>();
    if (doc["chem_publish_interval_s"].is<int>()) cfg.chem_publish_interval_s = doc["chem_publish_interval_s"].as<int>();
    if (doc["chem_kh_interval_h"].is<int>()) cfg.chem_kh_interval_h = doc["chem_kh_interval_h"].as<int>();
    if (doc["ph_cal_liquid1"].is<float>()) cfg.ph_cal_liquid1 = doc["ph_cal_liquid1"].as<float>();
    if (doc["ph_cal_liquid2"].is<float>()) cfg.ph_cal_liquid2 = doc["ph_cal_liquid2"].as<float>();
    save_config();

    bool push = doc["push_to_chem"].is<bool>() && doc["push_to_chem"].as<bool>();
    if (push) {
      bool ok = true;
      ok = ok && forward_chem_number("publish_interval_s", String(cfg.chem_publish_interval_s));
      ok = ok && forward_chem_number("kh_interval_h", String(cfg.chem_kh_interval_h));
      ok = ok && forward_chem_number("ph_cal_liquid1", String(cfg.ph_cal_liquid1, 2));
      ok = ok && forward_chem_number("ph_cal_liquid2", String(cfg.ph_cal_liquid2, 2));
      ok = ok && forward_chem_command("/button/apply_settings/press");
      send_json_ok(ok, ok ? "Pushed to chem" : "Push to chem failed");
      return;
    }

    send_json_ok(true, "Saved");
  });

  server.on("/api/module/chem/command", HTTP_POST, []() {
    if (!server.hasArg("plain")) {
      send_json_ok(false, "Missing JSON body");
      return;
    }
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, server.arg("plain"));
    if (err || !doc["cmd"].is<const char *>()) {
      send_json_ok(false, "Invalid JSON");
      return;
    }
    const String cmd = doc["cmd"].as<const char *>();
    bool ok = false;

    if (cmd == "start_kh_test") ok = forward_chem_command("/button/start_kh_test/press");
    else if (cmd == "apply_settings") ok = forward_chem_command("/button/apply_settings/press");
    else if (cmd == "ph_capture_1") {
      ok = true;
      ok = ok && forward_chem_number("ph_cal_liquid1", String(cfg.ph_cal_liquid1, 2));
      ok = ok && forward_chem_number("ph_cal_liquid2", String(cfg.ph_cal_liquid2, 2));
      ok = ok && forward_chem_command("/button/apply_settings/press");
      ok = ok && forward_chem_command("/button/ph_cal_capture_liquid1/press");
    } else if (cmd == "ph_capture_2") {
      ok = true;
      ok = ok && forward_chem_number("ph_cal_liquid1", String(cfg.ph_cal_liquid1, 2));
      ok = ok && forward_chem_number("ph_cal_liquid2", String(cfg.ph_cal_liquid2, 2));
      ok = ok && forward_chem_command("/button/apply_settings/press");
      ok = ok && forward_chem_command("/button/ph_cal_capture_liquid2/press");
    }
    else if (cmd == "pump_cal_hcl") ok = forward_chem_command("/button/pump_cal_run_hcl/press");
    else if (cmd == "pump_cal_waste") ok = forward_chem_command("/button/pump_cal_run_waste/press");
    else if (cmd == "pump_cal_ro") ok = forward_chem_command("/button/pump_cal_run_ro/press");
    else if (cmd == "pump_cal_sample") ok = forward_chem_command("/button/pump_cal_run_sample/press");
    else {
      send_json_ok(false, "Unknown command");
      return;
    }

    send_json_ok(ok, ok ? "OK" : "Command failed");
  });

  server.on("/api/cloud/settings", HTTP_POST, []() {
    if (!server.hasArg("plain")) {
      send_json_ok(false, "Missing JSON body");
      return;
    }
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, server.arg("plain"));
    if (err) {
      send_json_ok(false, "Invalid JSON");
      return;
    }

    if (doc["reef_webhook_url"].is<const char *>()) cfg.reef_webhook_url = doc["reef_webhook_url"].as<const char *>();
    if (doc["reef_api_key"].is<const char *>()) {
      String k = doc["reef_api_key"].as<const char *>();
      if (k.length() > 0) cfg.reef_api_key = k;
    }
    if (doc["reef_tank_id"].is<const char *>()) cfg.reef_tank_id = doc["reef_tank_id"].as<const char *>();
    if (doc["reef_device_id"].is<const char *>()) cfg.reef_device_id = doc["reef_device_id"].as<const char *>();
    if (doc["cloud_sync_interval_min"].is<int>()) cfg.cloud_sync_interval_min = doc["cloud_sync_interval_min"].as<int>();
    if (cfg.cloud_sync_interval_min < 1) cfg.cloud_sync_interval_min = 1;
    if (cfg.cloud_sync_interval_min > 240) cfg.cloud_sync_interval_min = 240;
    save_config();
    send_json_ok(true, "Cloud settings saved");
  });

  server.on("/api/cloud/sync_now", HTTP_POST, []() {
    String payload;
    String err;
    if (!build_cloud_payload(&payload, &err)) {
      last_cloud_sync_ok = false;
      last_cloud_sync_message = err;
      send_json_ok(false, err);
      return;
    }

    last_cloud_sync_try_ms = millis();
    const bool ok = send_cloud_payload(payload, &err);
    last_cloud_sync_ok = ok;
    if (ok) {
      last_cloud_sync_ms = millis();
      last_cloud_sync_message = err;
      send_json_ok(true, err);
      return;
    }

    enqueue_cloud_retry_after_fail(payload);
    last_cloud_sync_message = err + " (queued)";
    send_json_ok(false, last_cloud_sync_message);
  });

  server.onNotFound([]() {
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n[reef-hub] boot");
  load_config();
  oled_init();

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  bool connected = connect_saved_wifi(90000);

  if (!connected) {
    WiFiManager wm;
    wm.setConfigPortalBlocking(true);
    wm.setConfigPortalTimeout(180);
    wm.setConnectRetries(20);
    wm.setMinimumSignalQuality(8);
    bool ok = wm.autoConnect("SentinelHub");
    if (!ok) {
      Serial.println("[reef-hub] WiFi config timeout, restarting...");
      ESP.restart();
    }
  }

  if (!MDNS.begin("reef-sentinel")) {
    Serial.println("[reef-hub] mDNS failed");
  } else {
    Serial.println("[reef-hub] mDNS ready: http://reef-sentinel.local");
  }

  if (cfg.reef_device_id == "hub_unknown" || cfg.reef_device_id.length() == 0) {
    cfg.reef_device_id = default_device_id();
    save_config();
  }

  configTime(0, 0, "pool.ntp.org", "time.google.com");

  setup_routes();
  server.begin();
  Serial.print("[reef-hub] IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  server.handleClient();
  oled_render();

  const bool wifi_ok = WiFi.status() == WL_CONNECTED;
  const uint32_t now = millis();
  if (wifi_ok) {
    wifi_disconnected_since_ms = 0;
  } else {
    if (wifi_disconnected_since_ms == 0) {
      wifi_disconnected_since_ms = now;
    } else if (now - wifi_disconnected_since_ms > 10000) {
      // After 10s offline, retry saved Wi-Fi silently (without forcing portal).
      WiFi.reconnect();
      wifi_disconnected_since_ms = now;
    }
  }

  // Periodic snapshot enqueue (works also offline; queue is flushed when Wi-Fi returns).
  if (cfg.cloud_sync_interval_min > 0 &&
      cfg.reef_api_key.length() > 0 &&
      cfg.reef_tank_id.length() > 0) {
    const uint32_t interval_ms = static_cast<uint32_t>(cfg.cloud_sync_interval_min) * 60U * 1000U;
    if (last_cloud_enqueue_ms == 0 || (now - last_cloud_enqueue_ms) >= interval_ms) {
      String payload;
      String err;
      if (build_cloud_payload(&payload, &err)) {
        enqueue_cloud_payload(payload);
        last_cloud_enqueue_ms = now;
      } else {
        last_cloud_sync_ok = false;
        last_cloud_sync_message = err;
      }
    }
  }

  process_cloud_queue_once();
}

