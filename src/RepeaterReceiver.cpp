//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "RepeaterReceiver.h"
#include <iostream>
#include <WiFi.h>
#include "esp_wifi.h"
#include <Preferences.h>
#include "network.h"
#include "esp_log.h"
static const char* REPEATER_RCV_TAG = "Repeater Receiver";
// using namespace std;
#define MASK_REVERSE_COLORS 0x00000001
//RepeaterReceiver &LocalRepeaterReiver = RepeaterReceiver::getInstance();

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
static long LastNumber = 0;
static long MessageCounter = 0;

struct_message m_message;
  memcpy(&m_message, incomingData, sizeof(m_message));
  RepeaterReceiver &LocalRepeaterReiver = RepeaterReceiver::getInstance();
  if(m_message.piste_ID == LocalRepeaterReiver.MasterPiste())
  {
    LocalRepeaterReiver.ResetWatchDog();
    long difference = m_message.messagenumber - LastNumber;
    if(difference > 0){
      LocalRepeaterReiver.StateChanged(m_message.event);
      MessageCounter++;
    }
    LastNumber = m_message.messagenumber;
  }
}

RepeaterReceiver::RepeaterReceiver()
{
    //ctor

}

RepeaterReceiver::~RepeaterReceiver()
{
    //dtor
}

void changeEventMainType(uint32_t *event, uint32_t newType){
  *event &=  SUB_TYPE_MASK;    // clear original type
  *event |=  newType;  // set new type
}

uint32_t swapLights(const uint32_t &event) {
  uint32_t result = EVENT_LIGHTS || MASK_REVERSE_COLORS;
  if(event & MASK_RED)
    result |= MASK_GREEN;

  if(event & MASK_GREEN)
    result |= MASK_RED;

  if(event & MASK_WHITE_L)
    result |= MASK_WHITE_R;

  if(event & MASK_WHITE_R)
        result |= MASK_WHITE_L;

  if(event & MASK_ORANGE_L)
    result |= MASK_ORANGE_R;

  if(event & MASK_ORANGE_R)
    result |= MASK_ORANGE_L;

  return result;
}
void RepeaterReceiver::StateChanged (uint32_t event)
{
// we have to modify the event in case of mirroring
if(!m_Mirror){
  notify(event);
  return;
}
uint32_t event_data = event & SUB_TYPE_MASK;
uint32_t maineventtype = event & MAIN_TYPE_MASK ;
uint32_t tempevent = event;
  switch(maineventtype)
  {
    case EVENT_LIGHTS:
    tempevent = swapLights(event);
    break;
    case EVENT_SCORE_LEFT:
    changeEventMainType(&tempevent,EVENT_SCORE_RIGHT);
    break;

    case EVENT_SCORE_RIGHT:
    changeEventMainType(&tempevent,EVENT_SCORE_LEFT);
    break;

    case EVENT_PRIO:
    if(event_data == 1)
      event_data = 2;
    else
      if(event_data == 2)
        event_data = 1;
    tempevent = event_data | EVENT_PRIO;
    break;

    case EVENT_YELLOW_CARD_LEFT:
    changeEventMainType(&tempevent,EVENT_YELLOW_CARD_RIGHT);
    break;

    case EVENT_YELLOW_CARD_RIGHT:
    changeEventMainType(&tempevent,EVENT_YELLOW_CARD_LEFT);
    break;

    case EVENT_RED_CARD_LEFT:
    changeEventMainType(&tempevent,EVENT_RED_CARD_RIGHT);
    break;

    case EVENT_RED_CARD_RIGHT:
    changeEventMainType(&tempevent,EVENT_RED_CARD_LEFT);
    break;


    case EVENT_P_CARD:
        //StateChanged(EVENT_P_CARD |  m_PCardLeft | m_PCardRight << 8);
        mix_t PCardInfo;
        PCardInfo.theDWord = event_data;
        uint8_t temp = PCardInfo.theBytes[0];
        PCardInfo.theBytes[0] = PCardInfo.theBytes[1];
        PCardInfo.theBytes[1] = temp;
        tempevent = PCardInfo.theDWord | EVENT_P_CARD;

    break;

  }
  notify(tempevent);
}


void RepeaterReceiver::RegisterRepeater(uint8_t *broadcastAddress)
{
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    ESP_LOGE(REPEATER_RCV_TAG, "%s","Failed to add peer");
    return;
  }
}
void RepeaterReceiver::begin()
{
  Preferences networkpreferences;
  networkpreferences.begin("scoringdevice", false);
  m_MasterPiste = networkpreferences.getInt("MasterPiste", -1);
  m_Mirror = networkpreferences.getBool("MirrorLights", true);

  networkpreferences.end();
    // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    ESP_LOGE(REPEATER_RCV_TAG, "%s","Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_recv_cb(OnDataRecv);


  uint8_t primary;
  wifi_second_chan_t second;
  esp_wifi_get_channel(&primary, &second);
  m_espnowchannel = primary;
  peerInfo.channel = primary;
  peerInfo.ifidx = WIFI_IF_AP;
  ESP_LOGI(REPEATER_RCV_TAG, "Wifi channel: %d",peerInfo.channel);
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
