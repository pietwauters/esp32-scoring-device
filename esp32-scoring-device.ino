//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>

/*! \mainpage ESP32 Scoring Machine for fencing
 *
 * \section intro_sec Introduction
 *
 * For more information about this project pls. go to https://github.com/pietwauters/esp32-scoring-device/wiki
 *
 * \section main_structure_sec Main structure
 * The esp32 has 2 cores. One core is used for the physical scoring device sensor.
 * The other core is used for everything else: State machine, timers, network, Cyrano, Configuration Portal, ...
 *
 * \subsection The sensor
 *
 *
 */
//#include "LedMatrix.h"
#include "3WeaponSensor.h"
#include "WS2812BLedStrip.h"
#include "FencingStateMachine.h"
#include "FPA422Handler.h"
#include "RepeaterReceiver.h"
#include "RepeaterSender.h"
#include "UDPIOHandler.h"
#include "TimeScoreDisplay.h"
#include "CyranoHandler.h"
#include "driver/adc.h"
#include "esp_task_wdt.h"
#include "network.h"
#include "ResetHandler.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"


WS2812B_LedStrip *MyLedStrip;
TimeScoreDisplay *MyTimeScoreDisplay;
MultiWeaponSensor *MySensor;
NetWork *MyNetWork;
FencingStateMachine *MyStatemachine;
FPA422Handler *MyFPA422Handler;
UDPIOHandler *MyUDPIOHandler;
CyranoHandler *MyCyranoHandler;
RepeaterReceiver *MyRepeaterReiver;
RepeaterSender *MyRepeaterSender;

bool bIsRepeater = false;
bool bEnableDeepSleep = false;
int FactoryResetCounter = 50;

const int PowerPin = 12;
void setup() {
  //cout << "Time to get here: " << millis() << endl;
  //uint32_t brown_reg_temp = READ_PERI_REG(RTC_CNTL_BROWN_OUT_REG); //save WatchDog register
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, brown_reg_temp); //enable brownout detector

  //Switch the power of all LED modules on
  pinMode(PowerPin, OUTPUT);
  digitalWrite(PowerPin,HIGH);

  MyLedStrip = &WS2812B_LedStrip::getInstance();
  MyLedStrip->begin();
  MyTimeScoreDisplay = new TimeScoreDisplay();
  MyLedStrip->ClearAll();
  MySensor = &MultiWeaponSensor::getInstance();
  MyLedStrip->ClearAll();
  MyStatemachine= &FencingStateMachine::getInstance();
  // put your setup code here, to run once:
  Serial.begin(115200);
  MyLedStrip->ClearAll();
  MyTimeScoreDisplay->begin();
  MyLedStrip->ClearAll();
  MyTimeScoreDisplay->DisplayPisteId();
  MyFPA422Handler = new FPA422Handler();
  Preferences mypreferences;
  mypreferences.begin("scoringdevice", RO_MODE);
  bIsRepeater = mypreferences.getBool("RepeaterMode",false);
  bEnableDeepSleep = mypreferences.getBool("Powersave",false);
  mypreferences.end();
  MyNetWork = &NetWork::getInstance();
  MyNetWork->begin();
  update_reset_reasons();
  print_historical_reset_reason();
  MyNetWork->GlobalStartWiFi();
  Serial.println("Wifi started");
  Serial.println("by now the you should have seen all the lights one by one");
  MyUDPIOHandler = &UDPIOHandler::getInstance();
  MyUDPIOHandler->ConnectToAP();
  MyUDPIOHandler->attach(*MyNetWork);
  esp_task_wdt_init(20, false);
// In repeater mode don't start these 2 tasks
if(!bIsRepeater){
Serial.println("Bwahahaaha I am the master!");
  MyCyranoHandler = &CyranoHandler::getInstance();
  MyStatemachine->ResetAll();
  MyFPA422Handler->StartWiFi();
  MyStatemachine->attach(*MyFPA422Handler);
  MyStatemachine->attach(*MyTimeScoreDisplay);
  MyUDPIOHandler->attach(*MyStatemachine);
  MyStatemachine->attach(*MyUDPIOHandler);
  MyStatemachine->attach(*MyCyranoHandler);
  MyUDPIOHandler->attach(*MyCyranoHandler);
  MyCyranoHandler->attach(*MyStatemachine);
  MyCyranoHandler->attach(*MyFPA422Handler);
  MySensor->attach(*MyStatemachine);
  MyStatemachine->RegisterMultiWeaponSensor(MySensor);
  MyStatemachine->begin();
  MySensor->begin();
  MyCyranoHandler->Begin();
  MyRepeaterSender  = &RepeaterSender::getInstance();
  MyRepeaterSender->begin();
  MyStatemachine->attach(*MyRepeaterSender);
  MyStatemachine->attach(*MyLedStrip);
  MyStatemachine->SetMachineWeapon(MySensor->GetActualWeapon());
  switch (MySensor->GetActualWeapon()) {
    case FOIL:
    MyStatemachine->StateChanged(EVENT_WEAPON | WEAPON_MASK_FOIL);
    break;
    case EPEE:
    MyStatemachine->StateChanged(EVENT_WEAPON | WEAPON_MASK_EPEE);
    break;
    case SABRE:
    MyStatemachine->StateChanged(EVENT_WEAPON | WEAPON_MASK_SABRE);
    break;
  }
}
else{
  // When running in repeater mode
  MyRepeaterReiver = &RepeaterReceiver::getInstance();
  Serial.println("Ouch! I am a repeater!");
  MyRepeaterReiver->begin();
  MyRepeaterReiver->attach(*MyLedStrip);
  MyRepeaterReiver->attach(*MyTimeScoreDisplay);
  MyRepeaterReiver->StartWatchDog();
  MyLedStrip->SetMirroring(MyRepeaterReiver->Mirror());
}
Serial.println(WiFi.localIP());
Serial.println("MAC address: ");
  Serial.println(WiFi.macAddress());
// Add GPIO0 code here


}



void loop() {
  // put your main code here, to run repeatedly:
  delay(1); // without this it simply doesn't work
  //vTaskDelay(1 / portTICK_PERIOD_MS);
  if(!bIsRepeater){
    MyFPA422Handler->WifiPeriodicalUpdate();
    esp_task_wdt_reset();
    vTaskDelay(1 / portTICK_PERIOD_MS);
    MyTimeScoreDisplay->ProcessEvents();
    esp_task_wdt_reset();
    vTaskDelay(1 / portTICK_PERIOD_MS);
    MyFPA422Handler->WifiPeriodicalUpdate();
    esp_task_wdt_reset();
    vTaskDelay(1 / portTICK_PERIOD_MS);
    if(MyNetWork->IsExternalWifiAvailable())
    {
      //MyCyranoHandler->PeriodicallyBroadcastStatus();
      esp_task_wdt_reset();
      vTaskDelay(1 / portTICK_PERIOD_MS);
      MyCyranoHandler->CheckConnection();
      //MyFPA422Handler->WifiPeriodicalUpdate();  // Not really needed because already done above
    }
    esp_task_wdt_reset();
    vTaskDelay(1 / portTICK_PERIOD_MS);

    if(MyStatemachine->IsConnectedToRemote())
    {
      MyTimeScoreDisplay->CycleScoreMatchAndTimeWhenNotFighting();
      esp_task_wdt_reset();
      vTaskDelay(1 / portTICK_PERIOD_MS);
      MyFPA422Handler->WifiPeriodicalUpdate();
      MyFPA422Handler->WifiPeriodicalUpdate();
    }

    MyStatemachine->PeriodicallyBroadcastFullState(MyRepeaterSender,FULL_STATUS_REPETITION_PERIOD);
    esp_task_wdt_reset();
    vTaskDelay(1 / portTICK_PERIOD_MS);
    MyRepeaterSender->RepeatLastMessage();
    esp_task_wdt_reset();
    vTaskDelay(1 / portTICK_PERIOD_MS);
    //MyRepeaterSender->BroadcastHeartBeat();

    if(bEnableDeepSleep && MyStatemachine->GoToSleep())
    {
      prepareforDeepSleep();
    }
  }
  else{ // when in repeater mode
    MyTimeScoreDisplay->ProcessEvents();
    MyTimeScoreDisplay->CycleScoreMatchAndTimeWhenNotFighting();
    esp_task_wdt_reset();
    vTaskDelay(1 / portTICK_PERIOD_MS);

    if(MyRepeaterReiver->IsWatchDogTriggered())
    { // We lost connection with the master scoring device
      // clear displays and start looking for MasterId
      MyTimeScoreDisplay->DisplayPisteId();
      MyLedStrip->ClearAll();
      MyNetWork->FindAndSetMasterChannel(1,false);
    }
  }
  esp_task_wdt_reset();
  if(!digitalRead(0)){
    if(!FactoryResetCounter){
      cout << "Bootpin pressed" << endl;
      MyNetWork->DoFactoryReset();
      ESP.restart();
      FactoryResetCounter = 200;
    }
    else
      FactoryResetCounter--;
  }
  else {
    FactoryResetCounter = 200;
  }

}
