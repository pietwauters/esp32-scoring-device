//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#ifndef WEAPONSENSOR_H
#define WEAPONSENSOR_H
#include "SubjectObserverTemplate.h"
#include "Singleton.h"
#include <Arduino.h>
#include "TimingConstants.h"
#include <cstdio>
#include <cinttypes>
#include <cstddef>
#include "weaponenum.h"
#include "hardwaredefinition.h"

// If you have different pins, change below defines
/*#define cl_analog 36
#define bl_analog 39
#define piste_analog 34
#define cr_analog 35
#define br_analog 32*/
// Below table uses AD channels and not pin numbers
#define cl_analog 0
#define bl_analog 3
#define piste_analog 6
#define cr_analog 7
#define br_analog 4

#ifdef FIRST_PROTO
#define al_driver 22
#define bl_driver 21
#define cl_driver 23
#define ar_driver 05
#define br_driver 04
#define cr_driver 18
#define piste_driver 19
#endif

#ifdef SECOND_PROTO
#define al_driver 33
#define bl_driver 21
#define cl_driver 23
#define ar_driver 25
#define br_driver 05
#define cr_driver 18
#define piste_driver 19
#endif

// below defines are generated with the excel tool

#define IODirection_ar_br 231
#define IODirection_ar_cr 215
#define IODirection_ar_piste 183
#define IODirection_ar_bl 245
#define IODirection_ar_cl 243
#define IODirection_al_br 238
#define IODirection_al_cr 222
#define IODirection_al_piste 190
#define IODirection_al_bl 252
#define IODirection_al_cl 250
#define IODirection_br_cr 207
#define IODirection_br_bl 237
#define IODirection_br_cl 235
#define IODirection_br_piste 175
#define IODirection_bl_cl 249
#define IODirection_bl_piste 189
#define IODirection_bl_cr 221
#define IODirection_cr_piste 159
#define IODirection_cr_cl 219
#define IODirection_cl_piste 187


#define IOValues_ar_br 8
#define IOValues_ar_cr 8
#define IOValues_ar_piste 8
#define IOValues_ar_bl 8
#define IOValues_ar_cl 8
#define IOValues_al_br 1
#define IOValues_al_cr 1
#define IOValues_al_piste 1
#define IOValues_al_bl 1
#define IOValues_al_cl 1
#define IOValues_br_cr 16
#define IOValues_br_bl 16
#define IOValues_br_cl 16
#define IOValues_br_piste 16
#define IOValues_bl_cl 2
#define IOValues_bl_piste 2
#define IOValues_bl_cr 2
#define IOValues_cr_piste 32
#define IOValues_cr_cl 32
#define IOValues_cl_piste 4

#define MASK_RED 0x80
#define MASK_WHITE_L 0x40
#define MASK_ORANGE_L 0x20
#define MASK_ORANGE_R 0x10
#define MASK_WHITE_R 0x08
#define MASK_GREEN 0x04
#define MASK_BUZZ 0x02
#define LIGHTS_DURATION_MS 2000

//enum weapon_t {FOIL, EPEE, SABRE, UNKNOWN};
enum weapon_detection_mode_t{MANUAL,AUTO, HYBRID};

typedef struct MeasurementCtlStruct
{
    uint8_t IODirection;
    uint8_t IOValues;
    uint8_t ADChannel;
    int ADThreashold;
} MeasurementCtl;

class MultiWeaponSensor  : public Subject<MultiWeaponSensor>, public SingletonMixin<MultiWeaponSensor>
{
public:

    void begin();
    /** Default destructor */
    virtual ~MultiWeaponSensor();

    void SensorStateChanged (uint32_t eventtype) {notify(eventtype);}

    /** Access m_ActualWeapon
     * \return The current value of m_ActualWeapon
     */
    weapon_t GetActualWeapon()
    {
        return m_ActualWeapon;
    }
    /** Set m_ActualWeapon
     * \param val New value to set
     */
    void SetActualWeapon(weapon_t val)
    {
        m_ActualWeapon = val;
        DoReset();
    }
    /** Access m_DetectedWeapon
     * \return The current value of m_DetectedWeapon
     */
    weapon_t GetDetectedWeapon()
    {
        return m_DetectedWeapon;
    }

void DoSabre();
void DoEpee(void);
void DoFoil(void);
void Skip_phase();
    void DoFullScan();
    bool Wait_For_Next_Timer_Tick();
    unsigned char get_Lights(){return Lights;};
    void BlockAllNewHits(){SignalLeft = true; SignalRight = true;};
    void AllowAllNewHits(){SignalLeft = false; SignalRight = false;};
    void Setweapon_detection_mode(weapon_detection_mode_t mode){m_DectionMode = mode;};

protected:

private:
    friend class SingletonMixin<MultiWeaponSensor>;
    /** Default constructor */
    MultiWeaponSensor();
    bool Do_Common_Start();
    //void Skip_phase();
    void HandleLights();
    //void DoEpee(void);
    //void DoFoil(void);
    //void DoSabre();
    void DoReset(void);
    void StartLock(int TimeToLock);
    bool IsLocked();
    bool OKtoReset();
    weapon_t GetWeapon();
    int tempADValue = 0;


    hw_timer_t * timer = NULL;


    weapon_t m_ActualWeapon = EPEE; //!< Member variable "m_ActualWeapon"
    weapon_t m_DetectedWeapon = EPEE; //!< Member variable "m_DetectedWeapon"
    weapon_detection_mode_t m_DectionMode = AUTO;
    bool m_NoNewHitsAllowed = false;



    bool SignalLeft;
    bool SignalRight;
    bool WaitingForResetStarted;
    bool NotConnectedLeft;
    bool NotConnectedRight;
    bool bAutoDetect;
    bool bPreventBuzzer = false;
    int FullScanCounter =1;

    // below are the counters used for "debouncing"
    // "normal" counters are used for contact duration of hits
    int Counter_b1;
    int Counter_b2;
    int Counter_c1;
    int Counter_c2;
    // below const values are used to make timing calibration possible without having constants in the code
    int Const_COUNT_B1_INIT_FOIL;
    int Const_COUNT_C1_INIT_FOIL;
    int Const_COUNT_Cx_INIT_EPEE;
    int Const_COUNT_C1_INIT_SABRE;
    int Const_FOIL_LOCK_TIME;
    int Const_EPEE_LOCK_TIME;
    int Const_SABRE_LOCK_TIME;
    int Const_FOIL_PARRY_ON_TIME;
    int Const_FOIL_PARRY_OFF_TIME;

    // "long" counters are used for automatic weapon detection
    int LongCounter_b1;
    int LongCounter_b2;
    int LongCounter_c1;
    int LongCounter_c2;
    // counters introduced for automatic switch to epee if no foil or sabre connected

    int LongCounter_NotConnected;
    int LongCounter_AtLeastOneNotConnected;



    unsigned char Lights;

    int BlockCounter;
    bool MaybeSignalRight;
    bool MaybeSignalLeft;
    bool BlockWhipover;
    bool WeaponContact;


    bool Red ;
    bool  WhiteL;
    bool  OrangeR;
    bool Green ;
    bool  WhiteR;
    bool  OrangeL;
    bool  Buzz;
    bool  b1_reached1;
    bool  b2_reached1;
    bool  c1_reached1;
    bool  c2_reached1;
    bool  PossiblyRed;
    bool  PossiblyGreen;
    bool  WeHaveBlockedAhit;
    bool  bParrySignal;
    int BlockedAHitCounter;
    int Counter_parry;


    MeasurementCtl *Set;
    int TimeOfLock;
    bool LockStarted;
    int TimeToReset;
    int LightsDuration = LIGHTS_DURATION_MS;
    int64_t TimetoNextPhase = 0;
    int CurrentPhaseDuration = FULLSCANDURATION_EPEE;
    SemaphoreHandle_t  SensorMutex;
    uint32_t ShortIndicatorsDebouncer = 0;


};

extern void prepareforDeepSleep();

#endif // WEAPONSENSOR_H
