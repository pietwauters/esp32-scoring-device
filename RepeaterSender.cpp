//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "RepeaterSender.h"
#include "network.h"
#include <iostream>
#include <Preferences.h>
#include "esp_err.h"

using namespace std;

RepeaterSender::RepeaterSender()
{
    //ctor

}

RepeaterSender::~RepeaterSender()
{
    //dtor
}

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //char macStr[18];
  //Serial.print("Packet to: ");
  // Copies the sender mac address to a string
  /*snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
  Serial.print(" send status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  */
}

void RepeaterSender::begin()
{
    // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  uint8_t primary;
  wifi_second_chan_t second;
  esp_wifi_get_channel(&primary, &second);
  peerInfo.channel = primary;
  peerInfo.ifidx = ESP_IF_WIFI_AP;
  Serial.print("Wifi channel");Serial.println(peerInfo.channel);
  peerInfo.encrypt = false;
  Preferences networkpreferences;
  networkpreferences.begin("credentials", false);
  m_message.piste_ID = networkpreferences.getInt("pisteNr", 500);
  networkpreferences.end();

  RegisterRepeater(m_broadcastAddress);
  RegisterRepeater(m_receiverAddress);
  }

void RepeaterSender::RegisterRepeater(uint8_t *broadcastAddress)
{
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}
void RepeaterSender::BroadcastHeartBeat()
{
  if(millis() < TimeToNextHeartbeat)
    return;
  TimeToNextHeartbeat = millis() + HeartPeriod;
  m_message.type = HEARTBEAT;
  m_message.event = m_HeartbeatCounter++;
  esp_err_t result = esp_now_send(m_broadcastAddress, (uint8_t *) &m_message, sizeof(m_message));
}

void RepeaterSender::update (FencingStateMachine *subject, uint32_t eventtype)
{
  // Send message via ESP-NOW
  m_message.event = eventtype;
  m_message.type = EVENT;
  //esp_err_t result = esp_now_send(m_receiverAddress, (uint8_t *) &m_message, sizeof(m_message));
  esp_err_t result = esp_now_send(m_broadcastAddress, (uint8_t *) &m_message, sizeof(m_message));

  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println(esp_err_to_name(result));
  }
  Serial.println(m_message.piste_ID);
  
}
