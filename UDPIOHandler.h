//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#ifndef UDPIOHANDLER_H
#define UDPIOHANDLER_H
#include "SubjectObserverTemplate.h"
#include "FencingStateMachine.h"

#include <WiFi.h>
#include <WiFiClient.h>


#define UDP_GREEN     "#23C48E"
#define UDP_BLUE      "#04C0F8"
#define UDP_YELLOW    "#ED9D00"
#define UDP_RED       "#D3435C"
#define UDP_DARK_BLUE "#5F7CD8"

#include "EventDefinitions.h"




class UDPIOHandler : public Subject<UDPIOHandler> , public Observer<FencingStateMachine>
{
    public:
        /** Default constructor */
        UDPIOHandler();
        /** Default destructor */
        virtual ~UDPIOHandler();

        void update (FencingStateMachine *subject, uint32_t eventtype);

        void InputChanged(uint32_t eventtype = EVENT_UI_INPUT) {notify(eventtype);}

        void run();
        void ConnectToAP();

        void Start();
        

    protected:

    private:
    
    void ProcessLightsChange(uint32_t eventtype);
    int m_minutes;
    int m_seconds;
    int m_hundredths;
    int previous_seconds;
    
};

extern UDPIOHandler MyUDPIOHandler;

#endif // UDPIOHANDLER_H
