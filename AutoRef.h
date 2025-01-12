//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#ifndef AUTOREF_H
#define AUTOREF_H
#include "SubjectObserverTemplate.h"
#include "3WeaponSensor.h"
#include "WS2812BLedStrip.h"
#include "FencingStateMachine.h"

enum RefereeState_t {IDLE,STARTING, ENDING, WATCHING, WAITING_FOR_CONFIRMATION};

class UDPIOHandler;

class AutoRef : public Subject<FencingStateMachine> , public Observer<MultiWeaponSensor> ,  public Observer<UDPIOHandler>
{
    public:
        /** Default constructor */
        AutoRef();  // tickPeriod in miliseconds
        /** Default destructor */
        virtual ~AutoRef();


        //void StateChanged () {notify();}

        void update (MultiWeaponSensor *subject, uint32_t eventtype);
 //       void update (BlynkIOHandler *subject, uint32_t eventtype);
        void update (UDPIOHandler *subject, uint32_t eventtype);
        void update (CyranoHandler *subject, uint32_t eventtype){};
        void update (CyranoHandler *subject, string eventtype);

        void StateChanged (uint32_t eventtype) {notify(eventtype);}



    protected:

    private:
    // private methods

    // private member variables
    RefereeState_t m_RefState = IDLE;


};

#endif // AUTOREF_H
