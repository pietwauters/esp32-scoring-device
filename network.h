//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#ifndef NETWORK_H
#define NETWORK_H
#include <Preferences.h>

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


class NetWork
{
    public:
        /** Default constructor */
        NetWork();
        /** Default destructor */
        virtual ~NetWork();
        int findBestWifiChannel();
        void GlobalStartWiFi();
        void reset_channels();
        int32_t FindFirstFreePisteID(uint32_t RequestedPiste = 0);
    protected:

    private:
    bool m_GlobalWifiStarted = false;
    char LocalIPAddress[16] = "255.255.255.255";
    char SoftAPIPAddress[16] = "255.255.255.255";
    Preferences networkpreferences;
    String ssid;
    String password;
    String soft_ap_ssid = "Piste_0";
    String soft_ap_password = "01041967";
    channel_t channels[CHANNEL_COUNT]; // Array with all the channels
};
#endif //NETWORK_H
