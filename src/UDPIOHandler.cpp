//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "UDPIOHandler.h"

#include <WiFi.h>
#include "AsyncUDP.h"
#include "esp_log.h"
static const char* UDPIO_HANDLER_TAG = "UDPIOHandler";



static bool bWifiConnected = false;
static bool bUDPConnected = false;

void ProcessUDPPacket (AsyncUDPPacket packet)
{
  UDPIOHandler &MyUDPIOHandler = UDPIOHandler::getInstance();
  mix_t Event;
  Event.theBytes[0] = packet.data()[0];
  Event.theBytes[1] = packet.data()[1];
  Event.theBytes[2] = packet.data()[2];
  Event.theBytes[3] = packet.data()[3];
  MyUDPIOHandler.InputChanged(Event.theDWord);
}

void UDPIOHandler::UDPCheck()
{
static long NextTimeToCheckUDP = millis() + 2500;
UDPIOHandler &MyUDPIOHandler = UDPIOHandler::getInstance();
  /*if(millis() < NextTimeToCheckUDP)
    return;*/
  NextTimeToCheckUDP = millis() + 2500;
#ifdef HOMENETWORK
  if(WiFi.status() == WL_CONNECTED)
  {
    if(!bWifiConnected)
    {
      bWifiConnected = true;
    }

    if(!bUDPConnected)
    {// Somehow we should call this only once. It will keep on trying for ever.

      if(Commandudp.listen(1234))
      {
        ESP_LOGI(UDPIO_HANDLER_TAG, "UDP Listening on IP: %s",(WiFi.localIP().toString()).c_str()));

        Commandudp.onPacket([](AsyncUDPPacket packet) {
          ProcessUDPPacket (packet);
        });
    }
    MyUDPIOHandler.InputChanged(UI_INPUT_RESET | EVENT_UI_INPUT);
      bUDPConnected = true;
    }

  }

#else
    //if(bWifiConnected)
    {
      if(!bUDPConnected)
      {// Somehow we should call this only once. It will keep on trying for ever.

        if(Commandudp.listen(1234))
        {
          ESP_LOGI(UDPIO_HANDLER_TAG, "UDP Listening on IP: %s",(WiFi.softAPIP().toString()).c_str());


            Commandudp.onPacket([](AsyncUDPPacket packet) {
              ProcessUDPPacket (packet);
            });
        }
        MyUDPIOHandler.InputChanged(UI_INPUT_RESET | EVENT_UI_INPUT);
        MyUDPIOHandler.InputChanged(UI_INPUT_RESET | EVENT_UI_INPUT);
        bUDPConnected = true;
    }

  }
#endif
}

void UDPIOHandler::ConnectToAP()
{
  UDPIOHandler &MyUDPIOHandler = UDPIOHandler::getInstance();
  if(!bUDPConnected)
  {// We should call this only once.
    bUDPConnected = true;
    if(Commandudp.listen(1234))
    {
      ESP_LOGI(UDPIO_HANDLER_TAG, "UDP Listening on IP: %s",(WiFi.softAPIP().toString()).c_str());

        Commandudp.onPacket([](AsyncUDPPacket packet) {
          ProcessUDPPacket (packet);
        });
    }
    MyUDPIOHandler.InputChanged(UI_INPUT_RESET | EVENT_UI_INPUT);
    MyUDPIOHandler.InputChanged(UI_INPUT_RESET | EVENT_UI_INPUT);
  }
}

void UDPIOHandler::run()
{
  UDPCheck();
  //if(UDP.connected())
    //UDP.run();

};

UDPIOHandler::UDPIOHandler()
{
    //ctor

}

void UDPIOHandler::Start()
{
  /*WiFi.hostname(espHostName);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);*/
}

UDPIOHandler::~UDPIOHandler()
{
    //dtor
}

// For now, below function is not used. It could be used to send back Lights
// Information to the app
void UDPIOHandler::ProcessLightsChange(uint32_t eventtype)
{
  uint32_t event_data = eventtype & SUB_TYPE_MASK;
/*  if(event_data & MASK_RED )
    ledRed.on();
  else
    ledRed.off();
  if(event_data & MASK_GREEN )
    ledGreen.on();
  else
    ledGreen.off();
  if(event_data & MASK_WHITE_L )
    ledWhiteL.on();
  else
    ledWhiteL.off();
  if(event_data & MASK_WHITE_R )
    ledWhiteR.on();
  else
    ledWhiteR.off();
  if(event_data & MASK_ORANGE_L )
    ledOrangeL.on();
  else
    ledOrangeL.off();
  if(event_data & MASK_ORANGE_R )
    ledOrangeR.on();
  else
        ledOrangeR.off();
        */
}


void UDPIOHandler::update (FencingStateMachine *subject, uint32_t eventtype)
{
  uint32_t event_data = eventtype & SUB_TYPE_MASK;
  uint32_t maineventtype = eventtype & MAIN_TYPE_MASK ;
  char chrono[16];
  char strRound[8];
  int newseconds;
  int currentRound, nrOfRounds;

  switch(maineventtype)
  {

    case EVENT_LIGHTS:
      ProcessLightsChange(eventtype);
    break;
    case EVENT_WEAPON:

    break;

    case EVENT_SCORE_LEFT:
      //UDP.virtualWrite(V3, eventtype & DATA_24BIT_MASK);
    break;

    case EVENT_SCORE_RIGHT:
      //UDP.virtualWrite(V4, eventtype & DATA_24BIT_MASK);
    break;

    case EVENT_TIMER_STATE:
      if(eventtype & DATA_24BIT_MASK)
      {
        //UDP.setProperty(V0,"color",UDP_GREEN);
        //UDP.virtualWrite(V1,1);
      }
      else
      {
        //UDP.setProperty(V0,"color",UDP_RED);
        //UDP.virtualWrite(V1,0);
      }
    break;
    case EVENT_TIMER:
    newseconds = event_data & (DATA_BYTE1_MASK |DATA_BYTE2_MASK);

      if(previous_seconds != newseconds)
      {
        subject->GetFormattedStringTime(chrono,2,0);
        //UDP.virtualWrite(V0, chrono);
        previous_seconds = newseconds;
      }

    break;

    case EVENT_ROUND:
      currentRound = event_data & DATA_BYTE0_MASK;
      nrOfRounds = (event_data & DATA_BYTE1_MASK)>>8;
      if(!currentRound || !nrOfRounds)
      {
        sprintf(strRound," ");

      }
      else
        sprintf(strRound,"%d/%d",currentRound,nrOfRounds);

      //UDP.virtualWrite(V11, strRound);

    break;



  }

}
