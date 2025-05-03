//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "3WeaponSensor.h"

// Below are are all the settings needed per phase: IO direction (input/outpout), IO values (High or Low), analog channel, Threashold
// These are machine constants. It might be usefull to move this to flash and make it calibratable

MeasurementCtl SabreSets[] = {{IODirection_al_cr, IOValues_al_cr, bl_analog, 1750},
  {IODirection_ar_br, IOValues_ar_br, cr_analog, 1750},
  {IODirection_ar_cl, IOValues_ar_cl, br_analog, 1750},
  {IODirection_br_bl, IOValues_br_bl, cl_analog, 1750},
  {IODirection_al_cl, IOValues_al_cl, bl_analog, 1184},
  {IODirection_ar_cr, IOValues_ar_cr, cl_analog, 1750},
  {IODirection_al_bl, IOValues_al_bl, cr_analog, 1750}
};

/*
MeasurementCtl SabreSets[] = {{IODirection_al_cr, IOValues_al_cr, bl_analog, 1750},
  {IODirection_bl_cl, IOValues_bl_cl, cr_analog, 1750},
  {IODirection_ar_br, IOValues_ar_br, cl_analog, 1184},
  {IODirection_ar_cl, IOValues_ar_cl, br_analog, 1750},
  {IODirection_br_cr, IOValues_br_cr, cl_analog, 1750},
  {IODirection_br_bl, IOValues_br_bl, cr_analog, 1184},
  {IODirection_al_cl, IOValues_al_cl, bl_analog, 1184},
  {IODirection_ar_cr, IOValues_ar_cr, cl_analog, 1750},
  {IODirection_al_bl, IOValues_al_bl, cr_analog, 1750}
};
*/
void MultiWeaponSensor::DoSabre()
{
static bool AdAboveThreshold;
static bool TempOrangeL, TempOrangeR;
    //*************************************************************************
    //*************************************************************************
    //**         From this point you find the functions for sabre            **
    //*************************************************************************
    //*************************************************************************
    //       	PhaseCounter =START_OF_SABRE_TABLE;
    Set = SabreSets;

    //*************************************************************************
    //            Phas_S0: Test if wire is broken: a1-b1 -> b1 =?= low        *
    //*************************************************************************
    if(FullScanCounter == 0)
    {
      AdAboveThreshold = Do_Common_Start();
      if (!AdAboveThreshold)
      {
          Counter_b1--;
          if (!Counter_b1)
          {
              WhiteL = true;
              Counter_b1++;
          }
          NotConnectedLeft = true;
      }
      else
      {
          Counter_b1 = COUNT_B1_INIT_SABRE;
          NotConnectedLeft = false;
          WhiteL = false;
      }
    }
    else
    {
      Skip_phase();
    }

    //*************************************************************************
    //        Phas_S1: Test if opponent contact: a1-b1-c2 -> c2 =?= high      *
    //*************************************************************************
    AdAboveThreshold = Do_Common_Start();

    if (WhiteL)
    {
        // A1-wire is broken
        /*			if(ADRESH < SABRE_A_BROKEN_THREASHOLD)
        				AdAboveThreshold = 0;
        			else
        				AdAboveThreshold = 1;*/
    }
    if (AdAboveThreshold)
    {
        if (!SignalRight)
        {
            if(Counter_c1)
                Counter_c1--;
            if (!Counter_c1)
            {
                MaybeSignalRight = true;
                Const_COUNT_C1_INIT_SABRE = COUNT_C1_INIT_SABRE - 1;
                if(Counter_c2)
                    Counter_c2--;	//here I assume Counter_c2 is not yet 0, or if it is, it will not make a difference anymore
                c1_reached1 = false;
            }
            else
            {
                if (Counter_c1 == 1)
                {
                    c1_reached1 = true;
                }
            }
        }
        // This part is needed for automatic weapon detection. It is only relevant if "the point is pressed down", because
        // to see if we have to switch from sabre to foil we check that there is contact between the point and the opponent's
        // lame AND that the point is  pressed down. (white light is on)
        // We will check during the dummy phase if the highbyte of the long is also zero
        if (LongCounter_b1)
            LongCounter_b1--;

    }
    else
    {

        if (c1_reached1)
        {
            // this means we have blocked a hit, so we should block the otherone too
            c1_reached1 = false;
            WeHaveBlockedAhit = true;
            Const_COUNT_C1_INIT_SABRE = COUNT_C1_INIT_SABRE + 1;
            Counter_c2++;
        }
        Counter_c1 = Const_COUNT_C1_INIT_SABRE;
        LongCounter_b1 = LONG_COUNT_B_INIT_SABRE;

    }



    //*************************************************************************
    //    Phas_S2: Test if Leak jacket - Weapon: b1-c1 ==>  c1 ?= high        *
    //*************************************************************************
    //Skip_phase();
    /*AdAboveThreshold = Do_Common_Start();

    if (AdAboveThreshold)
    {
        TempOrangeR = true;
    }
    else
    {
        TempOrangeR = false;
    }
    */


    //*************************************************************************
    // Phas_S3: Test if wire is broken: a2-b2 -> b2 =?= low                   *
    //*************************************************************************
    if(FullScanCounter == 2)
    {
      AdAboveThreshold = Do_Common_Start();
      if (!AdAboveThreshold)
      {
          Counter_b2--;
          if (!Counter_b2)
          {
              WhiteR = true;
              Counter_b2++;
          }
          NotConnectedRight = true;
      }
      else
      {
          Counter_b2 = COUNT_B1_INIT_SABRE;
          NotConnectedRight = false;
          WhiteR = false;
      }
    }
    else
    {
      Skip_phase();
    }



    //*************************************************************************
    //        Phas_S4: Test if opponent contact: a2-b2-c1 -> c1 =?= high      *
    //*************************************************************************

    AdAboveThreshold = Do_Common_Start();
    if (WhiteR)
    {
        // A2-wire is broken
        /*			if(ADRESH < SABRE_A_BROKEN_THREASHOLD)
        				AdAboveThreshold = 0;
        			else
        				AdAboveThreshold = 1;*/
    }
    if (AdAboveThreshold)
    {
        if (!SignalLeft)
        {
            if(Counter_c2)
                Counter_c2--;
            if (!Counter_c2)
            {
                MaybeSignalLeft = true;
                Const_COUNT_C1_INIT_SABRE = COUNT_C1_INIT_SABRE - 1;
                if(Counter_c1)
                    Counter_c1--;	//here I assume Counter_b2 is not yet 0, or if it is, it will not make a difference anymore
                c2_reached1 = false;
            }
            else
            {
                if (Counter_c2 == 1)
                {
                    c2_reached1 = true;
                }
            }
        }
        // This part is needed for automatic weapon detection. It is only relevant if "the point is pressed down", because
        // to see if we have to switch from sabre to foil we check that there is contact between the point and the opponent's
        // lame AND that the point is  pressed down. (white light is on)
        // We will check during the dummy phase if the highbyte of the long is also zero
        if (LongCounter_b2)
            LongCounter_b2--;
    }
    else
    {

        if (c2_reached1)
        {
            // this means we have blocked a hit, so we should block the otherone too
            c2_reached1 = false;
            WeHaveBlockedAhit = true;
            Const_COUNT_C1_INIT_SABRE = COUNT_C1_INIT_SABRE + 1;
            Counter_c1++;
        }
        Counter_c2 = Const_COUNT_C1_INIT_SABRE;
        LongCounter_b2 = LONG_COUNT_B_INIT_SABRE;
    }



    //*************************************************************************
    //                      Phas_S5: Test if Leak jacket - Weapon: c2 =?= high*
    //*************************************************************************
    //Skip_phase();
    /*{
        AdAboveThreshold = Do_Common_Start();
        if (AdAboveThreshold)
        {
            TempOrangeL = true;
        }
        else
        {
            TempOrangeL = false;
        }
        //Set++;
    }*/

    //*************************************************************************
    //                      Phas_S6: Test if Weapons in contact: b2 =?= High  *
    //*************************************************************************
    WeaponContact = false;
    AdAboveThreshold = Do_Common_Start();
    if ((SignalLeft && SignalRight))
        return;
    if (AdAboveThreshold)
    {
        // There is weapon contact
        WeaponContact = true;
        BlockWhipover = true;
    }
    if (BlockWhipover)
    {
        // Maybe we need to block
        BlockCounter++;
        if (BlockCounter > BLOCKINGPERIOD1) //if less than 4.5 ms allow touch
        {
            if (BlockCounter < BLOCKINGPERIOD2)
            {
                // Between 4.5 and 10 ms block touch
                MaybeSignalLeft = false;
                MaybeSignalRight = false;
            }
            else
            {
                // after more than 10 ms allow touch
                BlockCounter--;  // needed to avoid overflow if weapon contact remains
                if (!WeaponContact)
                {
                    //if after 15 ms the weapons are not in contact any more -> forget about blocking
                    BlockCounter = 0;
                    BlockWhipover = false;
                }
            }
        }

    }
    // No need for Blocking!!
    if (MaybeSignalLeft)
    {
        StartLock(SABRE_LOCK_TIME);
        Green = true;
        Buzz = true;
        SignalLeft = true;
    }
    if (MaybeSignalRight)
    {
        StartLock(SABRE_LOCK_TIME);
        Red = true;
        Buzz = true;
        SignalRight = true;
    }
    /************************************************************************/
    /*    used for videoref to check if there is a parry      */
    /************************************************************************/
    /* Since the Parry-signal will be transmitted by serial line to the videoref application */
    /* We don't want to send too many signals going up and down. */
    /* I have no clue yet of what a good parry debouncing time would be, but if we decide there was a parry,
      /* it has to stay visible for at least a few video frames*/

    if (!bParrySignal)
    {
        //No Parry is signalled yet, check if we need to light the Parry signal
        if (Counter_parry)
        {
            if (!AdAboveThreshold)
            {
                Counter_parry = Const_FOIL_PARRY_ON_TIME;
            }
            else
                Counter_parry--;
        }
        else
        {
            bParrySignal = true;
            Counter_parry = Const_FOIL_PARRY_OFF_TIME;
        }
    }
    else
    {
        // We have allready signalled a parry -> check if we should switch off the parry signal
        if (Counter_parry)
        {
            if (AdAboveThreshold)
            {
                Counter_parry = Const_FOIL_PARRY_OFF_TIME;
            }
            else
                Counter_parry--;
        }
        else
        {
            bParrySignal = false;
            Counter_parry = Const_FOIL_PARRY_ON_TIME;
        }
    }

    // This part is needed for automatic weapon detection. It is only relevant if "the point is pressed down", because
    // to see if we have to switch from sabre to foil we check that there is contact between the point and the opponent's
    // lame AND that the point is  pressed down. (white light is on)
    // We will check during the dummy phase if the highbyte of the long is also zero
    //*************************************************************************
    //    Phas_S7: Test if contact jacket - point: a1-c1 ==>  c1 ?= high        *
    //*************************************************************************
    if((FullScanCounter == 1)&& (m_DectionMode == AUTO))
    {
      AdAboveThreshold = Do_Common_Start();
      if (AdAboveThreshold)
      {
          TempOrangeR = true;
          if (LongCounter_c1)
              LongCounter_c1--;
      }
      else
      {
          TempOrangeR = false;
          LongCounter_c1 = LONG_COUNT_C_INIT_SABRE;
      }
    }
    else
    {
      Skip_phase();
    }
    //*************************************************************************
    //    Phas_S8: Test if contact jacket - point: a1-c1 ==>  c1 ?= high        *
    //*************************************************************************
    if((FullScanCounter == 3) && (m_DectionMode == AUTO))
    {
      AdAboveThreshold = Do_Common_Start();
      if (AdAboveThreshold)
      {
          TempOrangeL = true;
          if (LongCounter_c2)
              LongCounter_c2--;
      }
      else
      {
          TempOrangeL = false;
          LongCounter_c2 = LONG_COUNT_C_INIT_SABRE;
      }
    }
    else
    {
      Skip_phase();
    }
    OrangeR = TempOrangeL;
    OrangeL = TempOrangeR;
    return;
}
