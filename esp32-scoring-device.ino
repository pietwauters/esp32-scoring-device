//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
//#include "LedMatrix.h"
#include "3WeaponSensor.h"
#include "WS2812BLedStrip.h"
#include "FencingStateMachine.h"
#include "FPA422Handler.h"
#include "UDPIOHandler.h"
#include "TimeScoreDisplay.h"
#include "CyranoHandler.h"
#include "driver/adc.h"
#include "esp_task_wdt.h"
#include "network.h"


#define WELCOME_ANIMATION_SPEED 70

MultiWeaponSensor MySensor(1);
NetWork MyNetWork;
WS2812B_LedStrip MyLedStrip;
FencingStateMachine MyStatemachine(2,10);
FPA422Handler MyFPA422Handler;
UDPIOHandler MyUDPIOHandler;
TimeScoreDisplay MyTimeScoreDisplay;
CyranoHandler MyCyranoHandler;

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

TaskHandle_t LedStripTask;
void LedStripHandler(void *parameter)
{
  while(true)
  {
    MyLedStrip.ProcessEventsBlocking();
    esp_task_wdt_reset();
  }
}

void PrintReasonForReset()
{
  Serial.print("reason for reset: ");
  esp_reset_reason_t ResetReason = esp_reset_reason();
  switch(ResetReason)
  {
    case ESP_RST_UNKNOWN:
    Serial.println("ESP_RST_UNKNOWN");
    break;

    case ESP_RST_POWERON:
    Serial.println("ESP_RST_POWERON");
    break;

    case ESP_RST_EXT:
    Serial.println("ESP_RST_EXT");
    MyLedStrip.ClearAll();
    MyLedStrip.setWhiteLeft(true);
    MyLedStrip.setWhiteRight(true);
    MyLedStrip.myShow();
    delay(5000);
    break;

    case ESP_RST_SW:
    Serial.println("ESP_RST_SW");
    MyLedStrip.ClearAll();
    MyLedStrip.setWhiteLeft(true);
    MyLedStrip.setWhiteRight(true);
    MyLedStrip.myShow();
    delay(5000);
    break;

    case ESP_RST_PANIC:
    Serial.println("ESP_RST_PANIC");
    MyLedStrip.ClearAll();
    MyLedStrip.setRed(true);
    MyLedStrip.myShow();
    delay(5000);
    break;

    case ESP_RST_INT_WDT:
    Serial.println("ESP_RST_INT_WDT");
    MyLedStrip.ClearAll();
    MyLedStrip.setRed(true);
    MyLedStrip.setGreen(true);
    MyLedStrip.myShow();
    break;

    case ESP_RST_DEEPSLEEP:
    Serial.println("ESP_RST_DEEPSLEEP");
    delay(5000);
    break;

    case ESP_RST_BROWNOUT:
    Serial.println("ESP_RST_BROWNOUT");
    MyLedStrip.ClearAll();
    MyLedStrip.setGreen(true);
    MyLedStrip.myShow();
    delay(5000);
    break;

    case ESP_RST_SDIO:
    Serial.println("ESP_RST_SDIO");
    break;

    case ESP_RST_TASK_WDT:
    Serial.println("ESP_RST_TASK_WDT");
    MyLedStrip.ClearAll();
    MyLedStrip.setWhiteLeft(true);
    MyLedStrip.myShow();
    delay(5000);
    break;

    case ESP_RST_WDT:
    Serial.println("ESP_RST_WDT");
    MyLedStrip.ClearAll();
    MyLedStrip.setWhiteRight(true);
    MyLedStrip.myShow();
    delay(5000);
    break;

  }
}

void ShowWelcomeLights()
{
  for(int i = 0; i< 50; i++)
  {
    MyLedStrip.ClearAll();
    delay(10);
  }

  MyLedStrip.setWhiteLeft(true);
  MyLedStrip.myShow();
  esp_task_wdt_reset();
  delay(5 * WELCOME_ANIMATION_SPEED);// Block for 500ms.
  MyLedStrip.setWhiteLeft(false);
  MyLedStrip.myShow();
  MyLedStrip.setRed(true);
  MyLedStrip.myShow();
  esp_task_wdt_reset();
  delay(5 * WELCOME_ANIMATION_SPEED);// Block for 500ms.
  MyLedStrip.setRed(false);
  MyLedStrip.myShow();
  MyLedStrip.setWhiteRight(true);
  MyLedStrip.myShow();
  esp_task_wdt_reset();
  delay(5 * WELCOME_ANIMATION_SPEED);
  MyLedStrip.setWhiteRight(false);
  MyLedStrip.myShow();
  MyLedStrip.setGreen(true);
  MyLedStrip.myShow();
  esp_task_wdt_reset();
  delay(5 * WELCOME_ANIMATION_SPEED);
  MyLedStrip.ClearAll();
  MyLedStrip.setUWFTimeLeft(1);
  MyLedStrip.myShow();
  delay(2 * WELCOME_ANIMATION_SPEED);
  MyLedStrip.setUWFTimeLeft(2);
  MyLedStrip.myShow();
  delay(2 * WELCOME_ANIMATION_SPEED);
  MyLedStrip.setUWFTimeLeft(3);
  MyLedStrip.myShow();
  delay(2 * WELCOME_ANIMATION_SPEED);
  MyLedStrip.setUWFTimeLeft(4);
  MyLedStrip.myShow();
  delay(2 * WELCOME_ANIMATION_SPEED);
  MyLedStrip.setUWFTimeLeft(5);
  MyLedStrip.myShow();
  delay(2 * WELCOME_ANIMATION_SPEED);
  MyLedStrip.setUWFTimeLeft(6);
  MyLedStrip.myShow();
  delay(4 * WELCOME_ANIMATION_SPEED);
  MyLedStrip.setUWFTimeLeft(0);
  MyLedStrip.myShow();
}

void setup() {


  // put your setup code here, to run once:
  Serial.begin(115200);

  MyTimeScoreDisplay.begin();
  MyTimeScoreDisplay.DisplayWeapon(EPEE);
  //PrintReasonForReset();    // This is only for debugging instabilities. Comment out when you think it works
  ShowWelcomeLights();
  MyNetWork.GlobalStartWiFi();
  MyFPA422Handler.StartWiFi();
  Serial.println("Wifi started");
  Serial.println("by now the you should have seen all the lights one by one");

  //MyFPA422Handler.StartBluetooth();
  //Serial.println("Bluetooth started");
  MyUDPIOHandler.ConnectToAP();
  MyStatemachine.attach(MyLedStrip);
  MyStatemachine.attach(MyFPA422Handler);
  MyStatemachine.attach(MyTimeScoreDisplay);
  MyUDPIOHandler.attach(MyStatemachine);
  MyStatemachine.attach(MyUDPIOHandler);
  MyStatemachine.attach(MyCyranoHandler);
  MyUDPIOHandler.attach(MyCyranoHandler);
  MyCyranoHandler.attach(MyStatemachine);
  MyCyranoHandler.attach(MyFPA422Handler);


  esp_task_wdt_init(10, false);

  xTaskCreatePinnedToCore(
            CoreScoringMachineHandler,        /* Task function. */
            "CoreScoringMachineHandler",      /* String with name of task. */
            10240,                            /* Stack size in words. */
            NULL,                             /* Parameter passed as input of the task */
            0,                                /* Priority of the task. */
            &CoreScoringMachineTask,           /* Task handle. */
            0);
  esp_task_wdt_add(CoreScoringMachineTask);
  MySensor.attach(MyStatemachine);
  MyStatemachine.RegisterMultiWeaponSensor(&MySensor);

  xTaskCreatePinnedToCore(
            StateMachineHandler,        /* Task function. */
            "StateMachineHandlerMyLedStrip",      /* String with name of task. */
            32768,                            /* Stack size in words. */
            NULL,                             /* Parameter passed as input of the task */
            0,                                /* Priority of the task. */
            &StateMachineTask,           /* Task handle. */
            1);
  esp_task_wdt_add(StateMachineTask);

  xTaskCreatePinnedToCore(
            LedStripHandler,        /* Task function. */
            "LedStripHandler",      /* String with name of task. */
            10240,                            /* Stack size in words. */
            NULL,                            /* Parameter passed as input of the task */
            0,                                /* Priority of the task. */
            &LedStripTask,           /* Task handle. */
            1);
  esp_task_wdt_add(LedStripTask);
  delay(100);
  MyStatemachine.ResetAll();
  MyStatemachine.StateChanged(EVENT_WEAPON | WEAPON_MASK_EPEE);
  StopSearchingForWifi = millis() + 30000;

}



void loop() {
  // put your main code here, to run repeatedly:

  delay(1); // without this it simply doesn't work
  MyFPA422Handler.WifiPeriodicalUpdate();
  delay(1);
  MyTimeScoreDisplay.ProcessEvents();
  delay(1);
  // If there is no WiFi within 30 seconds after start, it will not come
  if(millis() < StopSearchingForWifi)
    MyCyranoHandler.CheckConnection();
  if(MyStatemachine.IsConnectedToRemote())
  {
    MyTimeScoreDisplay.CycleScoreMatchAndTimeWhenNotFighting();
    delay(1);
    MyLedStrip.AnimatePrio();
  }
  esp_task_wdt_reset();

}
