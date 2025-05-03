//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#ifndef FENCINGSTATEMACHINE_H
#define FENCINGSTATEMACHINE_H
#include "SubjectObserverTemplate.h"
#include "Singleton.h"
#include "3WeaponSensor.h"
#include "FencingTimer.h"
#include "UW2FTimer.h"
#include "EFP1Message.h"
#include "RepeaterSender.h"



enum Priority_t {NO_PRIO, PRIO_LEFT, PRIO_RIGHT};
enum TimerState_t{FIGHTING, BREAK, INJURY, ADDITIONAL_MINUTE, MATCH_ENDED,UNDEFINED};
enum UI_State_t {LOCKED, UNLOCKED};
//enum weapon_t{FOIL, EPEE, SABRE, UNKNOWN};
//enum WeaponSelectionMode_t {MANUAL, AUTO, HYBRID};


class UDPIOHandler;
class CyranoHandler;
class FencingStateMachine : public Subject<FencingStateMachine> , public Observer<MultiWeaponSensor> ,  public Observer<UDPIOHandler> ,  public Observer<CyranoHandler>, public SingletonMixin<FencingStateMachine>
{
    public:

        /** Default destructor */
        virtual ~FencingStateMachine();


        //void StateChanged () {notify();}
        void DoStateMachineTick();
        void update (MultiWeaponSensor *subject, uint32_t eventtype);
 //       void update (BlynkIOHandler *subject, uint32_t eventtype);
        void update (UDPIOHandler *subject, uint32_t eventtype);
        void update (CyranoHandler *subject, uint32_t eventtype);
        void update (CyranoHandler *subject, std::string eventtype);
        void TransmitFullStateToDisplay (class RepeaterSender *TheRepeater);
        void StateChanged (uint32_t eventtype) {notify(eventtype);}
        void ProcessDisplayMessage (const EFP1Message &input);
        void PeriodicallyBroadcastFullState(class RepeaterSender *TheRepeater,long Period = 7919);

        /** Access m_Red
         * \return The current value of m_Red
         */
        bool GetRed() { return m_Lights & MASK_RED; }
        /** Set m_Red
         * \param val New value to set
         */
        void SetRed(bool val) { if(val != m_Red){m_Red = val; m_StateChanged = true;}}
        /** Access m_Green
         * \return The current value of m_Green
         */
        bool GetGreen() { return m_Lights & MASK_GREEN; }
        /** Set m_Green
         * \param val New value to set
         */
        void SetGreen(bool val) { if(val != m_Green){m_Green = val; m_StateChanged = true;}}
        /** Access m_WhiteL
         * \return The current value of m_WhiteL
         */
        bool GetWhiteL() { return m_Lights & MASK_WHITE_L; }
        /** Set m_WhiteL
         * \param val New value to set
         */
        void SetWhiteL(bool val) { if(val != m_WhiteL){m_WhiteL = val; m_StateChanged = true;}}

        /** Access m_WhiteR
         * \return The current value of m_WhiteL
         */
        bool GetWhiteR() { return m_Lights & MASK_WHITE_R; }

        /** Access m_OrangeL
         * \return The current value of m_OrangeL
         */
        bool GetOrangeL() { return m_Lights & MASK_ORANGE_L; }
        /** Set m_OrangeL
         * \param val New value to set
         */
        void SetOrangeL(bool val) { if(val != m_OrangeL){m_OrangeL = val; m_StateChanged = true;}}
        /** Access m_OrangeR
         * \return The current value of m_OrangeR
         */
        bool GetOrangeR() { return m_Lights & MASK_ORANGE_R; }
        /** Set m_OrangeR
         * \param val New value to set
         */
        void SetOrangeR(bool val) { if(val != m_OrangeR){m_OrangeR = val; m_StateChanged = true;}}
        /** Access m_Priority&
         * \return The current value of m_Priority
         */
        Priority_t GetPriority() { return m_Priority; }
        /** Set m_Priority
         * \param val New value to set
         */
        void SetPriority(Priority_t val) { if(val != m_Priority){m_Priority = val; m_StateChanged = true;}}
        /** Access m_YellowCardLeft
         * \return The current value of m_YellowCardLeft
         */
        int GetYellowCardLeft() { return m_YellowCardLeft; }
        /** Set m_YellowCardLeft
         * \param val New value to set
         */
        void SetYellowCardLeft(int val) { if(val != m_YellowCardLeft){m_YellowCardLeft = val; m_StateChanged = true;}}
        /** Access m_YellowCardRight
         * \return The current value of m_YellowCardRight
         */
        int GetYellowCardRight() { return m_YellowCardRight; }
        /** Set m_YellowCardRight
         * \param val New value to set
         */
        void SetYellowCardRight(int val) {if(val != m_YellowCardRight){ m_YellowCardRight = val; m_StateChanged = true;}}
        /** Access m_RedCardLeft
         * \return The current value of m_RedCardLeft
         */
        int GetRedCardLeft() { return m_RedCardLeft; }
        /** Set m_RedCardLeft
         * \param val New value to set
         */
        void SetRedCardLeft(int val) { if(val != m_RedCardLeft){m_RedCardLeft = val; m_StateChanged = true;}}
        /** Access m_RedCardRightm_timeToRearm
         * \return The current value of m_RedCardRight
         */
        int GetRedCardRight() { return m_RedCardRight; }
        /** Set m_RedCardRight
         * \param val New value to set
         */
        void SetRedCardRight(int val) { if(val != m_RedCardRight){m_RedCardRight = val; m_StateChanged = true;}}
        /** Access m_Timerstate
         * \return The current value of m_Timerstate
         */
        TimerState_t GetTimerstate() { return m_Timerstate; }
        /** Set m_Timerstate
         * \param val New value to set
         */
        void SetTimerstate(TimerState_t val) { if(val != m_Timerstate){m_Timerstate = val; m_StateChanged = true;}}
        /** Access m_ScoreLeft
         * \return The current value of m_ScoreLeft
         */
        unsigned int GetScoreLeft() { return m_ScoreLeft; }
        /** Set m_ScoreLeft
         * \param val New value to set
         */
        void SetScoreLeft(unsigned int val) { if(val != m_ScoreLeft){m_ScoreLeft = val; m_StateChanged = true;}}
        /** Access m_ScoreRight
         * \return The current value of m_ScoreRight
         */
        unsigned int GetScoreRight() { return m_ScoreRight; }
        /** Set m_ScoreRight
         * \param val New value to set
         */
        void SetScoreRight(unsigned int val) { if(val != m_ScoreRight){m_ScoreRight = val; m_StateChanged = true;}}
        /** Access m_MachineWeapon
         * \return The current value of m_MachineWeapon
         */
        weapon_t GetMachineWeapon() { return m_MachineWeapon; }
        /** Set m_MachineWeapon
         * \param val New value to set
         */
        void SetMachineWeapon(weapon_t val) { if(val != m_MachineWeapon){m_MachineWeapon = val; m_WeaponChanged = true;}}
        void SetMachineLights(unsigned char val) { if(val != m_Lights){m_Lights = val; m_LightsChanged = true; }}
        void RegisterMultiWeaponSensor(MultiWeaponSensor *MySensor){m_TheSensor = MySensor;SetMachineWeapon(MySensor->GetActualWeapon());}
        void ResetAll();
        uint32_t MakeTimerEvent();
        void GetFormattedStringTime(char *Destination, int MinutePrecision, int HundredthsPrecision){m_Timer.GetFormattedStringTime(Destination, MinutePrecision, HundredthsPrecision);};
        bool IsConnectedToRemote(){return m_IsConnectedToRemote;};
        void SetConnectedToRemote(bool value){m_IsConnectedToRemote = value;};
        void ClearAllCards(bool bIncludePCards = true);
        void ProcessSpecialSetting (uint32_t eventtype);
        bool incrementScoreAndCheckForMinuteBreak(bool bLeftFencer);
        uint32_t get_max_score();
        bool GoToSleep() {return m_GoToSleep;};
        void begin();

    protected:

    private:
    // private methods
    /** Default constructor */
    friend class SingletonMixin<FencingStateMachine>;
      FencingStateMachine(int hw_timer_nr=2, int tickPeriod=10);  // tickPeriod in miliseconds
      bool ShouldBlockHitsOnTimerZero();
      void SetNextTimerStateAndRoundAndNewTimeOnTimerZero();
      void ProcessUW2F();
      void ProcessUW2FUndo();

    // private member variables
        bool m_Red; //!< Member variable "m_Red"
        bool m_Green; //!< Member variable "m_Green"
        bool m_WhiteL; //!< Member variable "m_WhiteL"
        bool m_OrangeL; //!< Member variable "m_OrangeL"
        bool m_OrangeR; //!< Member variable "m_OrangeR"
        bool m_StateChanged;
        Priority_t m_Priority; //!< Member variable "m_Priority"
        int m_YellowCardLeft; //!< Member variable "m_YellowCardLeft"
        int m_YellowCardRight; //!< Member variable "m_YellowCardRight"
        int m_RedCardLeft; //!< Member variable "m_RedCardLeft"
        int m_RedCardRight; //!< Member variable "m_RedCardRight"
        int m_BlackCardLeft; //!< Member variable "m_RedCardLeft"
        int m_BlackCardRight; //!< Member variable "m_RedCardRight"
        int m_YellowPCardLeft; //!< Member variable "m_YellowCardLeft"
        int m_YellowPCardRight; //!< Member variable "m_YellowCardRight"
        int m_RedPCardLeft; //!< Member variable "m_RedCardLeft"
        int m_RedPCardRight; //!< Member variable "m_RedCardRight"
        int m_BlackPCardLeft; //!< Member variable "m_RedCardLeft"
        int m_BlackPCardRight; //!< Member variable "m_RedCardRight"
        int m_PCardLeft = 0;
        int m_PCardRight = 0;

        unsigned int m_ScoreLeft; //!< Member variable "m_ScoreLeft"
        unsigned int m_ScoreRight; //!< Member variable "m_ScoreRight"
        weapon_t m_MachineWeapon; //!< Member variable "m_MachineWeapon"
        bool m_WeaponChanged = true;  //!< Member variable "m_StateChanged"
        unsigned char m_Lights = 0;
        bool m_LightsChanged = true;  //!< Member variable "m_LightsChanged"
        TimerState_t m_Timerstate; //!< Member variable "m_Timerstate"
        uint32_t m_timeToRearm = 0;

        int m_currentRound=1;
        int m_nrOfRounds=1;
        FencingTimer m_Timer;
        UW2FTimer m_UW2FTimer;
        long m_UW2FSeconds = 0;
        bool m_NoHitsAllowed = false;
        hw_timer_t * timer_FSMPeriod = NULL;
        MultiWeaponSensor *m_TheSensor = NULL;
        long m_NextIdleTime = 0;
        bool m_IsConnectedToRemote = false;

        long m_TimeToBroadcastFullState = 0;
        int m_low_prio_divider = 5;
        int m_low_prio_divider_init = 7;

        bool m_GlobalIdle = false;
        long m_LastLightEventTime = 0;
        bool m_GoToSleep = false;
        bool m_HasBegun = false;
        UI_State_t m_UI_State = UNLOCKED;


};

#endif // FENCINGSTATEMACHINE_H
