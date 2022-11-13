#ifndef CYRANOHANDLER_H
#define CYRANOHANDLER_H
#include <WiFi.h>
#include "AsyncUDP.h"
#include <iostream>
#include "EFP1Message.h"
#include "SubjectObserverTemplate.h"
#include "FencingStateMachine.h"
#include "EventDefinitions.h"
#include <Preferences.h>


#define CYRANO_PORT 50100
enum CyranoState
{
    FENCING,
    HALT,
    PAUSE,
    ENDING,
    WAITING
};

/*enum EventType
{
    NextButtonPressed,
    PrevButtonPressed,
    BeginButtonPressed,
    EndButtonPressed
};*/

class UDPIOHandler;
class CyranoHandler : public Observer<FencingStateMachine> , public Observer<UDPIOHandler>, public Subject<CyranoHandler>
{
    public:
        /** Default constructor */
        CyranoHandler();
        /** Default destructor */
        virtual ~CyranoHandler();
        void ProcessMessageFromSoftware(const EFP1Message &input);
        void SendInfoMessage();
        void ProcessUIEvents(uint32_t const event);
        void SetPisteID(const string & ID){m_MachineStatus[PisteId]=ID;};
        void update (FencingStateMachine *subject, uint32_t eventtype);
        void update (UDPIOHandler *subject, uint32_t eventtype){ProcessUIEvents(eventtype);};
        void StateChanged (uint32_t eventtype) {notify(eventtype);}
        void StateChanged (string eventtype) {notify(eventtype);}
        void ProcessLightsChange(uint32_t eventtype);
        void CheckConnection();
        void Begin();

    protected:

    private:
        EFP1Message m_MachineStatus; //!< Member variable "m_Status"
        CyranoState m_State = WAITING;
        int previous_seconds =99;  // This will always result in setting a correct initial value
        uint32_t m_timeToShowTimer = 0;
        long NextTimeToCheckConnection = 0;
        bool bWifiConnected = false;
        bool bCyranoConnected = false;

        AsyncUDP CyranoHandlerudpRcv;
        bool bOKToSend = false;
        Preferences networkpreferences;
        uint16_t CyranoPort = CYRANO_PORT;

};

extern CyranoHandler MyCyranoHandler;
#endif // CYRANOHANDLER_H
