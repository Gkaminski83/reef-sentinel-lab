(() => {
  const LOGO_DATA_URI =
    "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAgCAYAAABU1PscAAADk0lEQVR42u2Xz29UVRTHz7n33Zl5M53X6e9OC5RSqrR0KEiFQEMIGgiRGGhicGOicWHjxhhMTHShaMQV3WhiQtKF0bjTxDRGhRi1CMS0tRawtCVTW8q0A22lZX69eW/eucc/QJbTOMb5bO/NOfeTs/ieC1CmTJky/ya4EUUvMCvXdWNhKQ+y1sJT6moC4OZ7iG7JCryRTIb8NY07wwo6fURb8q50J+6DAADcXQdcEQDDlXDXAZhcuweTA1HMFqOvUSwB17TeySX/So1OrIqFVMUWx/DtV4q7hJAw4tCMSd61zeE0d+yuj5oRsw8A3iopAVtp59LHl8JU3fZmtWUj5jKQtx0Q5OZ2NEfMxSy/eDth4PTwxPmTZ49ni9W3aALpAjm+qlAouz6PQZcBQf9aYYZ+3lZXCR+c3lv17pe/dH39x0qvGUQEx86VnEDOgZSqQtTxP6Gxs+2bM32Hv+porF11tbfZE6K+Unoj+cWZJ2LPHbkfsMLLxeorilVIs5yqjrUB3Vu4/kx361SNZT59a26pemn1wcuLd5PplYU73VbEGGg/EjuQXctcKTkBSjkjodbWWsPQ3yXicSO7ngoLL9exvrjkfjZ0MTvbEr186Gz/sbW8N/tJQ8NcSebAU7PpFx7OJ7baHw3s6ulorzt66vhvQwGLlkPmfr+EJsF63Of3DQ5tqv2hJAUO/8RGrjJ3bnVs7AZZVlewteW0ZDsTUHo8GFRBU3FC6Ma3v38MnZJN4l0XkyEX/C/lHToPSn/hUxgJVfl6Az7nRx9C/3BXQ6bkV4m9F1it1Czt03nndX+t9aS/Qp671RsZBET+Ty1a2z+/bT0+OBUumQe9Msbq5PV07MTv2eZHnffdzHS3zHGAmZuYeeej7jCzYuZNzFz/jzS37a3MHNiwILujUtsEyFcF6tSJ8cxljdCMSo0yFXqAOe0QP3/GD/3geR+S1leZOQhEewpaTwshepg5rrU+xp53BaTUnuc1MfMkERmIGEHEdgD4FAAWNkSA0Y/5go5K1sDMp6RSLAq0D9AIAVFcCx5/LQpr5OphFuIgER1FxoeI2CkB4h7AIdDgAEAFM+9B5mnN/CwitgDAPDODbdu8YRMg21kRKN8nicIgIE/D9gDDaAH0AYnyhsGeAgBiIZYB4FsimpVS7tCkZ0DrtBYiYgio9ISQzDxlECVJiAgihokoIKV8YJrmevmbVqZMmTJl/jf8DfDur1iVX1eLAAAAAElFTkSuQmCC";

  const METRICS = [
    { key: "ph", labels: ["pH"], unit: "", precision: 2, color: "#46d7ff" },
    { key: "kh", labels: ["KH"], unit: "dKH", precision: 1, color: "#8de8ff" },
    { key: "temp_aq", labels: ["Temperatura Akwarium"], unit: "C", precision: 1, color: "#80baff" },
    { key: "temp_sump", labels: ["Temperatura Sump"], unit: "C", precision: 1, color: "#629fff" },
    { key: "temp_room", labels: ["Temperatura Komora"], unit: "C", precision: 1, color: "#4c83e2" },
    { key: "ec", labels: ["EC Zasolenie"], unit: "", precision: 0, color: "#47efcc" },
  ];

  const STORAGE_KEY = "reef_sentinel_app_history_v1";
  const MAX_POINTS = 180;
  const SAMPLE_MS = 10_000;

  const params = new URLSearchParams(window.location.search);
  const serviceMode = params.get("service") === "1";
  if (serviceMode) {
    return;
  }

  function loadHistory() {
    try {
      const raw = localStorage.getItem(STORAGE_KEY);
      return raw ? JSON.parse(raw) : {};
    } catch {
      return {};
    }
  }

  function saveHistory(history) {
    try {
      localStorage.setItem(STORAGE_KEY, JSON.stringify(history));
    } catch {
      // ignore storage failures
    }
  }

  function parseValue(text) {
    if (!text) return NaN;
    const m = text.replace(",", ".").match(/-?\d+(\.\d+)?/g);
    return m && m.length ? Number(m[m.length - 1]) : NaN;
  }

  function findValueInServiceDoc(doc, labels) {
    const rows = doc.querySelectorAll("tr, li, .row, .entity-row, .item");
    for (const row of rows) {
      const txt = (row.textContent || "").trim();
      if (!txt) continue;
      if (!labels.some((l) => txt.includes(l))) continue;
      const v = parseValue(txt);
      if (Number.isFinite(v)) return v;
    }
    return NaN;
  }

  function sparkline(values, color) {
    const w = 240;
    const h = 70;
    if (!values.length) return `<svg viewBox="0 0 ${w} ${h}" width="100%" height="${h}"></svg>`;
    let min = Math.min(...values);
    let max = Math.max(...values);
    if (min === max) {
      min -= 1;
      max += 1;
    }
    const points = values
      .map((v, i) => {
        const x = (i / Math.max(values.length - 1, 1)) * (w - 8) + 4;
        const y = h - 4 - ((v - min) / (max - min)) * (h - 8);
        return `${x.toFixed(1)},${y.toFixed(1)}`;
      })
      .join(" ");
    return `
      <svg viewBox="0 0 ${w} ${h}" width="100%" height="${h}">
        <rect x="0" y="0" width="${w}" height="${h}" rx="10" ry="10" fill="rgba(3,11,25,0.72)"></rect>
        <polyline fill="none" stroke="${color}" stroke-width="2.2" points="${points}"></polyline>
      </svg>
    `;
  }

  function buildLayout() {
    document.body.innerHTML = `
      <style>
        :root { color-scheme: dark; }
        html, body { margin:0; padding:0; }
        body {
          min-height: 100vh;
          color: #e7f7ff;
          font-family: "Segoe UI", Tahoma, sans-serif;
          background: radial-gradient(circle at 58% 0%, #0a2b63 0%, #081a3a 38%, #030916 100%);
        }
        .wrap { max-width: 1280px; margin: 0 auto; padding: 20px; }
        .head { display:flex; align-items:center; justify-content:space-between; gap:10px; flex-wrap:wrap; }
        .brand { display:flex; align-items:center; gap:12px; }
        .brand img { width:44px; height:44px; object-fit:contain; }
        h1 { margin:0; font-size:38px; font-weight:800; }
        .sub { margin:2px 0 0; color:#9fd8ee; font-size:14px; }
        .btn {
          border: 1px solid rgba(45,208,255,0.55);
          border-radius: 12px;
          background: rgba(6,22,45,0.85);
          color: #d6f7ff;
          padding: 10px 14px;
          text-decoration:none;
          font-weight:700;
        }
        .grid { display:grid; gap:12px; grid-template-columns:repeat(auto-fit,minmax(210px,1fr)); margin:16px 0; }
        .card {
          border:1px solid rgba(41,212,255,0.25);
          border-radius:14px;
          background: rgba(4,13,28,0.78);
          padding:12px;
        }
        .k { margin:0; color:#99cbe0; font-size:13px; }
        .v { margin:2px 0 0; font-size:30px; font-weight:800; }
        .panel {
          border:1px solid rgba(41,212,255,0.25);
          border-radius:14px;
          background: rgba(4,13,28,0.75);
          padding:12px;
        }
        .panel h3 { margin:0 0 10px; color:#d4f7ff; font-size:16px; }
        .line { color:#9cddee; font-size:13px; }
        iframe#service-probe { width:1px; height:1px; opacity:0; border:0; position:absolute; left:-9999px; top:-9999px; }
      </style>
      <div class="wrap">
        <header class="head">
          <div class="brand">
            <img src="${LOGO_DATA_URI}" alt="Reef Sentinel">
            <div>
              <h1>Reef Sentinel Hub</h1>
              <p class="sub">Lokalny panel huba (ESP32)</p>
            </div>
          </div>
          <a class="btn" href="/?service=1">Tryb serwisowy ESPHome</a>
        </header>
        <section class="grid" id="kpi-grid"></section>
        <section class="panel">
          <h3>Podstawowe wykresy lokalne</h3>
          <div class="grid" id="chart-grid"></div>
          <p class="line">Rozszerzona historia i analityka w Reef Sentinel App / HA (opcjonalnie).</p>
        </section>
      </div>
      <iframe id="service-probe" src="/?service=1"></iframe>
    `;
  }

  function render(history) {
    const kpi = document.getElementById("kpi-grid");
    const charts = document.getElementById("chart-grid");
    if (!kpi || !charts) return;
    kpi.innerHTML = "";
    charts.innerHTML = "";

    for (const m of METRICS) {
      const series = Array.isArray(history[m.key]) ? history[m.key] : [];
      const values = series.map((x) => x.v).filter((v) => Number.isFinite(v));
      const last = values.length ? values[values.length - 1] : NaN;
      const valText = Number.isFinite(last) ? `${last.toFixed(m.precision)}${m.unit ? ` ${m.unit}` : ""}` : "--";

      const card = document.createElement("article");
      card.className = "card";
      card.innerHTML = `<p class="k">${m.labels[0]}</p><p class="v" style="color:${m.color}">${valText}</p>`;
      kpi.appendChild(card);

      const chart = document.createElement("article");
      chart.className = "card";
      chart.innerHTML = `<p class="k">${m.labels[0]} trend</p>${sparkline(values, m.color)}`;
      charts.appendChild(chart);
    }
  }

  function sample() {
    const iframe = document.getElementById("service-probe");
    const doc = iframe && iframe.contentDocument ? iframe.contentDocument : null;
    if (!doc) return;

    const history = loadHistory();
    const now = Date.now();
    for (const m of METRICS) {
      const v = findValueInServiceDoc(doc, m.labels);
      if (!Number.isFinite(v)) continue;
      if (!Array.isArray(history[m.key])) history[m.key] = [];
      history[m.key].push({ t: now, v });
      if (history[m.key].length > MAX_POINTS) {
        history[m.key] = history[m.key].slice(history[m.key].length - MAX_POINTS);
      }
    }
    saveHistory(history);
    render(history);
  }

  function start() {
    buildLayout();
    render(loadHistory());
    setTimeout(sample, 1200);
    setInterval(sample, SAMPLE_MS);
  }

  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", start);
  } else {
    start();
  }
})();
