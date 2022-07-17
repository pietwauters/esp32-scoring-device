#ifndef CYRANOHANDLER_H
#define CYRANOHANDLER_H
#include <iostream>
#include "EFP1Message.h"
#include "SubjectObserverTemplate.h"
#include "FencingStateMachine.h"
#include "EventDefinitions.h"


#define CYRANO_PORT 50100
enum CyranoState
{
    FENCING,
    HALT,
    PAUSE,
    ENDING,
    WAITING
};

enum EventType
{
    NextButtonPressed,
    PrevButtonPressed,
    BeginButtonPressed,
    EndButtonPressed
};

class CyranoHandler : public Observer<FencingStateMachine>
{
    public:
        /** Default constructor */
        CyranoHandler();
        /** Default destructor */
        virtual ~CyranoHandler();
        void ProcessMessageFromSoftware(const EFP1Message &input);
        void SendInfoMessage(){cout << "Sending info message" << endl; return;};
        void ProcessUIEvents(EventType const event);
        void SetPisteID(const string & ID){m_MachineStatus[PisteId]=ID;};
        void update (FencingStateMachine *subject, uint32_t eventtype);
        void ProcessLightsChange(uint32_t eventtype);

    protected:

    private:
        EFP1Message m_MachineStatus; //!< Member variable "m_Status"
        CyranoState m_State = WAITING;
        int previous_seconds =99;  // This will always result in setting a correct initial value
        uint32_t m_timeToShowTimer = 0;

};

#endif // CYRANOHANDLER_H
