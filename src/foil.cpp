//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "3WeaponSensor.h"
// Below are are all the settings needed per phase: IO direction (input/outpout), IO values (High or Low), analog channel, Threashold
// These are machine constants. It might be usefull to move this to flash and make it calibratable

MeasurementCtl FoilSets[] = {{IODirection_al_cr, IOValues_al_cr, bl_analog, 1750},
  {IODirection_al_piste, IOValues_al_piste, cr_analog, 1750},
  {IODirection_al_br, IOValues_al_br, piste_analog, 1750},
  {IODirection_bl_cl, IOValues_bl_cl, br_analog, 1750},
  {IODirection_ar_br, IOValues_ar_br, cl_analog, 1184},
  {IODirection_ar_cl, IOValues_ar_cl, br_analog, 1750},
  {IODirection_ar_piste, IOValues_ar_piste, cl_analog, 1750},
  {IODirection_ar_bl, IOValues_ar_bl, piste_analog, 1750},
  {IODirection_br_cr, IOValues_br_cr, bl_analog, 1750},
  {IODirection_ar_cr, IOValues_ar_cr, cr_analog, 1184},
  {IODirection_al_cl, IOValues_al_cl, cr_analog, 1750},
  {IODirection_br_bl, IOValues_br_bl, cl_analog, 1750},
  {IODirection_al_bl, IOValues_al_bl, bl_analog, 1184}
};
void MultiWeaponSensor::DoFoil(void)
{
    //PhaseCounter =START_OF_FOIL_TABLE;
    static bool AdAboveThreshold;
    static bool TempOrangeL, TempOrangeR;
    Set = FoilSets;
    /************************************************************************/
    /*    Phase0: Test if point is pressed down: b1 =?= low                  */
    /************************************************************************/
    AdAboveThreshold = Do_Common_Start();
//     	if (!SignalLeft)
//		{
    if (!AdAboveThreshold)
    {
        if(Counter_b1)
        {
            Counter_b1--;
        }
        if(Counter_b1 == 1)
        {
            b1_reached1 = true;
        }
        NotConnectedLeft = true;

    }
    else
    {
        Counter_b1 = Const_COUNT_B1_INIT_FOIL;
        if(b1_reached1)
        {
            // this means we have blocked a hit, so we should block the otherone too
            b1_reached1 = false;
            WeHaveBlockedAhit = true;
            Const_COUNT_B1_INIT_FOIL = COUNT_B1_INIT_FOIL +1;
            Counter_b2++;
        }

        NotConnectedLeft = false;
    }
//	}
    /************************************************************************/
    /*    Phase1: Test if point makes contact with jacket: c2 =?= high      */
    /************************************************************************/
// hier ga ik moeten her schrijven. Ik moet kunnen weten om of er lang wapencontact met ander vest is om naar sabel te switchen.
// ik moet dat overigens alleen weten als de punt NIET is ingedrukt!

    AdAboveThreshold = Do_Common_Start();
    if (!SignalLeft)
    {
        if (AdAboveThreshold)
        {
            PossiblyRed = true;
        }
        if (!Counter_b1)
        {
            if (PossiblyRed)
            {
                Red = true;
                Buzz = true;
                SignalLeft = true;
                StartLock(Const_FOIL_LOCK_TIME);
                Const_COUNT_B1_INIT_FOIL = COUNT_B1_INIT_FOIL-1;
                Counter_b2--;	//here I assume Counter_b2 is not yet 0, or if it is, it will not make a difference anymore
                b1_reached1 = false;
            }
        }
        else
        {
            // This part is needed for automatic weapon detection. It is only relevant if "the point is not pressed down", because
            // to see if we have to switch from foil to sabre we check that there is contact between the blad and the opponent's
            // lame AND that the point is NOT pressed down.
            // Note that the assumption is that pressing down the point wil result in SignalLeft being set.

            if (!AdAboveThreshold)
            {
                LongCounter_b1 = LONG_COUNT_B_INIT_FOIL;
            }
            else
            {
                // We will check during the dummy phase if the highbyte of the long is also zero
                if(LongCounter_b1)
                    LongCounter_b1--;
            }
        }

        PossiblyRed = AdAboveThreshold; // in this way the test above is done on the old value

    }


    /************************************************************************/
    /*    Phase2: Test if point makes contact with ground: gnd =?= high      */
    /************************************************************************/
    if ((Counter_b1) || Red){
      Skip_phase();
      }
    else{
      AdAboveThreshold = Do_Common_Start();
      if (!SignalLeft)
      {
          if (!Counter_b1)
          {
              if (AdAboveThreshold)
              {
                  Counter_b1 = Const_COUNT_B1_INIT_FOIL;
              }
          }
      }
    }

    /************************************************************************/
    /*    Phase3: Test if point makes contact with weapon: b2 =?= high    */
    /************************************************************************/
    if ((Counter_b1) || Red){
      Skip_phase();
      }
    else{
      AdAboveThreshold = Do_Common_Start();
      if (!SignalLeft)
      {
          if (!Counter_b1)
          {

              if (AdAboveThreshold)
              {
                  Counter_b1 = Const_COUNT_B1_INIT_FOIL;
              }

              else
              {
                  WhiteL = true;
                  Buzz = true;
                  SignalLeft = true;
                  StartLock(Const_FOIL_LOCK_TIME);
                  Const_COUNT_B1_INIT_FOIL = COUNT_B1_INIT_FOIL-1;
                  Counter_b2--;	//here I assume Counter_b2 is not yet 0, or if it is, it will not make a difference anymore
                  b1_reached1 = false;
              }
          }
      }
    }

    /************************************************************************/
    /*    Phase4: Test if weapon makes contact with jacket: c1 =?= high     */
    /************************************************************************/
    if(Red || Green || (FullScanCounter != 0)){
      Skip_phase();
    }
    else{
      AdAboveThreshold = Do_Common_Start();
      if (AdAboveThreshold)
      {
          Counter_c1--;
          if (!Counter_c1)
          {
              TempOrangeR = true;
              Counter_c1++;
          }
      }
      else
      {
          Counter_c1 = Const_COUNT_C1_INIT_FOIL;
          TempOrangeR = false;
      }
    }




    /************************************************************************/
    /*    Phase5: Test if point is pressed down: b2 =?= low                  */
    /************************************************************************/
    AdAboveThreshold = Do_Common_Start();
//	if (!SignalRight)
//	{
    if (!AdAboveThreshold)
    {
        if(Counter_b2)
        {
            Counter_b2--;
        }
        if(Counter_b2 == 1)
        {
            b2_reached1 = true;
        }

        NotConnectedRight = true;
    }
    else
    {
        Counter_b2 = Const_COUNT_B1_INIT_FOIL;
        if(b2_reached1)
        {
            b2_reached1 = false;
            WeHaveBlockedAhit = true;
            Const_COUNT_B1_INIT_FOIL = COUNT_B1_INIT_FOIL +1;
            Counter_b1++;
        }
        NotConnectedRight = false;
    }
//	}
    /************************************************************************/
    /*    Phase6: Test if point makes contact with jacket: c1 =?= high      */
    /************************************************************************/
    AdAboveThreshold = Do_Common_Start();
    if (!SignalRight)
    {
        if (AdAboveThreshold)
        {
            PossiblyGreen = true;
        }
        if (!Counter_b2)
        {
            if (PossiblyGreen)
            {
                Green = true;
                Buzz = true;
                SignalRight = true;
                StartLock(Const_FOIL_LOCK_TIME);
                Const_COUNT_B1_INIT_FOIL = COUNT_B1_INIT_FOIL-1;
                Counter_b1--;	//here I assume Counter_b2 is not yet 0, or if it is, it will not make a difference anymore
                b2_reached1 = false;
            }
        }
        else
        {
            // This part is needed for automatic weapon detection. It is only relevant if "the point is not pressed down", because
            // to see if we have to switch from foil to sabre we check that there is contact between the blad and the opponent's
            // lame AND that the point is NOT pressed down.
            // Note that the assumption is that pressing down the point wil result in SignalRight being set
            if (!AdAboveThreshold)
            {
                LongCounter_b2 = LONG_COUNT_B_INIT_FOIL;
            }
            else
            {
                // We will check during the dummy phase if the highbyte of the long is also zero
                if(LongCounter_b2)
                    LongCounter_b2--;
            }
        }
        PossiblyGreen = AdAboveThreshold;
    }


    /************************************************************************/
    /*    Phase7: Test if point makes contact with ground: gnd =?= high      */
    /************************************************************************/
    if ((Counter_b2) || Green){
      Skip_phase();}
    else{
      AdAboveThreshold = Do_Common_Start();
      if (!SignalRight)
      {
          if (!Counter_b2)
          {
              if (AdAboveThreshold)
              {
                  Counter_b2 = Const_COUNT_B1_INIT_FOIL;
              }
          }
      }
    }

    /************************************************************************/
    /*    Phase8: Test if point makes contact with weapon: b1 =?= high    */
    /************************************************************************/
    if ((Counter_b2) || Green){
      Skip_phase();}
    else{
      AdAboveThreshold = Do_Common_Start();
      if (!SignalRight)
      {
          if (!Counter_b2)
          {
              if (AdAboveThreshold)
              {
                  Counter_b2 = Const_COUNT_B1_INIT_FOIL;
              }
              else
              {
                  WhiteR = true;
                  Buzz = true;
                  SignalRight = true;
                  StartLock(Const_FOIL_LOCK_TIME);
                  Const_COUNT_B1_INIT_FOIL = COUNT_B1_INIT_FOIL-1;
                  Counter_b1--;	//here I assume Counter_b2 is not yet 0, or if it is, it will not make a difference anymore
                  b2_reached1 = false;
              }
          }
      }
    }

    /************************************************************************/
    /*    Phase9: Test if weapon makes contact with jacket: c2 =?= high     */
    /************************************************************************/
    if(Red || Green || (FullScanCounter != 1)){
      Skip_phase();
    }
    else{
      AdAboveThreshold = Do_Common_Start();
//     	if (!SignalRight)
//     	{
      if (AdAboveThreshold)
      {
          Counter_c2--;
          if (!Counter_c2)
          {
              TempOrangeL = true;
              Counter_c2++;
          }
      }
      else
      {
          Counter_c2 = Const_COUNT_C1_INIT_FOIL;
          TempOrangeL = false;
      }
//     	}
}

//*************************************************************************
//    Phase10: The next 2 phases are used only in club-version with
//    automatic weapon selection
//    Test if epee point pressed down on valid surface for at least 5 seconds =>
//    if (ax-cx) & !(ax-bx) -> switch to epee
//    Check if a2-c2  -> c2 low
//*************************************************************************
  if(Red || Green || (FullScanCounter != 2)){
    Skip_phase();
  }
  else{
    AdAboveThreshold = Do_Common_Start();
      if (!AdAboveThreshold)
      {
          LongCounter_c2 = LONG_COUNT_C_INIT_FOIL;
          //LongCounter_c2 = LONG_COUNT_C_INIT_FOIL_H;
      }
      else
      {
          if(LongCounter_c2)
              LongCounter_c2--;
      }
    }

//*************************************************************************
//    Phase11:
//*************************************************************************
  if(Red || Green || (FullScanCounter != 3)){
    Skip_phase();
  }
  else{
    AdAboveThreshold = Do_Common_Start();
      if (!AdAboveThreshold)
      {
          LongCounter_c1 = LONG_COUNT_C_INIT_FOIL;
          //LongCounter_c1_H = LONG_COUNT_C_INIT_FOIL_H;
      }
      else
      {
          if(LongCounter_c1)
              LongCounter_c1--;
      }
    }


    /************************************************************************/
    /*    Phase12: Test if both blades are in contact; used for videoref to check if there is a parry      */
    /************************************************************************/
    /* Since the Parry-signal will be transmitted by serial line to the videoref application */
    /* We don't want to send too many signals going up and down. */
    /* I have no clue yet of what a good parry debouncing time would be, but if we decide there was a parry,
    /* it has to stay visible for at least a few video frames*/

    if(Red || Green || (FullScanCounter != 4)){
      Skip_phase();
    }
    else{
      AdAboveThreshold = Do_Common_Start();

      if(!bParrySignal)
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
    }
    OrangeR = TempOrangeL;   // It seems I made a mistake and show the orange at the wrong side
    OrangeL = TempOrangeR;
    return;
}
