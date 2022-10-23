//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "network.h"
#include <esp_wifi.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <Preferences.h>
#include "AsyncUDP.h"
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager


/**
   Sets all the channels back to 0.
*/
void NetWork::reset_channels() {
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    channels[i].occupants = 0;
    channels[i].total_strength = 0;
    channels[i].max_strength = -999999;
  }
}

int32_t NetWork::FindFirstFreePisteID(uint32_t RequestedPiste)
{
  WiFi.disconnect();
  int networks = WiFi.scanNetworks();
  int32_t TempPisteId = 0;
  int32_t LastPisteId = 0;
  bool RequestedPisteAlreadyInUse = false;
  for (int i = 0; i < networks; ++i)
  {
    //WiFi.SSID(i);
    Serial.println(WiFi.SSID(i));
    if(sscanf(WiFi.SSID(i).c_str(),"%d",&TempPisteId))
    {
      if(RequestedPiste == TempPisteId)
        RequestedPisteAlreadyInUse = true;
      if(TempPisteId > LastPisteId)
        LastPisteId = TempPisteId;
    }
  }
  if(!RequestedPisteAlreadyInUse)
    return RequestedPiste;

  return LastPisteId + 1;
}



int NetWork::findBestWifiChannel()
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


void NetWork::GlobalStartWiFi()
{
  if(m_GlobalWifiStarted)
    return;


  networkpreferences.begin("credentials", false);
  ssid = networkpreferences.getString("ssid", "");
  password = networkpreferences.getString("password", "");
  uint32_t PisteNr = networkpreferences.getInt("pisteNr", -1);
  //PisteNr = 2;
  if(-1 != PisteNr )
  {
    char temp[8];
    sprintf(temp,"%03d",FindFirstFreePisteID(PisteNr));
    soft_ap_ssid = "Piste_" + (String)temp;
  }
  else
  {
    char temp[8];
    sprintf(temp,"%03d",FindFirstFreePisteID(500));
    soft_ap_ssid = "Piste_" + (String)temp;
  }
  networkpreferences.end();

  WiFi.mode(WIFI_MODE_APSTA);

  Serial.println("now looking to connect to global network");

  bool bConnectedToExternalNetwork = false;

  if((ssid != "") && (password != ""))
  {
    WiFi.begin(ssid.c_str(), password.c_str());
    for(int i =0; i<10;i++)
    {
      if (WiFi.waitForConnectResult() != WL_CONNECTED)
      {
            delay(500);
      }
      else
      {
        bConnectedToExternalNetwork = true;
        i = 1000;
      }

    }
  }

  if(bConnectedToExternalNetwork)
  {
    WiFi.softAP(soft_ap_ssid.c_str(), soft_ap_password.c_str());
    Serial.print("ESP32 IP on the WiFi network: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    int bestchannel = findBestWifiChannel() + 1;
    WiFi.mode(WIFI_MODE_AP);

    WiFi.softAP(soft_ap_ssid.c_str(), soft_ap_password.c_str());
    esp_wifi_set_channel(bestchannel,WIFI_SECOND_CHAN_NONE);
    Serial.print("current best channel = "); Serial.println(bestchannel);
  }

  m_GlobalWifiStarted = true;

}


NetWork::NetWork()
{
    //ctor
    m_GlobalWifiStarted = false;

}

NetWork::~NetWork()
{
    //dtor

}
