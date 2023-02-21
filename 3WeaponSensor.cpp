//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>

#include "3WeaponSensor.h"
#include "driver/adc.h"
#include "esp_timer.h"
#include <iostream>
#include <Preferences.h>


using namespace std;

// I have 7 Output /Tristate pins (3 per weapon + piste)
const uint8_t driverpins[] = {al_driver, bl_driver, cl_driver, ar_driver, br_driver, cr_driver, piste_driver};
// I use 5 analog inputs
//const uint8_t ADpins[] = {cl_analog, bl_analog, piste_analog, cr_analog, br_analog};


volatile SemaphoreHandle_t timerSemaphore;
void IRAM_ATTR onTimer()
{
  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(timerSemaphore, NULL);
}

//0.05 milliseconds
#define TimeInMicroSeconds 4800

MultiWeaponSensor::MultiWeaponSensor(int hw_timer_nr)
{
    //ctor
    adc_power_on();
    if(ESP_OK != adc_set_clk_div(1))
      cout << "I did not expect this!" << endl;
    if(ESP_OK != adc1_config_width(ADC_WIDTH_BIT_12))
      cout << "I did not expect this!" << endl;
    if(ESP_OK != adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_11))
      cout << "I did not expect this!" << endl;
    adc1_config_channel_atten(ADC1_CHANNEL_3,ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC1_CHANNEL_4,ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC1_CHANNEL_7,ADC_ATTEN_DB_11);
    cout << "ADC configured" << endl;
    int test = adc1_get_raw(ADC1_CHANNEL_0);
    cout << "First measurement = " << tempADValue << endl;
  // Create semaphore to inform us when the timer has fired
  //timerSemaphore = xSemaphoreCreateCounting(20,0);
  // Use 2nd timer of 4 (counted from zero). It seems timer0 is already in use (with a different scaler?)
  // Set 4 divider for prescaler (see ESP32 Technical Reference Manual for more info).
  //timer = timerBegin(hw_timer_nr, 8, true);
  // Attach onTimer function to our timer.
  //timerAttachInterrupt(timer, &onTimer, true);
  // Set alarm to call onTimer function every x microsecond microseconds.
  // Repeat the alarm (third parameter)
  //timerAlarmWrite(timer, TimeInMicroSeconds, true);
  // Start an alarm
  //timerAlarmEnable(timer);

  Const_COUNT_B1_INIT_FOIL = COUNT_B1_INIT_FOIL;
  Const_COUNT_C1_INIT_FOIL = COUNT_C1_INIT_FOIL;
  Const_COUNT_Cx_INIT_EPEE = COUNT_C1_INIT_EPEE;
  Const_COUNT_C1_INIT_SABRE = COUNT_C1_INIT_SABRE;
  Const_FOIL_PARRY_ON_TIME = 5;
  Const_FOIL_PARRY_OFF_TIME = 43;
  Const_FOIL_LOCK_TIME = FOIL_LOCK_TIME;
  LongCounter_b1 = LONG_COUNT_B_INIT_FOIL;
  LongCounter_b2 = LONG_COUNT_B_INIT_FOIL;
  LongCounter_c1 = LONG_COUNT_C_INIT_FOIL;
  LongCounter_c2 = LONG_COUNT_C_INIT_FOIL;
  LongCounter_NotConnected = LONG_COUNT_NOTCONNECTED_INIT;

  SensorMutex = xSemaphoreCreateBinary();
}

void MultiWeaponSensor:: begin()
{
  Preferences mypreferences;
  mypreferences.begin("scoringdevice", false);
  LightsDuration = mypreferences.getInt("LIGHTS_MS",0);
  cout << "LIGHTS_MS: " <<  LightsDuration << endl;
  if(!LightsDuration)
  {
    mypreferences.putInt("LIGHTS_MS", LIGHTS_DURATION_MS);
    LightsDuration = LIGHTS_DURATION_MS;
  }
  uint8_t storedweapon = mypreferences.getUChar("START_WEAPON",99);
  cout << "START_WEAPON: " <<  storedweapon << endl;
  if(99 == storedweapon)
  {
    mypreferences.putUChar("START_WEAPON",0);
    storedweapon = 0;
  }
  switch (storedweapon) {
    case 0:
    m_ActualWeapon = FOIL;
    break;

    case 1:
    m_ActualWeapon = EPEE;
    break;

    case 2:
    m_ActualWeapon = SABRE;
    break;
    defaut:
    m_ActualWeapon = EPEE;

  }
  mypreferences.end();
  DoReset();
}




void Set_IODirectionAndValue(uint8_t setting, uint8_t values)
{
  uint8_t mask = 1;
  for (int i = 0; i < 7; i++)
  {
    if (setting & mask)
    {
      pinMode(driverpins[i], INPUT);
    }
    else
    {
      pinMode(driverpins[i], OUTPUT);
      if (values & mask)
      {
        digitalWrite(driverpins[i], HIGH);
      }
      else
      {
        digitalWrite(driverpins[i], LOW);
      }

    }
    mask <<= 1;
  }
}
adc1_channel_t ADC1_CHANNELS[] = {ADC1_CHANNEL_0,ADC1_CHANNEL_1,ADC1_CHANNEL_2,ADC1_CHANNEL_3,ADC1_CHANNEL_4,ADC1_CHANNEL_5,ADC1_CHANNEL_6,ADC1_CHANNEL_7};

bool MultiWeaponSensor::Do_Common_Start()
{
  //while (!xSemaphoreTake(timerSemaphore, 1 / (portTICK_PERIOD_MS)) == pdTRUE);
  //while (!xSemaphoreTake(timerSemaphore, 0) == pdTRUE);

  //tempADValue = analogRead(Set->ADChannel);
  tempADValue = adc1_get_raw(ADC1_CHANNELS[Set->ADChannel]);
  Set_IODirectionAndValue(Set->IODirection, Set->IOValues);
  if (tempADValue > Set->ADThreashold)
  {
    Set++;
    return true;
  }
  else
  {
    Set++;
    return false;
  }

}

void MultiWeaponSensor::Skip_phase()
{
  //while (!xSemaphoreTake(timerSemaphore, 1 / (portTICK_PERIOD_MS)) == pdTRUE);
  //while (!xSemaphoreTake(timerSemaphore, 0) == pdTRUE);
  Set_IODirectionAndValue(Set->IODirection, Set->IOValues);
  Set++;
  return;
}

bool MultiWeaponSensor::Wait_For_Next_Timer_Tick()
{
  //while (!xSemaphoreTake(timerSemaphore, 1 / (portTICK_PERIOD_MS)) == pdTRUE);
  while (!xSemaphoreTake(timerSemaphore, 0) == pdTRUE);
    return true;
}

MultiWeaponSensor::~MultiWeaponSensor()
{
    //dtor
}

void MultiWeaponSensor::HandleLights()
{
  uint8_t temp = 0;
  if(millis() > ShortIndicatorsDebouncer ) // this is needed to avoid too many events for broken wires or bad mass contacts
  {
    ShortIndicatorsDebouncer = millis() + 233;
    if(OrangeL)
      temp |= 0x20;
    if(OrangeR)
      temp |= 0x10;
    if(m_ActualWeapon == SABRE)
    {
      if(WhiteL)
        temp |= 0x40;
      if(WhiteR)
        temp |= 0x08;
    }
  }
  else
  {
    temp = Lights & 0x30;
    if(m_ActualWeapon == SABRE)
    {
      temp = Lights & 0x78;
    }
  }
  if(Red)
    temp |= 0x80;
  if(m_ActualWeapon == FOIL)
  {
    if(WhiteL)
      temp |= 0x40;
    if(WhiteR)
      temp |= 0x08;
  }
  if(Green)
    temp |= 0x04;

  if(Buzz && !bPreventBuzzer)
    temp |= 0x02;
  if(Lights != temp)
  {
    Lights =temp;
    SensorStateChanged(EVENT_LIGHTS | temp);
  }

}

bool MultiWeaponSensor::OKtoReset()
{
  if (!WaitingForResetStarted)
  {
    WaitingForResetStarted = true;
    //InitLock();
    // every weapon has a diferent lock time. The buzzer has to stop after 2 sec
    switch (m_ActualWeapon)
    {
      case FOIL:
        TimeToReset = millis() + LightsDuration - FOIL_LOCK_TIME;
        break;

      case EPEE:
        TimeToReset = millis() + LightsDuration - EPEE_LOCK_TIME;
        break;

      case SABRE:
        TimeToReset = millis() + LightsDuration - SABRE_LOCK_TIME;
        break;

    }
    return false;
  }
  if (millis() > TimeToReset)
  {
    if (Buzz == false)
    {
      return (true);

    }

    //BUZZPIN = false;
    Buzz = false;
    /* here comes the extra time if you want to wait longer than 2 sec */
    TimeToReset = millis() + 500;
  }
  return (false);
}

void MultiWeaponSensor::DoReset()
{
    WhiteL = false;
    OrangeL = false;
    OrangeR = false;
    WhiteR = false;
    Green = false;
    Red = false;
    Buzz = false;
    HandleLights();
    MaybeSignalRight = false;
    MaybeSignalLeft = false;
    BlockWhipover = false;
    WeaponContact = false;
    WaitingForResetStarted = false;
    //chronostatus = CHRONO_RUNNING;
    Const_COUNT_B1_INIT_FOIL = COUNT_B1_INIT_FOIL;
    Const_COUNT_Cx_INIT_EPEE = COUNT_C1_INIT_EPEE;
    Const_COUNT_C1_INIT_SABRE = COUNT_C1_INIT_SABRE;
    b1_reached1 = false;
    b2_reached1 = false;
    c1_reached1 = false;
    c2_reached1 = false;
    PossiblyRed = false;
    PossiblyGreen = false;
    WeHaveBlockedAhit = false;
    BlockedAHitCounter = 650;
    FullScanCounter = 1;

    switch (m_ActualWeapon)
    {
    case FOIL:
        Counter_b1 = Const_COUNT_B1_INIT_FOIL;
        Counter_b2 = Const_COUNT_B1_INIT_FOIL;
        Counter_c1 = Const_COUNT_C1_INIT_FOIL;
        Counter_c2 = Const_COUNT_C1_INIT_FOIL;
        Const_FOIL_LOCK_TIME = FOIL_LOCK_TIME;
        CurrentPhaseDuration = FULLSCANDURATION_FOIL;

        break;

    case EPEE:
        Counter_c1 = Const_COUNT_Cx_INIT_EPEE;
        Counter_c2 = Const_COUNT_Cx_INIT_EPEE;
        CurrentPhaseDuration = FULLSCANDURATION_EPEE;

        break;

    case SABRE:
        Counter_b1 = COUNT_B1_INIT_SABRE;
        Counter_b2 = COUNT_B1_INIT_SABRE;
        Counter_c1 = Const_COUNT_C1_INIT_SABRE;
        Counter_c2 = Const_COUNT_C1_INIT_SABRE;
        CurrentPhaseDuration = FULLSCANDURATION_SABRE;

        break;

    }
    SignalLeft = 0;
    SignalRight = 0;
    LockStarted = false;
    /*  InitLock();
      InitTimer2Period();*/

    bParrySignal = false;

    Counter_parry = Const_FOIL_PARRY_ON_TIME;
    return;
}


void MultiWeaponSensor::StartLock(int TimeToLock)
{
    if(!LockStarted)
    {
        LockStarted = 1;
        TimeOfLock = millis() + TimeToLock;
    }
}

bool MultiWeaponSensor::IsLocked()
{
    if(LockStarted)
    {
        if(millis() > TimeOfLock)
            return true;
    }
    return false;
}


void MultiWeaponSensor::DoFullScan()
{
    /***************************************************************************************************/
    /*************  Start of Dummy phase          ******************************************************/
    /***************************************************************************************************/


    if(esp_timer_get_time() <  TimetoNextPhase)
      return;
    TimetoNextPhase = esp_timer_get_time() + CurrentPhaseDuration;
    /*while(esp_timer_get_time() <  TimetoNextPhase)
    {

    }*/
    weapon_t temp = GetWeapon();

    if (m_ActualWeapon != temp)
    { cout << "Detected a weapon change" << endl;
        m_ActualWeapon = temp;
        bPreventBuzzer = false;
        SensorStateChanged(EVENT_WEAPON | temp);
        DoReset();
    }

    if (WeHaveBlockedAhit)
    {
        BlockedAHitCounter--;
        if (!BlockedAHitCounter)
        {
            // after the blocking time we forget that we've blocked out a hit and reset the counters
            Const_COUNT_B1_INIT_FOIL = COUNT_B1_INIT_FOIL;
            Const_COUNT_Cx_INIT_EPEE = COUNT_C1_INIT_EPEE;
            Const_COUNT_C1_INIT_SABRE = COUNT_C1_INIT_SABRE;
            WeHaveBlockedAhit = false;
            switch (m_ActualWeapon)
            {
            case EPEE:
                BlockedAHitCounter = 95;
                break;

            case FOIL:
                BlockedAHitCounter = 670;
                break;

            case SABRE:
                BlockedAHitCounter = 340;
                break;
            }
        }
    }
    if (IsLocked())
    {
        SignalLeft = 1; // Now no changes will be registered for left
        SignalRight = 1; // Same for right
        if (OKtoReset())
        {
            DoReset();
        }
    }
    HandleLights();

    switch (m_ActualWeapon)
    {
    case FOIL:
        DoFoil();
        break;

    case EPEE:
        DoEpee();
        break;

    case SABRE:
        DoSabre();
        break;

    }

    if(FullScanCounter)
      FullScanCounter --;
    else
      FullScanCounter = 3;

}

weapon_t MultiWeaponSensor::GetWeapon()
{

  if (m_ActualWeapon != EPEE)
  {// In foil or sabre: check if both sides are disconnected
    if (NotConnectedRight && NotConnectedLeft)
    {
      LongCounter_NotConnected--;
      if(LongCounter_NotConnected < LONG_COUNT_NOTCONNECTED_STOP_BUZZING)
      {
        bPreventBuzzer = true;
      }
    }
    else
    {
      LongCounter_NotConnected = LONG_COUNT_NOTCONNECTED_INIT;
      bPreventBuzzer = false;
    }
    if (!LongCounter_NotConnected) // We've reached zero, so we switch back to default Epee
    {
      LongCounter_NotConnected = LONG_COUNT_NOTCONNECTED_INIT;
      bPreventBuzzer = false;
      cout << "Maybe Manual:Aha, it seems both sides are disconnected for a long time so we must be on EPEE" << endl;
      if(m_DectionMode != MANUAL)
      {
        m_DetectedWeapon =  EPEE;
        cout << "Not Manual:Aha, it seems both sides are disconnected for a long time so we must be on EPEE" << endl;
        return EPEE;
      }
    }
  }

  if(m_DectionMode == MANUAL)
  {
    bAutoDetect = 0;
    return m_ActualWeapon;
  }

  m_DetectedWeapon = m_ActualWeapon;
  bAutoDetect = 1;

  switch (m_ActualWeapon)
  {
    case FOIL:
      // if (ax-cx) & !(ax-bx) -> switch to epee

      if ((!LongCounter_c1) && (!LongCounter_c2))
      { // certainly not foil anymore)

        if ((Counter_b1) && (Counter_b2))
        {
          m_DetectedWeapon =  EPEE;
          bPreventBuzzer = false;
          LongCounter_c1 = LONG_COUNT_C_INIT_EPEE;
          LongCounter_c2 = LONG_COUNT_C_INIT_EPEE;

        }
      }
      // if (bx-cy) && (ax-bx) -> switch to sabre
      else
      {
        if ((!LongCounter_b1) && (!LongCounter_b2))
        {
          if ((Counter_b1) && (Counter_b2))
          {
            m_DetectedWeapon =  SABRE;
            bPreventBuzzer = false;
            LongCounter_b1 = LONG_COUNT_B_INIT_SABRE;
            LongCounter_b2 = LONG_COUNT_B_INIT_SABRE;
            LongCounter_c1 = LONG_COUNT_C_INIT_SABRE;
            LongCounter_c2 = LONG_COUNT_C_INIT_SABRE;
          }
        }
      }
      // in all other cases: -> keep foil
      return m_DetectedWeapon;

      break;

    case EPEE:
      // if (ax-cy) & !(ax-bx) -> switch to foil
      // if (ax-cy) & (ax-by) -> switch to sabre
      // keep epee
      if ((!LongCounter_c1) && (!LongCounter_c2))
      { // certainly not epee anymore)
        if ((OrangeR) && (OrangeL))
        {
          m_DetectedWeapon =  SABRE;
          bPreventBuzzer = false;
          LongCounter_b1 = LONG_COUNT_B_INIT_SABRE;
          LongCounter_b2 = LONG_COUNT_B_INIT_SABRE;
          LongCounter_c1 = LONG_COUNT_C_INIT_SABRE;
          LongCounter_c2 = LONG_COUNT_C_INIT_SABRE;
        }
        else
        {
          m_DetectedWeapon = FOIL;
          bPreventBuzzer = false;
          LongCounter_b1 = LONG_COUNT_B_INIT_FOIL;
          LongCounter_b2 = LONG_COUNT_B_INIT_FOIL;
          LongCounter_c1 = LONG_COUNT_C_INIT_FOIL;
          LongCounter_c2 = LONG_COUNT_C_INIT_FOIL;
        }

        return m_DetectedWeapon;
      }
      else
        return EPEE;
      break;

    case SABRE:
      // if (ax-cy) & !(ax-bx) -> switch to foil
      // if (ax-cx) & !(ax-bx) -> switch to epee
      // keep sabre
      if ((!WhiteR) || (!WhiteL)) // both points must be pressed down to trigger the switch
      {
        return SABRE;
      }
      else
      {
        if ((!LongCounter_b1) && (!LongCounter_b2))
        {
          m_DetectedWeapon =  FOIL;
          bPreventBuzzer = false;
          LongCounter_b1 = LONG_COUNT_B_INIT_FOIL;
          LongCounter_b2 = LONG_COUNT_B_INIT_FOIL;
          LongCounter_c1 = LONG_COUNT_C_INIT_FOIL;
          LongCounter_c2 = LONG_COUNT_C_INIT_FOIL;
        }
        if ((!LongCounter_c1) && (!LongCounter_c2))
        {
          m_DetectedWeapon =  EPEE;
          bPreventBuzzer = false;
          LongCounter_c1 = LONG_COUNT_C_INIT_EPEE;
          LongCounter_c2 = LONG_COUNT_C_INIT_EPEE;
        }
        return m_DetectedWeapon;
      }
      break;

  }
  return	EPEE;
}
