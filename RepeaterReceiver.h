//Copyright (c) Piet Wauters 2024 <piet.wauters@gmail.com>
#ifndef REPEATERRECEIVER
#define REPEATERRECEIVER
#include "SubjectObserverTemplate.h"
#include "RepeaterDefs.h"
#include <esp_now.h>





class RepeaterReceiver : public Subject<RepeaterReceiver>
{
    public:
        /** Default constructor */
        RepeaterReceiver();  // tickPeriod in miliseconds
        /** Default destructor */
        virtual ~RepeaterReceiver();
        void StateChanged (uint32_t eventtype);
        void RegisterRepeater(uint8_t *broadcastAddress);
        void begin(esp_now_recv_cb_t theCallBack);
        int32_t MasterPiste(){return m_MasterPiste;};
        void StartWatchDog(long Period = FULL_STATUS_REPETITION_PERIOD*3);
        void ResetWatchDog();
        bool IsWatchDogTriggered();
        bool Mirror(){return m_Mirror;};

    protected:

    private:
    // private methods
    int m_espnowchannel = -1;
    int32_t m_MasterPiste =-1;
    bool m_Mirror = false;
    uint8_t m_senderAddress[6] = {0x24,0xDC,0xC3,0x45,0xCD,0xA0};
    uint8_t m_broadcastAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    esp_now_peer_info_t peerInfo;
    long m_WatchDogTriggerTime = 999999;
    long m_WatchDogPeriod =  FULL_STATUS_REPETITION_PERIOD * 3;


    // private member variables


};

#endif // REPEATERRECEIVER
