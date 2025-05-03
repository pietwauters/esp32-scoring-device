//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#ifndef NETWORK_H
#define NETWORK_H
#include "SubjectObserverTemplate.h"
#include "UDPIOHandler.h"
#include "EventDefinitions.h"
#include <Preferences.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
//#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
// It seems we should not use channels above 11
#define CHANNEL_COUNT  12
#define VERY_STRONG -51
#define STRONG -61
#define WEAK -71

#define RW_MODE false
#define RO_MODE true


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


class NetWork : public Observer<UDPIOHandler>, public SingletonMixin<NetWork>
{
    public:

        /** Default destructor */
        virtual ~NetWork();
        int findBestWifiChannel();
        void GlobalStartWiFi();
        bool ConnectToExternalNetwork(long ConnectTimeout = 5);
        void reset_channels();
        int32_t FindFirstFreePisteID(uint32_t RequestedPiste = 0);
        void WaitForNewSettingsViaPortal();
        void update (UDPIOHandler *subject, uint32_t eventtype);
        bool IsExternalWifiAvailable(){return bConnectedToExternalNetwork;}
        void FindAndSetMasterChannel(int soft_retries=5, bool restart_on_timeout=false);
        int begin();
        void DoFactoryReset();

    protected:

    private:
      friend class SingletonMixin<NetWork>;
      /** Default constructor */
      NetWork();
    bool m_GlobalWifiStarted = false;
    bool bConnectedToExternalNetwork = false;
    char LocalIPAddress[16] = "255.255.255.255";
    char SoftAPIPAddress[16] = "255.255.255.255";
    Preferences networkpreferences;
    String ssid;
    String password;
    String soft_ap_ssid = "Piste_0";
    String soft_ap_password = "01041967";
    channel_t channels[CHANNEL_COUNT]; // Array with all the channels
    int networks = -1;
    bool SavedNetworkExists = false;
    bool LookForExternalWiFi = false;
    int bestchannel = -1;
    //AsyncWebServer server={80};
    WiFiManager wm;
};
#endif //NETWORK_H
