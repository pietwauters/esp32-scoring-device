//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "AtlasAsyncMqttClient.h"
#include <iostream>
#include <Preferences.h>
#include "esp_task_wdt.h"
#include <cstring>

static const char *TAG = "ATLAS_MQTT";

AtlasAsyncMqttClient::AtlasAsyncMqttClient() {}

AtlasAsyncMqttClient::~AtlasAsyncMqttClient() {
    if (client) {
        esp_mqtt_client_stop(client);
        esp_mqtt_client_destroy(client);
        client = nullptr;
    }
}

void AtlasAsyncMqttClient::setServer(IPAddress ip, uint16_t port) {
    m_host = m_host = std::string(ip.toString().c_str());
    m_port = port;
}

void AtlasAsyncMqttClient::setCredentials(const char* username, const char* password) {
    m_username = username ? username : "";
    m_password = password ? password : "";
}

void AtlasAsyncMqttClient::setClientId(const char* clientId) {
    m_clientId = clientId ? clientId : "";
}
void AtlasAsyncMqttClient::onConnect(mqtt_connect_cb_t cb) {
    connectCb = std::move(cb);
}

void AtlasAsyncMqttClient::onDisconnect(mqtt_disconnect_cb_t cb) {
    disconnectCb = std::move(cb);
}

void AtlasAsyncMqttClient::onMessage(mqtt_message_cb_t cb) {
    messageCb = std::move(cb);
}

void AtlasAsyncMqttClient::onSubscribe(mqtt_subscribe_cb_t cb) {
    subscribeCb = std::move(cb);
}

void AtlasAsyncMqttClient::publish(const char* topic, int qos, bool retain, const char* payload) {
    if (!client) return;
    esp_mqtt_client_publish(client, topic, payload, 0, qos, retain);
}

void AtlasAsyncMqttClient::publish(const char* topic, int qos, bool retain, const void* payload, size_t length) {
    if (!client) return;
    esp_mqtt_client_publish(client, topic, static_cast<const char*>(payload), length, qos, retain);
}

void AtlasAsyncMqttClient::publishString(const char* topic, int qos, bool retain, const std::string& payload) {
    publish(topic, qos, retain, payload.c_str(), payload.length());
}

void AtlasAsyncMqttClient::setWill(const char* topic, const char* message, int qos, bool retain) {
    m_lwtTopic = topic;
    m_lwtMessage = message;
    m_lwtQos = qos;
    m_lwtRetain = retain;
    m_lwtEnabled = true;
}


void AtlasAsyncMqttClient::subscribe(const char* topic, int qos) {
    if (!client) return;
    esp_mqtt_client_subscribe(client, topic, qos);
}

esp_err_t AtlasAsyncMqttClient::mqttEventHandlerCb(esp_mqtt_event_handle_t event) {
    auto* instance = static_cast<AtlasAsyncMqttClient*>(event->user_context);
    if (instance) {
        instance->handleEvent(event);
    }
    return ESP_OK;
}

void AtlasAsyncMqttClient::handleEvent(esp_mqtt_event_handle_t event) {
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            if (connectCb) connectCb(event->session_present);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            if (disconnectCb) disconnectCb();
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            if (messageCb) {
                std::string topic(event->topic, event->topic_len);
                std::string payload(event->data, event->data_len);
                messageCb(topic.c_str(), payload.c_str(), payload.size());
            }
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            if (subscribeCb) subscribeCb(event->msg_id, 1); // assume QoS 1
            break;
        default:
            break;
    }
}

void AtlasAsyncMqttClient::begin() {
    if (m_HasBegun) return;

    esp_mqtt_client_config_t mqtt_cfg = {};
    mqtt_cfg.host = m_host.c_str();
    mqtt_cfg.port = m_port;
    mqtt_cfg.username = m_username.empty() ? nullptr : m_username.c_str();
    mqtt_cfg.password = m_password.empty() ? nullptr : m_password.c_str();
    mqtt_cfg.client_id = m_clientId.empty() ? nullptr : m_clientId.c_str();
    mqtt_cfg.event_handle = mqttEventHandlerCb;
    mqtt_cfg.user_context = this;
    if (m_lwtEnabled) {
      mqtt_cfg.lwt_topic = m_lwtTopic.c_str();
      mqtt_cfg.lwt_msg = m_lwtMessage.c_str();
      mqtt_cfg.lwt_qos = m_lwtQos;
      mqtt_cfg.lwt_retain = m_lwtRetain;
  }

    client = esp_mqtt_client_init(&mqtt_cfg);
    if (client == nullptr) {
        ESP_LOGE(TAG, "Failed to create MQTT client");
        return;
    }

    esp_mqtt_client_start(client);
    m_HasBegun = true;
}

void AtlasAsyncMqttClient::clearWill() {
    m_lwtTopic.clear();
    m_lwtMessage.clear();
    m_lwtQos = 0;
    m_lwtRetain = false;
    m_lwtEnabled = false;
}


void AtlasAsyncMqttClient::disconnect() {
    if (client) {
        esp_mqtt_client_stop(client);
        esp_mqtt_client_destroy(client);
        client = nullptr;
        m_HasBegun = false;
        ESP_LOGI(TAG, "MQTT client disconnected and destroyed");
    }
}
void AtlasAsyncMqttClient::reconnectWithNewSettings() {
    if (client) {
        esp_mqtt_client_stop(client);
        esp_mqtt_client_destroy(client);
        client = nullptr;
    }
    m_HasBegun = false;
    begin();
}
