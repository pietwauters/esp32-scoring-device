//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "RepeaterSender.h"
#include "network.h"
#include <iostream>
#include <Preferences.h>
#include "esp_err.h"
#include "esp_log.h"
static const char* REPEATER_SND_TAG = "Repeater Sender";

// using namespace std;

RepeaterSender::RepeaterSender()
{
    //ctor

}

RepeaterSender::~RepeaterSender()
{
    //dtor
}



// callback when data is sent
void OnDataSentMaster(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //char macStr[18];
  //Serial.print("Packet to: ");
  // Copies the sender mac address to a string
  /*snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
  Serial.print(" send status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  */
  //if(status != ESP_NOW_SEND_SUCCESS)

}

void OnDataRecvMaster(const unsigned char * mac_addr, const uint8_t *incomingData, int len) {
/*    memcpy(&m_message, incomingData, sizeof(m_message));

    if(m_message.piste_ID == m_MasterPiste)
    {
      if(m_message.type == PAIRING_REQUEST){

      }
    }
    */
}

void RepeaterSender::begin()
{
    // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    ESP_LOGE(REPEATER_SND_TAG, "%s","Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSentMaster);

  uint8_t primary;
  wifi_second_chan_t second;
  esp_wifi_get_channel(&primary, &second);
  peerInfo.channel = primary;
  peerInfo.ifidx = WIFI_IF_AP;
  ESP_LOGI(REPEATER_SND_TAG, "Wifi channel: %d",peerInfo.channel);

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
    ESP_LOGE(REPEATER_SND_TAG, "%s","Failed to add peer");
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
  m_message.messagenumber = currentNewMessageCount++;
  m_message.event = eventtype;
  m_message.type = EVENT;
  //esp_err_t result = esp_now_send(m_receiverAddress, (uint8_t *) &m_message, sizeof(m_message));
  esp_err_t result = esp_now_send(m_broadcastAddress, (uint8_t *) &m_message, sizeof(m_message));
  m_nextResendTime = 0;
  if(eventtype && MAIN_TYPE_MASK == EVENT_TIMER){
    if(eventtype && DATA_BYTE0_MASK)  // don't resend hundreths
      m_resendCount = 0;
      return;
  }
  if(eventtype && MAIN_TYPE_MASK == EVENT_LIGHTS)
    m_resendCount = 7;
  else
    m_resendCount = MESSAGE_REPETITION_FACTOR;
}

void RepeaterSender::RepeatLastMessage(){
  if(m_resendCount){
    if(millis() > m_nextResendTime){
      esp_err_t result = esp_now_send(m_broadcastAddress, (uint8_t *) &m_message, sizeof(m_message));
      m_resendCount--;
      m_nextResendTime = millis() + m_ResendDelta[m_resendCount];
    }
  }
}
