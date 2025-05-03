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
#include "RS422_FPA_Type10_Message.h"
#include "RS422_FPA_Type20_Message.h"
#include "SubjectObserverTemplate.h"
#include "FencingStateMachine.h"
#include "EventDefinitions.h"
#include "network.h"


#define MAX_MESSAGE_TYPE 10
//#define ALLOW_BLE
//#define ALLOW_BLUETOOTH
//#define ALLOW_HARDWARESERIAL

class CyranoHandler;
class FencingStateMachine;

class FPA422Handler : public Observer<FencingStateMachine> , public Observer<CyranoHandler>
{
    public:
        /** Default constructor */
        FPA422Handler();
        /** Default destructor */
        virtual ~FPA422Handler();
        void update (FencingStateMachine *subject, uint32_t eventtype);
        void update (CyranoHandler *subject, std::string strEFP1Message);
        void update (CyranoHandler *subject, uint32_t eventtype);
        void ProcessLightsChange(uint32_t eventtype);
#ifdef ALLOW_BLUETOOTH
        void StartBluetooth();
        void BTTPeriodicalUpdate();
        void BTTransmitMessage(int Type);
#endif
#ifdef ALLOW_BLE

#endif
        void StartWiFi();
#ifdef ALLOW_HARDWARESERIAL
        void StartHWSerial();
#endif
        void WifiTransmitMessage(int Type);
        void AllProtocolsTransmitMessage(int Type);
        void WifiPeriodicalUpdate();
        void SetCyranoStatus(char TheState){Message10.SetCyranoStatus(TheState);};

    protected:

    private:
    RS422_FPA_Type1_Message Message1;
    RS422_FPA_Type2_Message Message2;
    RS422_FPA_Type3_Message Message3;
    RS422_FPA_Type4_Message Message4;
    RS422_FPA_Type5_6_Message Message5;
    RS422_FPA_Type5_6_Message Message6;
    //RS422_FPA_Type7_Message Message7;
    RS422_FPA_Type8_Message Message8;
    RS422_FPA_Type10_Message Message10;
    RS422_FPA_Type20_Message Message11;

    RS422_FPA_Message *Meassages[10]={&Message1,&Message2,&Message3,&Message4,&Message5,&Message6,&Message1,&Message8,&Message1,&Message10};


    uint8_t m_minutes;
    uint8_t m_seconds;
    uint8_t m_hundredths;
    int previous_seconds;
    bool m_BlueToothStarted = false;
    bool m_WifiStarted = false;
    bool m_HWSerialStarted = false;
    long TimeForNext1_2s = 0;
    long TimeForNext12s = 0;
    int UDPPort = 50112;
    int PisteNr = 1;
    int AnnouncingPort = 65534;
    char LocalIPAddress[16] = "255.255.255.255";
    char SoftAPIPAddress[16] = "255.255.255.255";
    int m_WifiPeriodicalUpdateCounter = 0;
    int m_SlowWifiPeriodicalUpdateCounter = 0;

};

#endif // FPA422HANDLER_H
