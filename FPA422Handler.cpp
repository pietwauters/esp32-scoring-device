//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "weaponenum.h"
#include "RS422_FPA_Type2_Message.h"
#include "RS422_FPA_Type3_Message.h"
#include "RS422_FPA_Message.h"
#include "RS422_FPA_Type4_Message.h"
#include "RS422_FPA_Type1_Message.h"
#include "RS422_FPA_Type5_Message.h"
#include "RS422_FPA_Type8_Message.h"

#include "FPA422Handler.h"
#include <HardwareSerial.h>
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

#include <esp_wifi.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <Preferences.h>
#include "AsyncUDP.h"

Preferences preferences;
String ssid;
String password;

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

void FPA422Handler::StartBluetooth()
{
  SerialBT.begin("ESP32ScoringDevice1"); //Bluetooth device name
  m_BlueToothStarted = true;
  TimeForNext1_2s =millis() + 1200;
  TimeForNext12s =millis() + 12500;
}

HardwareSerial MySerial(2);
void FPA422Handler::StartHWSerial()
{
    if(!m_HWSerialStarted)
    {
        MySerial.begin(38400, SERIAL_8N1, 16, 17);
        m_HWSerialStarted = true;
    }

}
// It seems we should not use channels above 11
#define CHANNEL_COUNT  12
#define VERY_STRONG -51
#define STRONG -61
#define WEAK -71
/*******************************************************************************
   TYPES
 ******************************************************************************/

typedef struct {
  uint8_t occupants;
  long    total_strength;
  long    max_strength;
} channel_t;

typedef struct {
  int start;
  int end;
  int width;
  int interference;
} interval_t;

/*******************************************************************************
   GLOBAL VARIABLES
 ******************************************************************************/

channel_t channels[CHANNEL_COUNT]; // Array with all the channels

/**
   Sets all the channels back to 0.
*/
void reset_channels() {
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    channels[i].occupants = 0;
    channels[i].total_strength = 0;
    channels[i].max_strength = -999999;
  }
}

int findBestWifiChannel()
{
  WiFi.disconnect();
  int networks = WiFi.scanNetworks();
  if (networks == 0) {
    // it doesn't matter, we're on an island or a cage of Farady
    return 6;
  }
  reset_channels();
  // Creating a representation of all the networks
  bool bEmptyChannelsExist = false;
  int nonEmptyChannelCount = 0;
  for (int i = 0; i < networks; ++i)
  {
   int c = WiFi.channel(i);
   if (c < CHANNEL_COUNT)
   {
     channels[c - 1].occupants += 1;
     if (channels[c - 1].max_strength < WiFi.RSSI(i))
       channels[c - 1].max_strength = WiFi.RSSI(i);
   }
 }
 for (int i = 0; i < CHANNEL_COUNT; i++)
 {
    if (channels[i].occupants > 0)
    {
      // add 3dB per access point. This is overkill when the other ap's have lower power than the highest
      channels[i].total_strength = channels[i].max_strength + (channels[i].occupants - 1) * 3;
      nonEmptyChannelCount++;
    }
    else
      bEmptyChannelsExist = true;
  }
  // At this point we know the power per channel
  // If there are "empty channels", we should use one them,
  // if not, we should use the one with the lowest esp_wifi_get_max_tx_power
  // From the empty channels, we should use the one with the least interference
  // from adjacent reset_channels
  int CurrentMinIndex = 0;
  long CurrentMin = 0;
  int proposal = 0;
  if(!bEmptyChannelsExist)
  {
    for (int i = 0; i < CHANNEL_COUNT - 1; i++)
    {
      if(channels[i].total_strength < CurrentMin)
      {
        CurrentMin = channels[i].total_strength;
        CurrentMinIndex = i;
      }
    }
    return CurrentMin;
  }
  // if one of the optimal channels is free, chose that one
  if(channels[0].occupants == 0) // chose 1 or 11
  {
    if(channels[10].occupants != 0)
    {
      return 0;
    }
    else
    {
      // you should chose 1 or 11 based on how "far" they are from other interference
      // for now always return 1
      return 0;
    }
  }
  else
  {
    if(channels[10].occupants == 0)
    {
      return 10;
    }
    else
    {
      if(channels[5].occupants == 0)
      {
        return 5;
      }
    }
  }
  // if we reach this point, none of the main channels are available

interval_t intervals [10];
int nr_intervals = 0;
int max_width = 0;
int max_width_index = 0;
long best_interference = -999999999;


  // Find pairs of non-empty channels
  for (int i = 1; i < CHANNEL_COUNT -1 ; i++)
  {
    int j = i;
    while(channels[j].occupants != 0)
      j++;
    intervals[nr_intervals].start = j;
    int k = j+1;
    while(channels[k].occupants == 0)
      k++;
    intervals[nr_intervals].end = k-1;

    if(k-j >= max_width)
    {
      long currentinterference = max(channels[j-1].total_strength,channels[k].total_strength);
      if(k-j == max_width)
      {// both intervals are equally wide, so chose the one with the lowest adjacent power

        if(currentinterference < best_interference)
        {
          max_width = k-j;
          max_width_index = nr_intervals;
          best_interference = currentinterference;
        }
      }
      else
      {
        max_width = k-j;
        max_width_index = nr_intervals;
        best_interference = currentinterference;
      }

    }
    nr_intervals++;
    i = k-1;
  }
  /*Serial.print("nr_intervals = "); Serial.println(nr_intervals);
  Serial.print("max_width = "); Serial.println(max_width);
  Serial.print("max_width_index = "); Serial.println(max_width_index);
  Serial.print("max_width_interval = ("); Serial.print(intervals[max_width_index].start); Serial.print(",");Serial.println(intervals[max_width_index].end);*/

  return ((intervals[max_width_index].end - intervals[max_width_index].start) / 2 + intervals[max_width_index].start);

}

void FPA422Handler::StartWiFi()
{
  preferences.begin("credentials", false);
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("password", "");

  WiFi.mode(WIFI_MODE_APSTA);
  WiFi.softAP(soft_ap_ssid, soft_ap_password);

  int bestchannel = findBestWifiChannel() + 1;
  Serial.print("current best channel = "); Serial.println(bestchannel);
  esp_wifi_set_channel(bestchannel,WIFI_SECOND_CHAN_NONE);


  WiFi.begin(ssid.c_str(), password.c_str());
  for(int i =0; i<50;i++)
  {
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
          delay(200);
    }
    else
     i = 1000;
  }


  IPAddress localip = WiFi.localIP();
  IPAddress ip = WiFi.softAPIP();

  Serial.print("ESP32 IP as soft AP: ");
  Serial.println(WiFi.softAPIP());

  Serial.print("ESP32 IP on the WiFi network: ");
  Serial.println(WiFi.localIP());

  m_WifiStarted = true;
  TimeForNext1_2s =millis() + 1200;
  TimeForNext12s =millis() + 12500;

  sprintf(LocalIPAddress, "%d.%d.%d.%d", localip[0], localip[1], localip[2], localip[3]);
  sprintf(SoftAPIPAddress, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  AnnouncingPort = 65535 - (2* PisteNr) +1;


}


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

void FPA422Handler::WifiPeriodicalUpdate()
{
    if(millis() > TimeForNext1_2s)
    {
        WifiTransmitMessage(1);
        WifiTransmitMessage(2);
        WifiTransmitMessage(3);
        TimeForNext1_2s = millis() + 1200;
        return;
    }
    if(millis() > TimeForNext12s)
    {
        WifiTransmitMessage(4);
        WifiTransmitMessage(5);
        WifiTransmitMessage(6);
        WifiTransmitMessage(7);
        WifiTransmitMessage(8);
        if(m_WifiStarted)
        {
          udp.broadcastTo(SoftAPIPAddress, AnnouncingPort,TCPIP_ADAPTER_IF_AP);
          //udp.broadcastTo(LocalIPAddress, AnnouncingPort,TCPIP_ADAPTER_IF_STA);

        }
        TimeForNext12s = millis() + 12000;
        return;
    }

}

void FPA422Handler::BTTransmitMessage(int Type)
{
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

}
void FPA422Handler::WifiTransmitMessage(int Type)
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

      default:
      ;
    }
  }

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
  BTTransmitMessage(1);
  WifiTransmitMessage(1);
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
    BTTransmitMessage(4);
    WifiTransmitMessage(4);

    break;

    case EVENT_SCORE_LEFT:
    Message3.SetScoreLeft(event_data);
    BTTransmitMessage(3);
    WifiTransmitMessage(3);

    break;

    case EVENT_SCORE_RIGHT:
      Message3.SetScoreRight(event_data);
      BTTransmitMessage(3);
      WifiTransmitMessage(3);
    break;

    case EVENT_TIMER_STATE:

      if(eventtype & DATA_24BIT_MASK)
      {
        //Blynk.setProperty(V0,"color",BLYNK_GREEN);
        //Blynk.virtualWrite(V1,1);
        Message2.SetTimerStatus('R');
        BTTransmitMessage(2);
        WifiTransmitMessage(2);
      }
      else
      {
        //Blynk.setProperty(V0,"color",BLYNK_RED);
        //Blynk.virtualWrite(V1,0);
        Message2.SetTimerStatus('N');
        BTTransmitMessage(2);
        WifiTransmitMessage(2);
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
        BTTransmitMessage(2);
        WifiTransmitMessage(2);
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
      BTTransmitMessage(3);
      WifiTransmitMessage(3);
    break;

    case EVENT_YELLOW_CARD_LEFT:
      Message3.SetYellowCardLeft(event_data);
      BTTransmitMessage(3);
      WifiTransmitMessage(3);
    break;

    case EVENT_YELLOW_CARD_RIGHT:
    Message3.SetYellowCardRight(event_data);
    BTTransmitMessage(3);
      WifiTransmitMessage(3);
    break;

    case EVENT_RED_CARD_LEFT:
    Message3.SetRedCardLeft(event_data);
    BTTransmitMessage(3);
      WifiTransmitMessage(3);
    break;

    case EVENT_RED_CARD_RIGHT:
    Message3.SetRedCardRight(event_data);
    BTTransmitMessage(3);
      WifiTransmitMessage(3);
    break;

    case EVENT_P_CARD:

        PCardInfo.theDWord = eventtype & DATA_24BIT_MASK;
        Message8.SetPCardLeft(PCardInfo.theBytes[0]);
        Message8.SetPCardRight(PCardInfo.theBytes[1]);
        BTTransmitMessage(8);
        WifiTransmitMessage(8);

    break;

    case EVENT_UW2F_TIMER:

        TimeInfo.theDWord = eventtype & DATA_24BIT_MASK;
        Message8.SetTime(TimeInfo.theBytes[2],TimeInfo.theBytes[1]);

        BTTransmitMessage(8);
        WifiTransmitMessage(8);
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
    BTTransmitMessage(3);
      WifiTransmitMessage(3);
    break;


  }

}


/*void SetName(const char* name, size_t len = 20);
void SetNOC(const char* NOC);*/

void FPA422Handler::update (CyranoHandler *subject, string strEFP1Message)
{
  EFP1Message EFP1Input(strEFP1Message);
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
  BTTransmitMessage(5);
  WifiTransmitMessage(5);
  BTTransmitMessage(6);
  WifiTransmitMessage(6);

}
