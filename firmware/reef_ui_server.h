#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/components/web_server_base/web_server_base.h"

class ReefUiHandler : public AsyncWebHandler {
 public:
  bool canHandle(AsyncWebServerRequest *request) const override {
    if (request->method() != HTTP_GET) {
      return false;
    }
    const std::string url = request->url();
    return url == "/app" || url == "/app/" || url == "/service";
  }

  void handleRequest(AsyncWebServerRequest *request) override {
    const std::string url = request->url();
    if (url == "/service") {
      request->redirect("/?service=1");
      return;
    }

    static const char APP_HTML[] =
        "<!doctype html><html><head>"
        "<meta charset='utf-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>Reef Sentinel Hub</title>"
        "<link rel='stylesheet' href='/0.css?v=reefhub3'>"
        "</head><body>"
        "<div id='reef-app-root'></div>"
        "<script src='/0.js?v=reefhub3'></script>"
        "</body></html>";

    request->send(200, "text/html; charset=utf-8", APP_HTML);
  }
};

class ReefUiServer : public esphome::Component {
 public:
  void setup() override {
    if (esphome::web_server_base::global_web_server_base == nullptr) {
      ESP_LOGW("reef_ui", "web_server_base not ready; custom /app endpoint not installed");
      return;
    }
    esphome::web_server_base::global_web_server_base->add_handler(new ReefUiHandler());
    ESP_LOGI("reef_ui", "Custom Reef UI endpoints ready: /app and /service");
  }

  float get_setup_priority() const override { return esphome::setup_priority::WIFI + 1.0f; }
};
