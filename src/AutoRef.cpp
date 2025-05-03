//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "AutoRef.h"
#include <iostream>
#include <Preferences.h>
#include "esp_task_wdt.h"
TaskHandle_t AutoRefTask;

void AutoRef::AutoRefHandler(void *parameter){
  uint32_t LastEvent;
  AutoRef &MyLocalAutoRef = AutoRef::getInstance();
  while(true)
  {
    if(xQueueReceive(MyLocalAutoRef.AutoRefqueue, &LastEvent, 4 / portTICK_PERIOD_MS)== pdPASS)
    {
      uint32_t event_type = LastEvent & MAIN_TYPE_MASK;
      uint32_t event_data = LastEvent & DATA_24BIT_MASK;

      switch(event_type){
        case EVENT_TIMER_STATE:
        if(LastEvent && 0x00000001){
          // Timer started

        }
        else {
          // Timer Stopped
          // Determine Time until AnimateEngardePretsAllez
        }

        break;

        case EVENT_ROUND:
        //m_round =  event_data & DATA_BYTE0_MASK;
        //m_maxround = (event_data & DATA_BYTE1_MASK) >>8;
        break;

      }
    }

    esp_task_wdt_reset();
  }
}

AutoRef::AutoRef()
{
    //ctor
    AutoRefqueue = xQueueCreate( 30, sizeof( int ) );
}

AutoRef::~AutoRef()
{
    //dtor
}

void AutoRef::begin()
{
  if(m_HasBegun)
    return;
    xTaskCreatePinnedToCore(
              AutoRefHandler,        /* Task function. */
              "AutoRefHandler",      /* String with name of task. */
              16384,                            /* Stack size in words. */
              NULL,                            /* Parameter passed as input of the task */
              0,                                /* Priority of the task. */
              &AutoRefTask,           /* Task handle. */
              1);
    m_HasBegun = true;
}
