//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#ifndef UDPIOHANDLER_H
#define UDPIOHANDLER_H
#include "SubjectObserverTemplate.h"
#include "FencingStateMachine.h"
#include "CyranoHandler.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include "AsyncUDP.h"


#define UDP_GREEN     "#23C48E"
#define UDP_BLUE      "#04C0F8"
#define UDP_YELLOW    "#ED9D00"
#define UDP_RED       "#D3435C"
#define UDP_DARK_BLUE "#5F7CD8"

#include "EventDefinitions.h"




class UDPIOHandler : public Subject<UDPIOHandler> , public Observer<FencingStateMachine>, public SingletonMixin<UDPIOHandler>
{
    public:

        /** Default destructor */
        virtual ~UDPIOHandler();

        void update (FencingStateMachine *subject, uint32_t eventtype);
        void InputChanged(uint32_t eventtype = EVENT_UI_INPUT) {notify(eventtype);}

        void run();
        void ConnectToAP();

        void Start();
        void UDPCheck();


    protected:

    private:
    friend class SingletonMixin<UDPIOHandler>;
    /** Default constructor */
    UDPIOHandler();
    void ProcessLightsChange(uint32_t eventtype);
    int m_minutes;
    int m_seconds;
    int m_hundredths;
    int previous_seconds;
    AsyncUDP Commandudp;
};



#endif // UDPIOHANDLER_H
