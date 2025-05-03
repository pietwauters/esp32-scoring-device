//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "weaponenum.h"
#include "RS422_FPA_Type2_Message.h"
#include "RS422_FPA_Type3_Message.h"
#include "RS422_FPA_Message.h"
#include "RS422_FPA_Type4_Message.h"
#include "RS422_FPA_Type1_Message.h"
#include "RS422_FPA_Type5_Message.h"
#include "RS422_FPA_Type8_Message.h"
#include <Preferences.h>
#include "FPA422Handler.h"
#include "esp_log.h"
static const char* FPA422_TAG = "FPA422";

#ifdef ALLOW_HARDWARESERIAL
#include <HardwareSerial.h>
#endif

#ifdef ALLOW_BLUETOOTH
#include "BluetoothSerial.h"
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;
#endif


#include <esp_wifi.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <Preferences.h>
#include "AsyncUDP.h"
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#ifdef ALLOW_BLE
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#endif


#ifdef HOMENETWORK
const char  Myssid[] = "YourHomeSSID";
const char  Mypassword[] = "YourHomePassWd";
#else
const char Myssid[] = "ESP32_01";
const char Mypassword[] = "010419671";
#endif

const char* wifi_network_ssid = "YourHomeSSID";
const char* wifi_network_password =  "YourHomePassWd";

const char *soft_ap_ssid = "ESP32_01";
const char *soft_ap_password = "010419671";
AsyncUDP udp;

#ifdef ALLOW_BLE
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
#define SERVICE_UUID        "6f000000-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID "6f000000-b5a3-f393-e0a9-e50e24dcca9e"


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      BLEDevice::startAdvertising();
      ESP_LOGI(FPA422_TAG,"%s","BLE Connected");

    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      ESP_LOGI(FPA422_TAG,"%s","BLE Disconnected");
    }
};

void StartBLE()
{
  BLEDevice::init("SFS-Link-FPA");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x06);  // set value to 0x00 to not advertise this parameter
  pAdvertising->setMaxPreferred(0x12);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
}

#endif




FPA422Handler::FPA422Handler()
{
    //ctor
    //

    Message5.SetTypeToLeft();
    Message5.SetName("Left fencer",11);
    Message5.SetNOC("BEL");
    Message6.SetTypeToRight();
    Message6.SetName("Right fencer",12);
    Message6.SetNOC("FRA");


}

FPA422Handler::~FPA422Handler()
{
    //dtor

}

#ifdef ALLOW_BLUETOOTH
void FPA422Handler::StartBluetooth()
{
  SerialBT.begin("ESP32ScoringDevice1"); //Bluetooth device name
  m_BlueToothStarted = true;
  TimeForNext1_2s =millis() + 1200;
  TimeForNext12s =millis() + 12500;
}
#endif

#ifdef ALLOW_HARDWARESERIAL
HardwareSerial MySerial(2);
void FPA422Handler::StartHWSerial()
{
    if(!m_HWSerialStarted)
    {
        MySerial.begin(115200, SERIAL_8N1, 16, 17);
        //MySerial.begin(9600, SERIAL_8N1, 16, 17);
        m_HWSerialStarted = true;
    }

}
#endif



void FPA422Handler::StartWiFi()
{
  NetWork &MyNetWork = NetWork::getInstance();
  MyNetWork.GlobalStartWiFi();
  IPAddress localip = WiFi.localIP();
  IPAddress ip = WiFi.softAPIP();

ESP_LOGI(FPA422_TAG,"ESP32 IP as soft AP: %s",(WiFi.softAPIP().toString()).c_str());


  m_WifiStarted = true;
  TimeForNext1_2s =millis() + 1200;
  TimeForNext12s =millis() + 12500;
  m_WifiPeriodicalUpdateCounter = 0;
  m_SlowWifiPeriodicalUpdateCounter = 0;

  sprintf(LocalIPAddress, "%d.%d.%d.%d", localip[0], localip[1], localip[2], localip[3]);
  sprintf(SoftAPIPAddress, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  Preferences networkpreferences;
  networkpreferences.begin("credentials", false);
  PisteNr = networkpreferences.getInt("pisteNr", 500);
  networkpreferences.end();
  Message10.SetPiste(PisteNr);
  Message10.SetIPAddress((uint32_t)localip);
  //Message10.Print();
  AnnouncingPort = 65535 - (2* PisteNr) +1;
  if(MyNetWork.IsExternalWifiAvailable()){
    SetCyranoStatus('1');
  }
#ifdef ALLOW_BLE
  StartBLE();
#endif
#ifdef ALLOW_HARDWARESERIAL
    StartHWSerial();
#endif
}

#ifdef ALLOW_BLUETOOTH
void FPA422Handler::BTTPeriodicalUpdate()
{
    if(millis() > TimeForNext1_2s)
    {
        BTTransmitMessage(1);
        BTTransmitMessage(2);
        BTTransmitMessage(3);
        TimeForNext1_2s = millis() + 1200;
        return;
    }
    if(millis() > TimeForNext12s)
    {
        BTTransmitMessage(4);
        BTTransmitMessage(5);
        BTTransmitMessage(6);
        BTTransmitMessage(7);
        BTTransmitMessage(8);
        TimeForNext12s = millis() + 12000;
        return;
    }
}
#endif
void FPA422Handler::WifiPeriodicalUpdate()
{
    if(millis() > TimeForNext1_2s)
    {
      if(0 == m_WifiPeriodicalUpdateCounter)
      {
        AllProtocolsTransmitMessage(1);
      }
      if(1 == m_WifiPeriodicalUpdateCounter)
      {
        AllProtocolsTransmitMessage(2);
      }
      if(2 == m_WifiPeriodicalUpdateCounter)
      {
        AllProtocolsTransmitMessage(3);
        TimeForNext1_2s = millis() + 1200;
        m_WifiPeriodicalUpdateCounter = 0;
        return;
      }
      m_WifiPeriodicalUpdateCounter++;
#ifdef ALLOW_BLE
      if (!deviceConnected && oldDeviceConnected) {

        pServer->startAdvertising(); // restart advertising
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
#endif
      return;

    }

    if(millis() > TimeForNext12s)
    {
      if(0 == m_SlowWifiPeriodicalUpdateCounter)
      {
        WifiTransmitMessage(4);
      }
      if(1 == m_SlowWifiPeriodicalUpdateCounter)
      {
        WifiTransmitMessage(5);
      }
      if(2 == m_SlowWifiPeriodicalUpdateCounter)
      {
        WifiTransmitMessage(6);
      }
      if(3 == m_SlowWifiPeriodicalUpdateCounter)
      {
        WifiTransmitMessage(7);
      }
      if(4 == m_SlowWifiPeriodicalUpdateCounter)
      {
        WifiTransmitMessage(8);
      }
      if(5 == m_SlowWifiPeriodicalUpdateCounter)
      {
        WifiTransmitMessage(10);
        if(m_WifiStarted)
        {
          udp.broadcastTo(SoftAPIPAddress, AnnouncingPort,TCPIP_ADAPTER_IF_AP);
          udp.broadcastTo(LocalIPAddress, AnnouncingPort,TCPIP_ADAPTER_IF_STA);

        }
        TimeForNext12s = millis() + 12000;
        m_SlowWifiPeriodicalUpdateCounter = 0;
        return;
      }
      m_SlowWifiPeriodicalUpdateCounter++;
      return;

    }

}


/*void FPA422Handler::BTTransmitMessage(int Type)
{
  return;
#ifdef ALLOW_BLUETOOTH
  if((Type < 1) || (Type > MAX_MESSAGE_TYPE))
    return;
  if(m_BlueToothStarted)
  {
    switch(Type)
    {
      case 1:
      SerialBT.write(Message1.GetBuffer(),Message1.GetCurrentSize());

      break;

      case 2:
      SerialBT.write(Message2.GetBuffer(),Message2.GetCurrentSize());

      break;

      case 3:
      SerialBT.write(Message3.GetBuffer(),Message3.GetCurrentSize());
      break;

      case 4:
      SerialBT.write(Message4.GetBuffer(),Message4.GetCurrentSize());
      break;

      case 5:
      SerialBT.write(Message5.GetBuffer(),Message5.GetCurrentSize());
      break;

      case 6:
      SerialBT.write(Message6.GetBuffer(),Message6.GetCurrentSize());
      break;

      case 7:
      //SerialBT.write(Message7.GetBuffer(),Message7.GetCurrentSize());
      break;

      case 8:
      SerialBT.write(Message8.GetBuffer(),Message8.GetCurrentSize());
      break;

      case 9:
      //SerialBT.write(Message9.GetBuffer(),Message9.GetCurrentSize());
      break;

      default:
      ;
    }
  }
#endif
}
*/


/*void FPA422Handler::WifiTransmitMessage(int Type)
{
  if((Type < 1) || (Type > MAX_MESSAGE_TYPE))
    return;
  if(m_WifiStarted)
  {
    switch(Type)
    {
      case 1:
      udp.broadcastTo(Message1.GetBuffer(),Message1.GetCurrentSize(), UDPPort,TCPIP_ADAPTER_IF_AP);
      //udp.broadcastTo(Message1.GetBuffer(),Message1.GetCurrentSize(), UDPPort,TCPIP_ADAPTER_IF_STA);

      break;

      case 2:
      udp.broadcastTo(Message2.GetBuffer(),Message2.GetCurrentSize(), UDPPort,TCPIP_ADAPTER_IF_AP);
      //udp.broadcastTo(Message2.GetBuffer(),Message2.GetCurrentSize(), UDPPort,TCPIP_ADAPTER_IF_STA);

      break;

      case 3:
      udp.broadcastTo(Message3.GetBuffer(),Message3.GetCurrentSize(), UDPPort,TCPIP_ADAPTER_IF_AP);
      //udp.broadcastTo(Message3.GetBuffer(),Message3.GetCurrentSize(), UDPPort,TCPIP_ADAPTER_IF_STA);

      break;

      case 4:
      udp.broadcastTo(Message4.GetBuffer(),Message4.GetCurrentSize(), UDPPort,TCPIP_ADAPTER_IF_AP);
      //udp.broadcastTo(Message4.GetBuffer(),Message4.GetCurrentSize(), UDPPort,TCPIP_ADAPTER_IF_STA);

      break;

      case 5:

      udp.broadcastTo(Message5.GetBuffer(),Message5.GetCurrentSize(), UDPPort,TCPIP_ADAPTER_IF_AP);
      //udp.broadcastTo(Message5.GetBuffer(),Message5.GetCurrentSize(), UDPPort,TCPIP_ADAPTER_IF_STA);
      break;

      case 6:

      udp.broadcastTo(Message6.GetBuffer(),Message6.GetCurrentSize(), UDPPort,TCPIP_ADAPTER_IF_AP);
      //udp.broadcastTo(Message6.GetBuffer(),Message6.GetCurrentSize(), UDPPort,TCPIP_ADAPTER_IF_STA);

      break;

      case 7:
      //udp.broadcastTo(Message7.GetBuffer(),Message7.GetCurrentSize(), UDPPort,TCPIP_ADAPTER_IF_AP);
      //udp.broadcastTo(MeMessage3ssage7.GetBuffer(),Message7.GetCurrentSize(), UDPPort,TCPIP_ADAPTER_IF_STA);
      break;

      case 8:
      udp.broadcastTo(Message8.GetBuffer(),Message8.GetCurrentSize(), UDPPort,TCPIP_ADAPTER_IF_AP);
      //udp.broadcastTo(Message8.GetBuffer(),Message8.GetCurrentSize(), UDPPort,TCPIP_ADAPTER_IF_STA);

      break;

      case 9:
      //udp.broadcastTo(Message9.GetBuffer(),Message9.GetCurrentSize(), UDPPort,TCPIP_ADAPTER_IF_AP);
      //udp.broadcastTo(Message9.GetBuffer(),Message9.GetCurrentSize(), UDPPort,TCPIP_ADAPTER_IF_STA);
      break;

      case 10:
      udp.broadcastTo(Message10.GetBuffer(),Message10.GetCurrentSize(), UDPPort,TCPIP_ADAPTER_IF_AP);
      //udp.broadcastTo(Message9.GetBuffer(),Message9.GetCurrentSize(), UDPPort,TCPIP_ADAPTER_IF_STA);

      break;

      default:
      ;
    }
  }

}
*/

void FPA422Handler::WifiTransmitMessage(int Type)
{
  if((Type < 1) || (Type > MAX_MESSAGE_TYPE))
    return;

  if(m_WifiStarted)
  {
      udp.broadcastTo(Meassages[Type-1]->GetBuffer(),Meassages[Type-1]->GetCurrentSize(), UDPPort,TCPIP_ADAPTER_IF_AP);
      NetWork &MyNetWork = NetWork::getInstance();
      if(MyNetWork.IsExternalWifiAvailable())
        udp.broadcastTo(Meassages[Type-1]->GetBuffer(),Meassages[Type-1]->GetCurrentSize(), UDPPort,TCPIP_ADAPTER_IF_STA);
  }
}

#ifdef ALLOW_BLUETOOTH
void FPA422Handler::BTTransmitMessage(int Type)
{
  if((Type < 1) || (Type > MAX_MESSAGE_TYPE))
    return;
  if(m_BlueToothStarted)
  {
    SerialBT.write(Meassages[Type-1]->GetBuffer(),Meassages[Type-1]->GetCurrentSize());
  }
}
#endif


void FPA422Handler::AllProtocolsTransmitMessage(int Type)
{
  if((Type < 1) || (Type > MAX_MESSAGE_TYPE))
    return;
  if(m_WifiStarted)
  {
      udp.broadcastTo(Meassages[Type-1]->GetBuffer(),Meassages[Type-1]->GetCurrentSize(), UDPPort,TCPIP_ADAPTER_IF_AP);
      NetWork &MyNetWork = NetWork::getInstance();
      if(MyNetWork.IsExternalWifiAvailable())
        udp.broadcastTo(Meassages[Type-1]->GetBuffer(),Meassages[Type-1]->GetCurrentSize(), UDPPort,TCPIP_ADAPTER_IF_STA);
  }
#ifdef ALLOW_BLUETOOTH
  if(m_BlueToothStarted)
  {
    SerialBT.write(Meassages[Type-1]->GetBuffer(),Meassages[Type-1]->GetCurrentSize());
  }
#endif
#ifdef ALLOW_BLE
if (deviceConnected) {
      pCharacteristic->setValue((uint8_t*)Meassages[Type-1]->GetBuffer(),Meassages[Type-1]->GetCurrentSize());
      pCharacteristic->notify();
      //Meassages[Type-1]->Print();
  }
#endif
#ifdef ALLOW_HARDWARESERIAL
MySerial.write(Meassages[Type-1]->GetBuffer(),Meassages[Type-1]->GetCurrentSize());

#endif
}

#define MASK_ANY_ORANGE (MASK_ORANGE_L | MASK_ORANGE_R)

void FPA422Handler::ProcessLightsChange(uint32_t eventtype)
{
  uint32_t event_data = eventtype & SUB_TYPE_MASK;
  //if(!(event_data & ~MASK_ANY_ORANGE))  // no need to broadcast orange lights
    //return;
  Message1.SetRed(event_data & MASK_RED);
  Message1.SetGreen(event_data & MASK_GREEN);
  Message1.SetWhiteLeft(event_data & MASK_WHITE_L);
  Message1.SetWhiteRight(event_data & MASK_WHITE_R );

  AllProtocolsTransmitMessage(1);
  //Message1.Print();
}

void FPA422Handler::update (FencingStateMachine *subject, uint32_t eventtype)
{
  uint32_t event_data = eventtype & SUB_TYPE_MASK;
  uint32_t maineventtype = eventtype & MAIN_TYPE_MASK ;
  mix_t PCardInfo;
  mix_t TimeInfo;
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
    switch (event_data)
    {

      case WEAPON_MASK_EPEE:
      Message4.setWeapon(EPEE);
      break;

      case WEAPON_MASK_SABRE:
      Message4.setWeapon(SABRE);
      break;

      case WEAPON_MASK_FOIL:
      Message4.setWeapon(FOIL);
      break;

      default:
      Message4.setWeapon(UNKNOWN);

    }

    //Message4.Print();
    AllProtocolsTransmitMessage(4);

    break;

    case EVENT_SCORE_LEFT:
    Message3.SetScoreLeft(event_data);
    AllProtocolsTransmitMessage(3);

    break;

    case EVENT_SCORE_RIGHT:
      Message3.SetScoreRight(event_data);
      AllProtocolsTransmitMessage(3);
    break;

    case EVENT_TIMER_STATE:

      if(eventtype & DATA_24BIT_MASK)
      {
        //Blynk.setProperty(V0,"color",BLYNK_GREEN);
        //Blynk.virtualWrite(V1,1);
        Message2.SetTimerStatus('R');
        AllProtocolsTransmitMessage(2);
      }
      else
      {
        //Blynk.setProperty(V0,"color",BLYNK_RED);
        //Blynk.virtualWrite(V1,0);
        Message2.SetTimerStatus('N');
        AllProtocolsTransmitMessage(2);
      }
    break;
    case EVENT_TIMER:
    newseconds = event_data & (DATA_BYTE1_MASK |DATA_BYTE2_MASK);

    TimeInfo.theDWord = eventtype & DATA_24BIT_MASK;
    m_seconds = TimeInfo.theBytes[1];
    m_minutes = TimeInfo.theBytes[2];
    m_hundredths = TimeInfo.theBytes[0];
      if(previous_seconds != newseconds)
      {
        //subject->GetFormattedStringTime(chrono,2,0);
        //Blynk.virtualWrite(V0, chrono);
        Message2.SetTime(m_minutes,m_seconds,m_hundredths);
        //Message2.SetTime(2,37,99);
        AllProtocolsTransmitMessage(2);
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
      if( 255 == currentRound)
        Message3.SetRound(-1);
      else
        Message3.SetRound(currentRound);
      AllProtocolsTransmitMessage(3);
    break;

    case EVENT_YELLOW_CARD_LEFT:
      Message3.SetYellowCardLeft(event_data);
      AllProtocolsTransmitMessage(3);
    break;

    case EVENT_YELLOW_CARD_RIGHT:
    Message3.SetYellowCardRight(event_data);
    AllProtocolsTransmitMessage(3);
    break;

    case EVENT_RED_CARD_LEFT:
    Message3.SetRedCardLeft(event_data);
    AllProtocolsTransmitMessage(3);
    break;

    case EVENT_RED_CARD_RIGHT:
    Message3.SetRedCardRight(event_data);
    AllProtocolsTransmitMessage(3);
    break;

    case EVENT_BLACK_CARD_LEFT:
    Message3.SetBlackCardLeft(event_data);
    AllProtocolsTransmitMessage(3);
    break;

    case EVENT_BLACK_CARD_RIGHT:
    Message3.SetBlackCardRight(event_data);
    AllProtocolsTransmitMessage(3);
    break;

    case EVENT_P_CARD:

        PCardInfo.theDWord = eventtype & DATA_24BIT_MASK;
        Message8.SetPCardLeft(PCardInfo.theBytes[0]);
        Message8.SetPCardRight(PCardInfo.theBytes[1]);
        AllProtocolsTransmitMessage(8);

    break;

    case EVENT_UW2F_TIMER:

        TimeInfo.theDWord = eventtype & DATA_24BIT_MASK;
        Message8.SetTime(TimeInfo.theBytes[2],TimeInfo.theBytes[1]);

        AllProtocolsTransmitMessage(8);
    break;


    case EVENT_PRIO:
    switch(event_data)
    {
      case 2:
        Message3.SetPrioRight();
      break;
      case 1:
        Message3.SetPrioLeft();
      break;
      default:
        Message3.SetNoPrio();
    }
    AllProtocolsTransmitMessage(3);
    break;

  }

}


/*void SetName(const char* name, size_t len = 20);
void SetNOC(const char* NOC);*/

void FPA422Handler::update (CyranoHandler *subject, std::string strEFP1Message)
{
  EFP1Message EFP1Input(strEFP1Message);
  if(EFP1Input[Command] == "NAK")
  {
    Message10.SetMachineStatus('5');
    AllProtocolsTransmitMessage(10);
    return;
  }
  if(EFP1Input[Command] == "ACK")
  {
    // Ignore for now
    return;
  }
  if((EFP1Input[Command] == "INFO") || (EFP1Input[Command] == "DISP"))
  {
    if(EFP1Input[RightFencerId] != "")
      Message6.SetUID(EFP1Input[RightFencerId].c_str(),EFP1Input[RightFencerId].length());
    if(EFP1Input[RightFencerName] != "")
      Message6.SetName(EFP1Input[RightFencerName].c_str(),EFP1Input[RightFencerName].length());
    if(EFP1Input[RightFencerNation] != "")
      Message6.SetNOC(EFP1Input[RightFencerNation].c_str());
    if(EFP1Input[LeftFencerId] != "")
      Message5.SetUID(EFP1Input[LeftFencerId].c_str(),EFP1Input[LeftFencerId].length());
    if(EFP1Input[LeftFencerName] != "")
      Message5.SetName(EFP1Input[LeftFencerName].c_str(),EFP1Input[LeftFencerName].length());
    if(EFP1Input[LeftFencerNation] != "")
      Message5.SetNOC(EFP1Input[LeftFencerNation].c_str());
    AllProtocolsTransmitMessage(5);
    AllProtocolsTransmitMessage(6);
    /*if(EFP1Input.EFP1StatusString2Type10MessageStatus() != Message10.GetMachineStatus())
    {

      Message10.SetMachineStatus(EFP1Input.EFP1StatusString2Type10MessageStatus());
      BTTransmitMessage(10);
      WifiTransmitMessage(10);
    }*/
  }

}

void FPA422Handler::update (CyranoHandler *subject, uint32_t eventtype)
{
  if(EVENT_CYRANO_STATE == (eventtype & MAIN_TYPE_MASK))
  {
    mix_t thestatus;
    thestatus.theDWord = eventtype & DATA_24BIT_MASK;
    Message10.SetMachineStatus(thestatus.theBytes[0]);
    AllProtocolsTransmitMessage(10);
  }

}
