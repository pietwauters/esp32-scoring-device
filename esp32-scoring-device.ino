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




//WS2812B_LedStrip &MyLedStrip = WS2812B_LedStrip::getInstance();
TimeScoreDisplay MyTimeScoreDisplay;
MultiWeaponSensor MySensor(1);
NetWork MyNetWork;
FencingStateMachine MyStatemachine(2,10);
FPA422Handler MyFPA422Handler;
UDPIOHandler MyUDPIOHandler;
CyranoHandler MyCyranoHandler;
RepeaterReceiver MyRepeaterReiver;
RepeaterSender MyRepeaterSender;

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
static long LastNumber = 0;
static long lossCounter = 0;
static long MessageCounter = 0;

struct_message m_message;
  memcpy(&m_message, incomingData, sizeof(m_message));

  if(m_message.piste_ID == MyRepeaterReiver.MasterPiste())
  {
    MyRepeaterReiver.ResetWatchDog();
    long difference = m_message.messagenumber - LastNumber;
    if(difference > 0){
      MyRepeaterReiver.StateChanged(m_message.event);
      MessageCounter++;
      if(difference > 1){
        lossCounter++;
        Serial.print("Total Message count = ");Serial.println(MessageCounter);
        Serial.print("Message lost; Total loss Count = ");
        Serial.println(lossCounter);
        Serial.print("Errorrate = ");
        Serial.println((float)lossCounter*100/(float)MessageCounter);
      }
    }
    LastNumber = m_message.messagenumber;
  }
}

TaskHandle_t CoreScoringMachineTask;
long StopSearchingForWifi = 60000;
void CoreScoringMachineHandler(void *parameter)
{
  while(true)
  {
    MySensor.DoFullScan();
    esp_task_wdt_reset();
  }
}

TaskHandle_t StateMachineTask;
void StateMachineHandler(void *parameter)
{
  while(true)
  {
    MyStatemachine.DoStateMachineTick();
    esp_task_wdt_reset();

  }
}





void GoBackToSleepIfWokenByTimer(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  if( ESP_SLEEP_WAKEUP_TIMER == wakeup_reason){
    delay(300);
    esp_deep_sleep_start();
  }
}

bool bIsRepeater = false;
void setup() {

  MyLedStrip.begin();

  // put your setup code here, to run once:
  Serial.begin(115200);
  MyLedStrip.ClearAll();
  //GoBackToSleepIfWokenByTimer();
  MyTimeScoreDisplay.begin();
  MyLedStrip.ClearAll();
  MyTimeScoreDisplay.DisplayPisteId();

  Preferences mypreferences;
  mypreferences.begin("scoringdevice", RO_MODE);
  bIsRepeater = mypreferences.getBool("RepeaterMode",false);
  mypreferences.end();

  MyNetWork.begin();

  MyNetWork.GlobalStartWiFi();
  Serial.println("Wifi started");
  Serial.println("by now the you should have seen all the lights one by one");

  MyUDPIOHandler.ConnectToAP();
  MyUDPIOHandler.attach(MyNetWork);
  esp_task_wdt_init(10, false);
// In repeater mode don't start these 2 tasks
if(!bIsRepeater){
Serial.println("Bwahahaaha I am the master!");
  MyStatemachine.ResetAll();

  MyFPA422Handler.StartWiFi();

  MyStatemachine.attach(MyFPA422Handler);
  MyStatemachine.attach(MyTimeScoreDisplay);
  MyUDPIOHandler.attach(MyStatemachine);
  MyStatemachine.attach(MyUDPIOHandler);
  MyStatemachine.attach(MyCyranoHandler);
  MyUDPIOHandler.attach(MyCyranoHandler);
  MyCyranoHandler.attach(MyStatemachine);
  MyCyranoHandler.attach(MyFPA422Handler);
  xTaskCreatePinnedToCore(
            CoreScoringMachineHandler,        /* Task function. */
            "CoreScoringMachineHandler",      /* String with name of task. */
            24576,                            /* Stack size in words. */
            NULL,                             /* Parameter passed as input of the task */
            0,                                /* Priority of the task. */
            &CoreScoringMachineTask,           /* Task handle. */
            0);
  esp_task_wdt_add(CoreScoringMachineTask);
  MySensor.attach(MyStatemachine);
  MyStatemachine.RegisterMultiWeaponSensor(&MySensor);

  xTaskCreatePinnedToCore(
            StateMachineHandler,        /* Task function. */
            "StateMachineHandler",      /* String with name of task. */
            24576,                            /* Stack size in words. 65535*/
            NULL,                             /* Parameter passed as input of the task */
            5,                                /* Priority of the task. */
            &StateMachineTask,           /* Task handle. */
            1);
  esp_task_wdt_add(StateMachineTask);


  MySensor.begin();
  StopSearchingForWifi = millis() + 60000;
  MyCyranoHandler.Begin();
  MyRepeaterSender.begin();
  MyStatemachine.attach(MyRepeaterSender);
  MyStatemachine.attach(MyLedStrip);
  MyStatemachine.SetMachineWeapon(MySensor.GetActualWeapon());
  switch (MySensor.GetActualWeapon()) {
    case FOIL:
    MyStatemachine.StateChanged(EVENT_WEAPON | WEAPON_MASK_FOIL);
    break;
    case EPEE:
    MyStatemachine.StateChanged(EVENT_WEAPON | WEAPON_MASK_EPEE);
    break;
    case SABRE:
    MyStatemachine.StateChanged(EVENT_WEAPON | WEAPON_MASK_SABRE);
    break;
  }
}
else{
  // When running in repeater mode
  Serial.println("Ouch! I am a repeater!");
  MyRepeaterReiver.begin(OnDataRecv);
  MyRepeaterReiver.attach(MyLedStrip);
  MyRepeaterReiver.attach(MyTimeScoreDisplay);
  MyRepeaterReiver.StartWatchDog();
  MyLedStrip.SetMirroring(MyRepeaterReiver.Mirror());
}
}



void loop() {
  // put your main code here, to run repeatedly:
  delay(1); // without this it simply doesn't work
  //vTaskDelay(1 / portTICK_PERIOD_MS);
  if(!bIsRepeater){
    MyFPA422Handler.WifiPeriodicalUpdate();
    esp_task_wdt_reset();
    vTaskDelay(1 / portTICK_PERIOD_MS);
    MyTimeScoreDisplay.ProcessEvents();
    esp_task_wdt_reset();
    vTaskDelay(1 / portTICK_PERIOD_MS);
    MyFPA422Handler.WifiPeriodicalUpdate();
    esp_task_wdt_reset();
    vTaskDelay(1 / portTICK_PERIOD_MS);
    if(MyNetWork.IsExternalWifiAvailable())
    {
      MyCyranoHandler.PeriodicallyBroadcastStatus();
      esp_task_wdt_reset();
      vTaskDelay(1 / portTICK_PERIOD_MS);
      MyCyranoHandler.CheckConnection();
      MyFPA422Handler.WifiPeriodicalUpdate();  // Not really needed because already done above
    }
    esp_task_wdt_reset();
    vTaskDelay(1 / portTICK_PERIOD_MS);

    if(MyStatemachine.IsConnectedToRemote())
    {
      MyTimeScoreDisplay.CycleScoreMatchAndTimeWhenNotFighting();
      esp_task_wdt_reset();
      vTaskDelay(1 / portTICK_PERIOD_MS);
      MyFPA422Handler.WifiPeriodicalUpdate();
      MyLedStrip.AnimatePrio();
      MyFPA422Handler.WifiPeriodicalUpdate();

    }
    MyLedStrip.AnimateWarning();
    MyStatemachine.PeriodicallyBroadcastFullState(&MyRepeaterSender,FULL_STATUS_REPETITION_PERIOD);
    esp_task_wdt_reset();
    vTaskDelay(1 / portTICK_PERIOD_MS);
    MyRepeaterSender.RepeatLastMessage();
    esp_task_wdt_reset();
    vTaskDelay(1 / portTICK_PERIOD_MS);
    //MyRepeaterSender.BroadcastHeartBeat();
    if(MyStatemachine.GoToSleep())
    {
      vTaskSuspend(CoreScoringMachineTask);
      delay(100);
      prepareforDeepSleep();
    }
  }
  else{ // when in repeater mode
    MyTimeScoreDisplay.ProcessEvents();
    MyTimeScoreDisplay.CycleScoreMatchAndTimeWhenNotFighting();
    esp_task_wdt_reset();
    vTaskDelay(1 / portTICK_PERIOD_MS);
    MyLedStrip.AnimatePrio();
    MyLedStrip.AnimateWarning();
    if(MyRepeaterReiver.IsWatchDogTriggered())
    { // We lost connection with the master scoring device
      // clear displays and start looking for MasterId
      MyTimeScoreDisplay.DisplayPisteId();
      MyLedStrip.ClearAll();
      MyNetWork.FindAndSetMasterChannel(1,false);

    }
  }
  esp_task_wdt_reset();
  //MyRepeaterReiver.StateChanged(fakelights);


}
