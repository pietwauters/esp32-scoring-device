//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "RepeaterReceiver.h"
#include <iostream>
#include <WiFi.h>
#include "esp_wifi.h"
#include <Preferences.h>
#include "network.h"
using namespace std;

RepeaterReceiver::RepeaterReceiver()
{
    //ctor

}

RepeaterReceiver::~RepeaterReceiver()
{
    //dtor
}



void RepeaterReceiver::RegisterRepeater(uint8_t *broadcastAddress)
{
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}
void RepeaterReceiver::begin(esp_now_recv_cb_t theCallBack)
{
  Preferences networkpreferences;
  networkpreferences.begin("scoringdevice", false);
  m_MasterPiste = networkpreferences.getInt("MasterPiste", -1);

  networkpreferences.end();
    // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_recv_cb(theCallBack);

  uint8_t primary;
  wifi_second_chan_t second;
  esp_wifi_get_channel(&primary, &second);
  m_espnowchannel = primary;
  peerInfo.channel = primary;
  peerInfo.ifidx = ESP_IF_WIFI_AP;
  Serial.print("Wifi channel");Serial.println(peerInfo.channel);
  peerInfo.encrypt = false;
  networkpreferences;

  RegisterRepeater(m_broadcastAddress);
  RegisterRepeater(m_senderAddress);
  }

  void RepeaterReceiver::StartWatchDog(long Period){
    m_WatchDogPeriod = Period;
    m_WatchDogTriggerTime = millis() + m_WatchDogPeriod;
  }

  void RepeaterReceiver::ResetWatchDog(){
    m_WatchDogTriggerTime = millis() + m_WatchDogPeriod;
  }
  
  bool RepeaterReceiver::IsWatchDogTriggered(){
    if(millis() > m_WatchDogTriggerTime)
    {
      return true;
    }
    return false;
  }
