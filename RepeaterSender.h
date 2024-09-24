//Copyright (c) Piet Wauters 2024 <piet.wauters@gmail.com>
#ifndef REPEATERSENDER
#define REPEATERSENDER
#include "SubjectObserverTemplate.h"
#include "FencingStateMachine.h"
#include "RepeaterDefs.h"
#include <esp_now.h>

#define HEART_RATE  61
class FencingStateMachine;
class RepeaterSender : public Observer<FencingStateMachine>
{
    public:
        /** Default constructor */
        RepeaterSender();  // tickPeriod in miliseconds
        /** Default destructor */
        virtual ~RepeaterSender();
        void update (FencingStateMachine *subject, uint32_t eventtype);
        void RegisterRepeater(uint8_t *broadcastAddress);
        void BroadcastHeartBeat();

        void begin();

    protected:

    private:
    // private methods
    esp_now_peer_info_t peerInfo;
    struct_message m_message;
    uint8_t m_receiverAddress[6] = {0x24,0xDC,0xC3,0x45,0xCD,0xA0};
    uint8_t m_broadcastAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint32_t m_HeartbeatCounter = 0;
    long TimeToNextHeartbeat = 0;
    long HeartPeriod = 60000/HEART_RATE;



    // private member variables


};

#endif // REPEATERSENDER
