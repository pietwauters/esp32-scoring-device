//Copyright (c) Piet Wauters 2025 <piet.wauters@gmail.com>
#include "ResetHandler.h"
#include <rom/rtc.h>
#include <iostream>
#include <string>
#include "esp_log.h"
static const char* RESET_HANDLER_TAG = "ResetHandler";
// using namespace std;

char *strReasons[]={"","POWERON_RESET","","SW_RESET","OWDT_RESET","DEEPSLEEP_RESET","SDIO_RESET","TG0WDT_SYS_RESET","TG1WDT_SYS_RESET","RTCWDT_SYS_RESET","INTRUSION_RESET","TGWDT_CPU_RESET",
"SW_CPU_RESET","","RTCWDT_CPU_RESET","EXT_CPU_RESET","RTCWDT_BROWN_OUT_RESET","RTCWDT_RTC_RESET","NO_MEAN"};

char * strreset_reason(RESET_REASON reason)
{
  if((reason <0) || (reason >16))
    return ("NO_MEAN");
  return(strReasons[reason]);
}

void print_historical_reset_reason()
{
  ESP_LOGI(RESET_HANDLER_TAG,"%s","Historical reasons for reset:");
  Preferences mypreferences;
  mypreferences.begin("resetcauses0", RO_MODE);
  ESP_LOGI(RESET_HANDLER_TAG,"%s","Core 0: ");
  int tempvalue = 0;
  for(int i = 1; i<17; i++){
    tempvalue = mypreferences.getInt(strReasons[i],0);
    if(tempvalue){
      ESP_LOGI(RESET_HANDLER_TAG,"\n%s: %d",strReasons[i],tempvalue);
    }
  }
  mypreferences.end();

  mypreferences.begin("resetcauses1", RO_MODE);
  ESP_LOGI(RESET_HANDLER_TAG,"%s","Core 1: ");
  for(int i = 1; i<17; i++){
    tempvalue = mypreferences.getInt(strReasons[i],0);
    if(tempvalue){
      ESP_LOGI(RESET_HANDLER_TAG,"\n%s: %d",strReasons[i],tempvalue);
    }
  }
  mypreferences.end();
}

void StoreInPreferences(RESET_REASON theReason, Preferences mypreferences){
  int currentValue = 0;
    currentValue = mypreferences.getInt(strreset_reason(theReason),0);
    mypreferences.putInt(strreset_reason(theReason),++currentValue);

}

void update_reset_reasons(){
  Preferences mypreferences;
  RESET_REASON theReason = rtc_get_reset_reason(0);
  mypreferences.begin("resetcauses0", RW_MODE);
  StoreInPreferences(theReason, mypreferences);
  mypreferences.end();

  theReason = rtc_get_reset_reason(1);
  mypreferences.begin("resetcauses1", RW_MODE);
  StoreInPreferences(theReason, mypreferences);
  mypreferences.end();
}
