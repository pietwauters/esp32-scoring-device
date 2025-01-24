//Copyright (c) Piet Wauters 2024 <piet.wauters@gmail.com>
#ifndef REPEATERSENDER
#define REPEATERSENDER
#include "SubjectObserverTemplate.h"
#include "FencingStateMachine.h"
#include "RepeaterDefs.h"
#include <esp_now.h>

#define HEART_RATE  61
class FencingStateMachine;
class RepeaterSender : public Observer<FencingStateMachine>, public SingletonMixin<RepeaterSender>
{
    public:

        /** Default destructor */
        virtual ~RepeaterSender();
        void update (FencingStateMachine *subject, uint32_t eventtype);
        void RegisterRepeater(uint8_t *broadcastAddress);
        void BroadcastHeartBeat();
        void RepeatLastMessage();
        void begin();

    protected:

    private:
    // private methods
    friend class SingletonMixin<RepeaterSender>;
    /** Default constructor */
    RepeaterSender();  // tickPeriod in miliseconds
    esp_now_peer_info_t peerInfo;
    struct_message m_message;
    uint8_t m_receiverAddress[6] = {0x24,0xDC,0xC3,0x45,0xCD,0xA0};
    uint8_t m_broadcastAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint32_t m_HeartbeatCounter = 0;
    long TimeToNextHeartbeat = 0;
    long HeartPeriod = 60000/HEART_RATE;
    long currentNewMessageCount = 0;
    int m_resendCount = MESSAGE_REPETITION_FACTOR;
    long m_nextResendTime = 0;
    uint8_t m_ResendDelta[7]={5,3,2,1,2,1,2};

    // private member variables


};

#endif // REPEATERSENDER
