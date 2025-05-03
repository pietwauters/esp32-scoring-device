//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "3WeaponSensor.h"

//MeasurementCtl EpeeSets[]={{IODirection_al_cl,IOValues_al_cl,cl_analog,1750},{IODirection_al_piste,IOValues_al_piste,cl_analog,1750},{IODirection_al_bl,IOValues_al_bl,piste_analog,1750},{IODirection_al_br,IOValues_al_br,bl_analog,1750},{IODirection_ar_cr,IOValues_ar_cr,br_analog,1750},{IODirection_ar_piste,IOValues_ar_piste,cr_analog,1750},{IODirection_ar_br,IOValues_ar_br,piste_analog,1750},{IODirection_ar_bl,IOValues_ar_bl,br_analog,1750},{IODirection_al_cr,IOValues_al_cr,bl_analog,1750},{IODirection_ar_cl,IOValues_ar_cl,cr_analog,1750},{IODirection_al_cl,IOValues_al_cl,cl_analog,1750}};
MeasurementCtl EpeeSets[] = {{IODirection_al_piste & IODirection_al_cl, IOValues_al_cl | IOValues_al_piste, cl_analog, 1750},
  {IODirection_al_bl & IODirection_al_cl, IOValues_al_cl | IOValues_al_bl, piste_analog, 1750},
  {IODirection_al_br & IODirection_al_cl, IOValues_al_cl | IOValues_al_br, bl_analog, 1750},
  {IODirection_ar_cr, IOValues_ar_cr, br_analog, 1750},
  {IODirection_ar_piste & IODirection_ar_cr, IOValues_ar_cr | IOValues_ar_piste, cr_analog, 1750},
  {IODirection_ar_br & IODirection_ar_cr, IOValues_ar_cr | IOValues_ar_br, piste_analog, 1750},
  {IODirection_ar_bl & IODirection_ar_cr, IOValues_ar_cr | IOValues_ar_bl, br_analog, 1750},
  {IODirection_al_cr, IOValues_al_cr, bl_analog, 1750},
  {IODirection_ar_cl, IOValues_ar_cl, cr_analog, 1000},
  {IODirection_al_cl, IOValues_al_cl, cl_analog, 1000}
};


void MultiWeaponSensor::DoEpee(void)
{
static bool TempOrangeR, TempOrangeL;
static bool AdAboveThreshold;
//*************************************************************************
//    Start of epee specific code                                         *
//*************************************************************************

    //PhaseCounter =START_OF_EPEE_TABLE;
    Set = EpeeSets;
    TempOrangeL = false;
    TempOrangeR=false;
//*************************************************************************
//    Phas_E0: Test if point is pressed down: c1 =?= high                 *
//*************************************************************************

    AdAboveThreshold = Do_Common_Start();
    if (!SignalLeft)
    {
        if (AdAboveThreshold)
        {
            if(Counter_c1)
            {
                Counter_c1--;
                if(Counter_c1 == 1)
                {
                    c1_reached1 = true;
                }
            }
            else
            {
                // Counter_c1 == 0 -> this means we did not block this hit
                c1_reached1 = false;
            }
        }
        else
        {
            if(c1_reached1 == true)
            {
                // this means we have blocked a hit (by just on loop too short, so we should block the otherone too
                c1_reached1 = false;
                Const_COUNT_Cx_INIT_EPEE = COUNT_C1_INIT_EPEE +1;
                Counter_c2++;
                WeHaveBlockedAhit = true;
            }
            Counter_c1 = Const_COUNT_Cx_INIT_EPEE;
        }
    }

//*************************************************************************
//    Phas_E1: Test if point makes contact with ground: gnd =?= high       *
//*************************************************************************
    AdAboveThreshold = Do_Common_Start();
    if (!SignalLeft)
    {
        if (AdAboveThreshold)
        {
            Counter_c1 = Const_COUNT_Cx_INIT_EPEE;
            TempOrangeL = true;
        }
    }
//*************************************************************************
//    Phas_E2: Test if point makes contact with bell: b1 =?= high       *
//*************************************************************************
    AdAboveThreshold = Do_Common_Start();
    if (!SignalLeft)
    {
        if (AdAboveThreshold)
        {
            Counter_c1 = Const_COUNT_Cx_INIT_EPEE;
            TempOrangeL = true;

        }
    }

//*************************************************************************
//    Phas_E3: Test if point makes contact with weapon: b2 =?= high     *
//*************************************************************************
    AdAboveThreshold = Do_Common_Start();
    if (!SignalLeft)
    {
        if (AdAboveThreshold)
        {
            Counter_c1 = Const_COUNT_Cx_INIT_EPEE;
            TempOrangeL = true;
        }
        else
        {
            if (!Counter_c1)
            {
                Red=true;
                Buzz = true;
                SignalLeft = true;
                StartLock(EPEE_LOCK_TIME);
                Const_COUNT_Cx_INIT_EPEE = COUNT_C1_INIT_EPEE-1;
                Counter_c2--;	//here I assume Counter_c2 is not yet 0, or if it is, it will not make a difference anymore
                c1_reached1 = false;
            }
        }
    }
//*************************************************************************
//    Phas_E4: Test if point is pressed down: c1 =?= high                 *
//*************************************************************************

    AdAboveThreshold = Do_Common_Start();
    if (!SignalRight)
    {
        if (AdAboveThreshold)
        {
            if(Counter_c2)
            {
                Counter_c2--;
                if(Counter_c2 == 1)
                {
                    c2_reached1 = true;
                }
            }
            else
            {
                c2_reached1 = false;
            }
        }

        else
        {
            if(c2_reached1 == true)
            {
                // this means we have blocked a hit, so we should block the otherone too
                c2_reached1 = false;
                Const_COUNT_Cx_INIT_EPEE = COUNT_C1_INIT_EPEE +1;
                Counter_c1++;
                WeHaveBlockedAhit = true;
            }
            Counter_c2 = Const_COUNT_Cx_INIT_EPEE;
        }
    }

//*************************************************************************
//    Phas_E5: Test if point makes contact with ground: gnd =?= high       *
//*************************************************************************
    AdAboveThreshold = Do_Common_Start();
    if (!SignalRight)
    {
        if (AdAboveThreshold)
        {
            Counter_c2 = Const_COUNT_Cx_INIT_EPEE;
            TempOrangeR = true;
        }
    }
//*************************************************************************
//    Phas_E6: Test if point makes contact with bell: b2 =?= high       *
//*************************************************************************
    AdAboveThreshold = Do_Common_Start();
    if (!SignalRight)
    {
        if (AdAboveThreshold)
        {
            Counter_c2 = Const_COUNT_Cx_INIT_EPEE;
            TempOrangeR = true;
        }
    }

//*************************************************************************
//    Phas_E7: Test if point makes contact with weapon: b2 =?= high     *
//*************************************************************************
    AdAboveThreshold = Do_Common_Start();
    if (!SignalRight)
    {
        if (AdAboveThreshold)
        {
            Counter_c2 = Const_COUNT_Cx_INIT_EPEE;
            TempOrangeR = true;
        }
        else
        {
            if (!Counter_c2)
            {
                Green=true;
                Buzz = true;
                SignalRight = true;
                StartLock(EPEE_LOCK_TIME);
                Const_COUNT_Cx_INIT_EPEE = COUNT_C1_INIT_EPEE-1;
                Counter_c1--;	//here I assume Counter_c2 is not yet 0, or if it is, it will not make a difference anymore
                c2_reached1 = false;
            }
        }
    }

//*************************************************************************
//    Phas_E8: The next 2 phases are used only in club-version with
//    automatic weapon selection
//    Test if foil point pressed down on valid surface for at least 5 seconds =>
//    contact between a1-c2; in order to switch to foil, a1-b1 should not be ok
//    to switch to sabre a1-b1 should be OK
//*************************************************************************
    AdAboveThreshold = Do_Common_Start();
    if (!AdAboveThreshold)
    {
        LongCounter_c2 = LONG_COUNT_C_INIT_EPEE;
    }
    else
    {
        if(LongCounter_c2)
            LongCounter_c2--;
    }

//*************************************************************************
//    Phas_E9:
//    Test if foil point pressed down on valid surface for at least 5 seconds =>
//    contact between a2-c1; in order to switch to foil, a1-b1 should not be ok
//    to switch to sabre a1-b1 should be OK
//*************************************************************************
    AdAboveThreshold = Do_Common_Start();
    if (!AdAboveThreshold)
    {
        LongCounter_c1 = LONG_COUNT_C_INIT_EPEE;
    }
    else
    {
        if(LongCounter_c1)
            LongCounter_c1--;
    }

    OrangeR = TempOrangeR;
    OrangeL = TempOrangeL;
    return;
}
