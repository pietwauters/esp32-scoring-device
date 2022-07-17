#include "CyranoHandler.h"
#include "EFP1Message.h"
#include <sstream>
#include <WiFi.h>
#include "AsyncUDP.h"


AsyncUDP CyranoHandlerudp;


CyranoHandler::CyranoHandler()
{
    //ctor
    Serial.println("CyranoHandler created");
    m_MachineStatus[PisteId]="1";
}

CyranoHandler::~CyranoHandler()
{
    //dtor
}



void CyranoHandler::ProcessMessageFromSoftware(const EFP1Message &input)
{
    if(input[PisteId] != m_MachineStatus[PisteId])
        return; // wrong Piste
    switch(input.GetType())
    {
        case HELLO :
        SendInfoMessage();
        break;

        case DISP :
        if(WAITING == m_State)
        {
            // Initialize with the received values
        }
        break;

        case ACK :
        if(WAITING != m_State)
        {
            // Initialize with the received values
            m_State = WAITING;
            SendInfoMessage();
        }
        break;

        case NAK :
        // The software doesn't accept the "END" message
        break;

    }

}

void CyranoHandler::ProcessUIEvents(EventType const event)
{
  bool bTransmit = true;
    switch(event)
    {
        case NextButtonPressed :
        if(WAITING == m_State)
        {

        }
        break;

        case PrevButtonPressed :
        if(WAITING == m_State)
        {

        }
        break;

        case BeginButtonPressed :
        if(WAITING == m_State)
        {

        }
        break;

        case EndButtonPressed :
        if(WAITING != m_State)
        {

        }
        break;

    }

}


#define MASK_ANY_ORANGE (MASK_ORANGE_L | MASK_ORANGE_R)

void CyranoHandler::ProcessLightsChange(uint32_t eventtype)
{
  uint32_t event_data = eventtype & SUB_TYPE_MASK;
  //if(!(event_data & ~MASK_ANY_ORANGE))  // no need to broadcast orange lights
    //return;

  m_MachineStatus.SetRed(event_data & MASK_RED);
  m_MachineStatus.SetGreen(event_data & MASK_GREEN);
  m_MachineStatus.SetWhiteLeft(event_data & MASK_WHITE_L);
  m_MachineStatus.SetWhiteRight(event_data & MASK_WHITE_R );


}

void CyranoHandler::update (FencingStateMachine *subject, uint32_t eventtype)
{
  uint32_t event_data = eventtype & SUB_TYPE_MASK;
  uint32_t maineventtype = eventtype & MAIN_TYPE_MASK ;
  uint32_t temp;
  mix_t PCardInfo;
  mix_t TimeInfo;
  char chrono[8];
  char strRound[8];
  int newseconds;
  int currentRound, nrOfRounds;
  bool bTransmit = true;

  switch(maineventtype)
  {

    case EVENT_LIGHTS:
      ProcessLightsChange(eventtype);
    break;

    case EVENT_WEAPON:
    switch (event_data)
    {

      case WEAPON_MASK_EPEE:
      m_MachineStatus[Weapon] = "E";
      break;

      case WEAPON_MASK_SABRE:
      m_MachineStatus[Weapon] = "S";
      break;

      case WEAPON_MASK_FOIL:
      m_MachineStatus[Weapon] = "F";
      break;

      default:
      m_MachineStatus[Weapon] = "";

    }

    break;

    case EVENT_SCORE_LEFT:
    {
      stringstream ss; ss << event_data;
      ss >> m_MachineStatus[LeftScore];
    }

    break;

    case EVENT_SCORE_RIGHT:
    {
      stringstream ss; ss << event_data;
      ss >> m_MachineStatus[RightScore];
    }

    break;
/*
    case EVENT_TIMER_STATE:

      if(eventtype & DATA_24BIT_MASK)
      {
        Message2.SetTimerStatus('R');
        BTTransmitMessage(2);
        WifiTransmitMessage(2);
      }
      else
      {
        Message2.SetTimerStatus('N');
        BTTransmitMessage(2);
        WifiTransmitMessage(2);
      }
    break;*/
    case EVENT_TIMER:
    temp = millis();
    if( temp < m_timeToShowTimer)
    {
      bTransmit = false;
    }
    else
    {
      bTransmit = true;
      if(m_timeToShowTimer - temp < 1000)
        m_timeToShowTimer += 1000;
      else
        m_timeToShowTimer = temp + 900;
    }


    //newseconds = event_data & (DATA_BYTE1_MASK |DATA_BYTE2_MASK);
    TimeInfo.theDWord = eventtype & DATA_24BIT_MASK;
    //if(previous_seconds != newseconds)
    {

      if((TimeInfo.theBytes[2] == 0) && (TimeInfo.theBytes[1] < 10))
      {
        sprintf(chrono,"%d.%02d",TimeInfo.theBytes[1],TimeInfo.theBytes[0]);
      }
      else
      {
        sprintf(chrono,"%d:%02d",TimeInfo.theBytes[2],TimeInfo.theBytes[1]);
      }

      m_MachineStatus[StopWatch] = chrono;
      //bTransmit = true;
      //previous_seconds = newseconds;
    }
    break;

    case EVENT_ROUND:
    {
      currentRound = event_data & DATA_BYTE0_MASK;
      if(currentRound < 10)
      {
        stringstream ss; ss << currentRound;
        ss >> m_MachineStatus[RoundNumber];
      }

    }
    break;

    case EVENT_YELLOW_CARD_LEFT:
    {
      stringstream ss; ss << event_data;
      ss >> m_MachineStatus[LeftYCard];
    }
    break;

    case EVENT_YELLOW_CARD_RIGHT:
    {
      stringstream ss; ss << event_data;
      ss >> m_MachineStatus[RightYCard];
    }
    break;

    case EVENT_RED_CARD_LEFT:
    {
      stringstream ss; ss << event_data;
      ss >> m_MachineStatus[LeftRCard];
    }
    break;

    case EVENT_RED_CARD_RIGHT:
    {
      stringstream ss; ss << event_data;
      ss >> m_MachineStatus[RightRCard];
    }
    break;

    case EVENT_P_CARD:
    if(m_MachineStatus[Protocol]!= "EFP1")
    {
      PCardInfo.theDWord = eventtype & DATA_24BIT_MASK;
      m_MachineStatus[RightPCards] = '0' + PCardInfo.theBytes[1];
      m_MachineStatus[LeftPCards] = '0' + PCardInfo.theBytes[0];
    }

    break;

    case EVENT_PRIO:
    switch(event_data)
    {
      case 2:
        m_MachineStatus[Priority] = "R";
      break;
      case 1:
        m_MachineStatus[Priority] = "L";
      break;
      default:
        m_MachineStatus[Priority] = "N";
    }
    break;

    default:
    bTransmit = false;

  }
  if(bTransmit)
  {
    string TheMessage;
    TheMessage = m_MachineStatus.ToString(TheMessage);
    Serial.println(TheMessage.c_str());
    CyranoHandlerudp.broadcastTo((uint8_t*)TheMessage.c_str(),TheMessage.length(), CYRANO_PORT,TCPIP_ADAPTER_IF_STA);
  }
}
