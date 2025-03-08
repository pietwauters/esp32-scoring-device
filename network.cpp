//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "network.h"
#include <esp_wifi.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <Preferences.h>
#include <nvs_flash.h>
#include "AsyncUDP.h"
#include <esp_now.h>
// Below is for OTA updates using a webserver
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include "esp_log.h"
static const char* NETWORK_TAG = "Network";

//AsyncWebServer server(80);
//WiFiManager wm;
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


int NetWork::begin()
{
  WiFi.disconnect();
  networkpreferences.begin("credentials", false);
  LookForExternalWiFi=networkpreferences.getBool("TryGlobalWiFi",false);
  networkpreferences.end();
  /*if(!LookForExternalWiFi)
    return 0;*/
  networks = WiFi.scanNetworks();
  if(wm.getWiFiIsSaved())
  {
    for (int i = 0; i < networks; ++i)
    {
      if(WiFi.SSID(i) == wm.getWiFiSSID(true))
      {
        SavedNetworkExists = true;
        i = networks;
      }
    }
  }
}


int32_t NetWork::FindFirstFreePisteID(uint32_t RequestedPiste)
{
  if(networks == -1)
  {
    WiFi.disconnect();
    networks = WiFi.scanNetworks();
  }
  int32_t TempPisteId = 0;
  int32_t LastPisteId = 0;
  bool RequestedPisteAlreadyInUse = false;
  for (int i = 0; i < networks; ++i)
  {

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
  if(networks == -1)
  {
    WiFi.disconnect();
    networks = WiFi.scanNetworks();
  }
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

  return ((intervals[max_width_index].end - intervals[max_width_index].start) / 2 + intervals[max_width_index].start);

}


bool NetWork::ConnectToExternalNetwork(long ConnectTimeout)
{
  if(bConnectedToExternalNetwork)
    return true;
  if(! wm.getWiFiIsSaved())
    return false;
  if(!SavedNetworkExists)
    return false;
  if(!LookForExternalWiFi)
    return false;
  WiFi.disconnect();

  long Stop = millis() + ConnectTimeout * 1000;
  WiFi.mode(WIFI_MODE_APSTA);
  WiFi.begin(wm.getWiFiSSID(true).c_str(), wm.getWiFiPass(true).c_str());
  wm.setEnableConfigPortal(false);
  wm.setConfigPortalBlocking(false);
  wm.setConfigPortalTimeout(5);
  wm.setConnectTimeout(ConnectTimeout);

  while(millis() < Stop)
  {
    if(WiFi.status() == WL_CONNECTED)
    {
      Stop = 0;
      bConnectedToExternalNetwork= true;
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);
    ESP_LOGI(NETWORK_TAG, "%s","x");
  }
  if(bConnectedToExternalNetwork)// if connected with saved credentials is successful we have to start the local AP ourselves
  {
    WiFi.softAP(soft_ap_ssid.c_str(), soft_ap_password.c_str());
    ESP_LOGI(NETWORK_TAG, "ESP32 IP on the WiFi network: %s","x",(WiFi.localIP().toString()).c_str());
  }
  return bConnectedToExternalNetwork;
}


void SetIPAddress(int CurrentPisteNr){
  if(CurrentPisteNr > 254)
    return;
  // Set your Gateway IP address

  Preferences networkpreferences;
  networkpreferences.begin("credentials", false);
  if(networkpreferences.getBool("UseDHCP",false))
    return;
  String strBaseAddress = networkpreferences.getString("BaseAddress", "172.20.255.1");
  networkpreferences.end();

  uint8_t octet1=172;
  uint8_t octet2=20;
  uint8_t octet3=255;
  uint8_t octet4=1;
  sscanf(strBaseAddress.c_str(),"%d.%d.%d.%d",&octet1,&octet2,&octet3,&octet4);
  IPAddress local_IP;
  IPAddress gateway(octet1, octet2, 0, 1); // I may want to make that condigurable too
  if(octet3 == 255){
    local_IP = IPAddress(octet1, octet2, CurrentPisteNr, octet4);
  }
  else {
    if(octet4 == 255){
      local_IP = IPAddress(octet1, octet2, octet3,CurrentPisteNr);}
    else
      return;
  }
  IPAddress subnet(255, 255, 0, 0);
  IPAddress primaryDNS(8, 8, 8, 8);   //optional
  IPAddress secondaryDNS(8, 8, 4, 4); //optional
  // Set your Static IP address
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    ESP_LOGE(NETWORK_TAG, "%s","STA Failed to configure");

  }


}

void NetWork::GlobalStartWiFi()
{
  if(m_GlobalWifiStarted)
    return;

  Preferences networkpreferences;
  networkpreferences.begin("credentials", false);
  int32_t PisteNr = networkpreferences.getInt("pisteNr", -1);
  soft_ap_password = networkpreferences.getString("AP_Password", "01041967");
  networkpreferences.end();
  networkpreferences.begin("scoringdevice", false);
  bool bIsrepeater = networkpreferences.getBool("RepeaterMode", false);
  networkpreferences.end();
  if(-1 != PisteNr )
  {
    char temp[8];
    int CurrentNr = FindFirstFreePisteID(PisteNr);
    sprintf(temp,"%03d",CurrentNr);
    soft_ap_ssid = "Piste_" + (String)temp;
    // Set your Static IP address
    SetIPAddress(CurrentNr);
  }
  else
  {
    char temp[8];
    sprintf(temp,"%03d",FindFirstFreePisteID(500));
    soft_ap_ssid = "Piste_" + (String)temp;
  }

  // In repeater mode this part is not needed, because we have to use the same channel as the master
  if(!bIsrepeater){
    if(!ConnectToExternalNetwork(15))
    {
      bestchannel = findBestWifiChannel() + 1;
      WiFi.mode(WIFI_MODE_AP);

      WiFi.softAP(soft_ap_ssid.c_str(), soft_ap_password.c_str());
      esp_wifi_set_channel(bestchannel,WIFI_SECOND_CHAN_NONE);
    }
  }
  else{
    FindAndSetMasterChannel();
    WiFi.mode(WIFI_MODE_AP);
    WiFi.softAP(soft_ap_ssid.c_str(), soft_ap_password.c_str());
    esp_wifi_set_channel(bestchannel,WIFI_SECOND_CHAN_NONE);
  }

  m_GlobalWifiStarted = true;
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! I am ESP32.");
  });
  //esp_wifi_set_max_tx_power(20);

}

void NetWork::FindAndSetMasterChannel(int soft_retries, bool restart_on_timeout)
{
  WiFi.disconnect();
  String MasterSSID;
  Preferences networkpreferences;
  networkpreferences.begin("scoringdevice", false);
  int32_t PisteNr = networkpreferences.getInt("MasterPiste", -1);

  networkpreferences.end();
  int tempchannel = -1;
  if(-1 != PisteNr )
  {
    char temp[8];
    sprintf(temp,"%03d",PisteNr);
    MasterSSID = "Piste_" + (String)temp;
    for(int j=soft_retries; j > 0; j--){
      int networks = WiFi.scanNetworks();
      for (int i = 0; i < networks; ++i)
      {
        if(WiFi.SSID(i) == MasterSSID)
        {
          tempchannel = WiFi.channel(i);
          i = networks;
        }
      }
      if(tempchannel != -1)
        j = 0;
      yield();
    }

  }
  if(tempchannel != -1)
  {
    bestchannel = tempchannel;
    ESP_ERROR_CHECK(esp_wifi_set_channel(bestchannel ,WIFI_SECOND_CHAN_NONE));
  }
  else
    if(restart_on_timeout)
      esp_restart() ;
}

void NetWork::update (UDPIOHandler *subject, uint32_t eventtype)
{
  uint32_t maineventtype = eventtype & MAIN_TYPE_MASK ;
  uint32_t subtype = eventtype & UI_SUB_TYPE_MASK ;


  if(UI_CONNECT_TO_WIFI == subtype)
    ConnectToExternalNetwork(45);

  if(UI_START_WIFI_PORTAL == subtype){

    WaitForNewSettingsViaPortal();
  }

  if(UI_START_OTA_PORTAL == subtype)
  {

    AsyncElegantOTA.begin(&server);    // Start ElegantOTA
    server.begin();
    
    ESP_LOGI(NETWORK_TAG, "%s","HTTP OTA Update server started");
    return;
  }
  if(UI_FULL_RESET == subtype){
    ESP.restart();
  }


  switch(subtype)
  {
    case UI_SWAP_FENCERS:
    case UI_RESERVE_LEFT :
    case UI_RESERVE_RIGHT :
    case UI_INPUT_CYRANO_NEXT :
    case UI_INPUT_CYRANO_PREV :
    case UI_INPUT_CYRANO_BEGIN :
    case UI_INPUT_CYRANO_END :
    if(!ConnectToExternalNetwork(45))
    {
      WiFi.disconnect();
      WiFi.mode(WIFI_MODE_AP);
      WiFi.softAP(soft_ap_ssid.c_str(), soft_ap_password.c_str());
    }


    break;

  }
}

WiFiManagerParameter WiFiPistId("WiFiPisteId", "PisteNr","",16);
WiFiManagerParameter WiFiAPPasswd("WiFiAPPasswd", "WiFiAPPasswd","01041967",64);
WiFiManagerParameter PowerMode("PowerSaveMode", "Deep Sleep","N",1);
WiFiManagerParameter TryGlobalWiFi("TryGlobalWiFi", "Look for external network","N",1);

WiFiManagerParameter CyranoPort("CyranoPort", "Cyrano Port","50100",16);
WiFiManagerParameter CyranoBroadcastPort("CyranoBroadcastPort", "Cyrano Broadcast Port","50101",16);
WiFiManagerParameter UseDHCP("UseDHCP", "Use DHCP","N",1);
WiFiManagerParameter FixedIPAddress("IPAddressing", "IP Addressing mode","172.20.255.1",16);
WiFiManagerParameter StartUpWeapon("StartUpWeapon", "Default Weapon at start_up","F",8);

WiFiManagerParameter RepeaterMode("RepeaterMode", "Is this a repeater","N",1);
WiFiManagerParameter MasterPisteId("MasterPiste", "Piste to repeat","500",3);
WiFiManagerParameter MirrorLights("MirrorLights", "Mirror lights","N",1);

bool ToBool(const char* input){
  bool result = false;
    switch(input[0])
    {
      case 'Y':
      case 'y':
      case '1':
        result = true;
      break;
    }
    return result;
}

void saveParamsCallback () {

  int newPistId = -1;
  sscanf(WiFiPistId.getValue(),"%d",&newPistId);
  Preferences networkpreferences;
  networkpreferences.begin("credentials", false);
  networkpreferences.putInt("pisteNr", newPistId);
  networkpreferences.putString("AP_Password",WiFiAPPasswd.getValue());
  uint16_t ThePort = 0;
  sscanf(CyranoPort.getValue(),"%d",&ThePort);
  networkpreferences.putUShort("CyranoPort", ThePort);

  uint16_t TheBroadcastPort = 0;
  sscanf(CyranoBroadcastPort.getValue(),"%d",&TheBroadcastPort);
  networkpreferences.putUShort("CyranoBcPort", TheBroadcastPort);

  networkpreferences.putBool("TryGlobalWiFi",ToBool(TryGlobalWiFi.getValue()));
  networkpreferences.putBool("UseDHCP",ToBool(UseDHCP.getValue()));
  networkpreferences.putString("BaseAddress",FixedIPAddress.getValue());

  networkpreferences.end();
  Preferences mypreferences;
  mypreferences.begin("scoringdevice", false);
  uint8_t startweapon = 1;
  char theweapon =StartUpWeapon.getValue()[0];
  switch (theweapon) {
    case 'F':
    startweapon = 0;
    break;

    case 'E':
    startweapon = 1;
    break;

    case 'S':
    startweapon = 2;
    break;
  }
  mypreferences.putUChar("START_WEAPON",startweapon);

// Code related to repeater / master mode
  mypreferences.putBool("RepeaterMode",ToBool(RepeaterMode.getValue()));
  mypreferences.putBool("Powersave",ToBool(PowerMode.getValue()));
  mypreferences.putBool("MirrorLights",ToBool(MirrorLights.getValue()));
  int MasterId = -1;
  sscanf(MasterPisteId.getValue(),"%d",&MasterId);
  mypreferences.putInt("MasterPiste",MasterId);

  mypreferences.end();
  ESP.restart();
}

void ConfigPortalTimeoutCallback()
{
  ESP.restart();
}
void ConfigResetCallback()
{
  ESP_LOGI(NETWORK_TAG, "%s","In ConfigResetCallback");
}
char temp[2];
char *BoolToStr(bool value){
  sprintf(temp,"N");
  if(value)
    sprintf(temp,"Y");
  return temp;
}

void NetWork::WaitForNewSettingsViaPortal()
{
  ESP_LOGI(NETWORK_TAG, "%s","In WaitForNewSettingsViaPortal()");
  ESP_LOGI(NETWORK_TAG, "%s",soft_ap_ssid);

  networkpreferences.begin("credentials", false);
  int32_t PisteNr = networkpreferences.getInt("pisteNr", -1);
  char temp[8];
  sprintf(temp,"%d",PisteNr);
  WiFiPistId.setValue(temp, 8);

  String soft_ap_password = networkpreferences.getString("AP_Password", "01041967");
  WiFiAPPasswd.setValue(soft_ap_password.c_str(),64);

  uint16_t CyranoPortNr = networkpreferences.getUShort("CyranoPort", 50100);
  sprintf(temp,"%d",CyranoPortNr);
  CyranoPort.setValue(temp,8);

  uint16_t CyranoBroadcastPortNr = networkpreferences.getUShort("CyranoBcPort", 50100);
  sprintf(temp,"%d",CyranoBroadcastPortNr);
  CyranoBroadcastPort.setValue(temp,8);

  TryGlobalWiFi.setValue(BoolToStr(networkpreferences.getBool("TryGlobalWiFi",false)),1);
  UseDHCP.setValue(BoolToStr(networkpreferences.getBool("UseDHCP",false)),1);
  FixedIPAddress.setValue((networkpreferences.getString("BaseAddress","172.20.255.1")).c_str(),16);

  networkpreferences.end();

  Preferences mypreferences;
  mypreferences.begin("scoringdevice", false);

  uint8_t storedweapon = mypreferences.getUChar("START_WEAPON",99);

  switch (storedweapon) {
    case 0:
    sprintf(temp,"F");
    break;

    case 1:
    sprintf(temp,"E");
    break;

    case 2:
    sprintf(temp,"S");
    break;
    default:
    sprintf(temp,"E");

  }
  StartUpWeapon.setValue(temp,1);

  PowerMode.setValue(BoolToStr(mypreferences.getBool("Powersave",false)),1);
  RepeaterMode.setValue(BoolToStr(mypreferences.getBool("RepeaterMode",false)),1);

  MirrorLights.setValue(BoolToStr(mypreferences.getBool("MirrorLights",false)),1);

  int32_t MasterNr = mypreferences.getInt("MasterPiste", -1);
  sprintf(temp,"%d",MasterNr);
  MasterPisteId.setValue(temp, 8);
  mypreferences.end();

  server.end();

  wm.addParameter(&WiFiPistId);
  wm.addParameter(&WiFiAPPasswd);

  wm.addParameter(&TryGlobalWiFi);
  wm.addParameter(&CyranoPort);
  wm.addParameter(&CyranoBroadcastPort);
  wm.addParameter(&UseDHCP);
  wm.addParameter(&FixedIPAddress);
  wm.addParameter(&StartUpWeapon);
  wm.addParameter(&PowerMode);
  wm.addParameter(&RepeaterMode);
  wm.addParameter(&MasterPisteId);
  wm.addParameter(&MirrorLights);

  wm.setEnableConfigPortal(true);
  wm.setConfigPortalBlocking(true);
  wm.setConfigPortalTimeout(120);
  wm.setSaveParamsCallback(saveParamsCallback);
  wm.setConfigPortalTimeoutCallback(ConfigPortalTimeoutCallback);
  wm.setConfigResetCallback(ConfigResetCallback);
  wm.setShowInfoUpdate(false);
  wm.setParamsPage(true);
  wm.startConfigPortal(soft_ap_ssid.c_str(), soft_ap_password.c_str());
  //ESP.restart();
  m_GlobalWifiStarted = false;
  NetWork::GlobalStartWiFi();
}

void NetWork::DoFactoryReset(){
  Preferences networkpreferences;
  networkpreferences.begin("credentials", false);
  networkpreferences.clear();
  networkpreferences.end();

  Preferences mypreferences;
  mypreferences.begin("scoringdevice", false);
  mypreferences.clear();
  mypreferences.end();
  nvs_flash_erase(); // erase the NVS partition and...
  nvs_flash_init(); // initialize the NVS partition.
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
