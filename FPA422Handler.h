//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#ifndef FPA422HANDLER_H
#define FPA422HANDLER_H
#include "RS422_FPA_Type2_Message.h"
#include "RS422_FPA_Type3_Message.h"
#include "RS422_FPA_Message.h"
#include "RS422_FPA_Type4_Message.h"
#include "RS422_FPA_Type1_Message.h"
#include "RS422_FPA_Type5_Message.h"
#include "RS422_FPA_Type8_Message.h"
#include "SubjectObserverTemplate.h"
#include "FencingStateMachine.h"
#include "EventDefinitions.h"

#define MAX_MESSAGE_TYPE 9

class FPA422Handler : public Observer<FencingStateMachine>
{
    public:
        /** Default constructor */
        FPA422Handler();
        /** Default destructor */
        virtual ~FPA422Handler();
        void update (FencingStateMachine *subject, uint32_t eventtype);
        void ProcessLightsChange(uint32_t eventtype);
        void StartBluetooth();
        void StartWiFi();
        void StartHWSerial();
        void BTTransmitMessage(int Type);
        void WifiTransmitMessage(int Type);
        void BTTPeriodicalUpdate();
        void WifiPeriodicalUpdate();

    protected:

    private:
    RS422_FPA_Type1_Message Message1;
    RS422_FPA_Type2_Message Message2;
    RS422_FPA_Type3_Message Message3;
    RS422_FPA_Type4_Message Message4;
    RS422_FPA_Type8_Message Message8;

    uint8_t m_minutes;
    uint8_t m_seconds;
    uint8_t m_hundredths;
    int previous_seconds;
    bool m_BlueToothStarted = false;
    bool m_WifiStarted = false;
    bool m_HWSerialStarted = false;
    int TimeForNext1_2s = 0;
    int TimeForNext12s = 0;
    int UDPPort = 50112;
    int PisteNr = 1;
    int AnnouncingPort = 65534;
    char LocalIPAddress[16] = "255.255.255.255";
    char SoftAPIPAddress[16] = "255.255.255.255";
};

#endif // FPA422HANDLER_H
