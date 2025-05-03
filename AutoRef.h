//Copyright (c) Piet Wauters 2025 <piet.wauters@gmail.com>
#ifndef AUTOREF_H
#define AUTOREF_H
#include "SubjectObserverTemplate.h"
#include "Singleton.h"
#include "3WeaponSensor.h"
#include "WS2812BLedStrip.h"
#include "FencingStateMachine.h"


enum RefereeState_t {IDLE,STARTING, ENDING, WATCHING, WAITING_FOR_CONFIRMATION};

class UDPIOHandler;
class FencingStateMachine;

class AutoRef :  public Subject<AutoRef> ,public Observer<FencingStateMachine>, public SingletonMixin<AutoRef>
{
    public:

        /** Default destructor */
        virtual ~AutoRef();
        void StateChanged (uint32_t eventtype) {notify(eventtype);}
        void begin();
        void static AutoRefHandler(void *parameter);
        void update (FencingStateMachine *subject, uint32_t eventtype);


    protected:

    private:
    // private methods
    friend class SingletonMixin<AutoRef>;
    /** Default constructor */
    AutoRef();
    // private member variables
    bool m_HasBegun = false;
    QueueHandle_t AutoRefqueue = NULL;
    RefereeState_t m_RefState = IDLE;
    long timesincetimertoggle;


};

#endif // AUTOREF_H
