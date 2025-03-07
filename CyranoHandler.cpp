#include "CyranoHandler.h"
#include "EFP1Message.h"
#include <sstream>
#include "esp_log.h"
static const char* CYRANO_TAG = "Cyrano";

CyranoHandler::CyranoHandler()
{
    //ctor
    m_MachineStatus[PisteId]="1";
    m_MachineStatus[State] ="W";

}

void CyranoHandler::Begin()
{

    networkpreferences.begin("credentials", false);
    uint32_t PisteNr = networkpreferences.getInt("pisteNr", 304);
    CyranoPort = networkpreferences.getUShort("CyranoPort", CYRANO_PORT);
    CyranoBroadcastPort = networkpreferences.getUShort("CyranoBroadcastPort", CYRANO_BROADCAST_PORT);
    networkpreferences.end();
    char temp[8];
    sprintf(temp,"%d",PisteNr);
    //cout << "PisteNr" << PisteNr << endl;
    m_MachineStatus[PisteId]= (std::string)temp;
    NextPeriodicalUpdate = millis() + 10000;
}

CyranoHandler::~CyranoHandler()
{
    //dtor

}



void CyranoHandler::SendInfoMessage()
{
  if(!bOKToSend)
    return;
  string TheMessage;
  m_MachineStatus[Command] = "INFO";
  TheMessage = m_MachineStatus.ToString(TheMessage);
  //CyranoHandlerudpRcv.broadcastTo((uint8_t*)TheMessage.c_str(),TheMessage.length(), CyranoPort,TCPIP_ADAPTER_IF_STA);

if(false)
  CyranoHandlerudpBroadcast.broadcastTo((uint8_t*)TheMessage.c_str(),TheMessage.length(), CyranoBroadcastPort,TCPIP_ADAPTER_IF_STA);
else
  CyranoHandlerudpRcv.writeTo((uint8_t*)TheMessage.c_str(),TheMessage.length(), SoftwareIPAddress(),CyranoBroadcastPort,TCPIP_ADAPTER_IF_STA);
  //cout << "Sending info message: " << TheMessage << "To:" << SoftwareIPAddress() << ":" << CyranoBroadcastPort <<endl;
  //StateChanged(TheMessage);
  return;
}

void CyranoHandler::ProcessMessageFromSoftware(const EFP1Message &input)
{
    if(input[PisteId] != m_MachineStatus[PisteId])
        return; // wrong Piste

    switch(input.GetType())
    {
        case HELLO :
        if("" == m_MachineStatus[State])
        {
          m_State = WAITING;
          m_MachineStatus[State] = "W";
          StateChanged(EVENT_CYRANO_STATE_W);
        }
        bOKToSend = true;
        bSoftwareIsLive = true;
        LastHelloReception = millis();
        //m_MachineStatus[CompetitionId] = input[CompetitionId];
        SendInfoMessage();

        break;

        case DISP :
        //cout << "received DISP" << endl;
        if(WAITING == m_State)
        {
            // Initialize with the received values
          EFP1Message  temp;
          temp = m_MachineStatus;
          temp.Prune(input);
          string msg;
          temp.ToString(msg);
          StateChanged(msg);
          m_MachineStatus.CopyIfNotEmpty(input);
          m_MachineStatus[State] = "W";
          m_State = WAITING;
          StateChanged(EVENT_CYRANO_STATE_W);
          m_MachineStatus[Command] = "INFO";
        }

        break;

        case ACK :
        if(WAITING != m_State)
        {
            // Initialize with the received values
            m_State = WAITING;
            m_MachineStatus[State] = "W";
            StateChanged(EVENT_CYRANO_STATE_W);
            SendInfoMessage();
        }

        break;

        case NAK :
        // The software doesn't accept the "END" message
        StateChanged(EVENT_CYRANO_STATE_NAK);
        break;
        cout << "Interesting, I should never ever get here" << endl;

    }

}

void CyranoHandler::ProcessUIEvents(uint32_t const event)
{
    uint32_t event_data = event & SUB_TYPE_MASK;

    switch(event_data)
    {
        case UI_INPUT_CYRANO_NEXT :
        bOKToSend = true;
        if(WAITING == m_State)
        {
            string TheMessage = m_MachineStatus.MakeNextMessageString();
            //CyranoHandlerudpRcv.writeTo((uint8_t*)TheMessage.c_str(),TheMessage.length(), IPAddress(10,154,1,109),CYRANO_PORT,TCPIP_ADAPTER_IF_STA);
            //CyranoHandlerudpRcv.broadcastTo((uint8_t*)TheMessage.c_str(),TheMessage.length(), CyranoBroadcastPort,TCPIP_ADAPTER_IF_STA);
            if(false)
              CyranoHandlerudpBroadcast.broadcastTo((uint8_t*)TheMessage.c_str(),TheMessage.length(), CyranoBroadcastPort,TCPIP_ADAPTER_IF_STA);
            else
              CyranoHandlerudpRcv.writeTo((uint8_t*)TheMessage.c_str(),TheMessage.length(), SoftwareIPAddress(),CyranoBroadcastPort,TCPIP_ADAPTER_IF_STA);

            StateChanged(EVENT_CYRANO_STATE_W);
        }
        break;

        case UI_INPUT_CYRANO_PREV :
        bOKToSend = true;
        if(WAITING == m_State)
        {
          string TheMessage = m_MachineStatus.MakePrevMessageString();
          //CyranoHandlerudpRcv.writeTo((uint8_t*)TheMessage.c_str(),TheMessage.length(), IPAddress(10,154,1,109),CYRANO_PORT,TCPIP_ADAPTER_IF_STA);
          //CyranoHandlerudpRcv.broadcastTo((uint8_t*)TheMessage.c_str(),TheMessage.length(), CyranoBroadcastPort,TCPIP_ADAPTER_IF_STA);
          if(false)
            CyranoHandlerudpBroadcast.broadcastTo((uint8_t*)TheMessage.c_str(),TheMessage.length(), CyranoBroadcastPort,TCPIP_ADAPTER_IF_STA);
          else
            CyranoHandlerudpRcv.writeTo((uint8_t*)TheMessage.c_str(),TheMessage.length(), SoftwareIPAddress(),CyranoBroadcastPort,TCPIP_ADAPTER_IF_STA);
          StateChanged(EVENT_CYRANO_STATE_W);
        }
        break;

        case UI_INPUT_CYRANO_BEGIN :
        bOKToSend = true;
        if(WAITING == m_State)
        {
          m_State = HALT;
          m_MachineStatus[State] = "H";
          StateChanged(EVENT_CYRANO_STATE_H);
          SendInfoMessage();
        }
        break;

        case UI_INPUT_CYRANO_END :
        bOKToSend = true;
        if(WAITING != m_State)
        {
          m_State = ENDING;
          m_MachineStatus[State] = "E";
          StateChanged(EVENT_CYRANO_STATE_E);
          SendInfoMessage();

        }
        break;

        case  UI_SWAP_FENCERS :
        bOKToSend = true;
        {
          m_MachineStatus.SwapFencersInclScoreCardsEtc();
          string msg;
          m_MachineStatus.ToString(msg);
          StateChanged(msg);
          SendInfoMessage();
        }

        break;

        case UI_RESERVE_LEFT:
        bOKToSend = true;
        {
          if(m_MachineStatus[CompetitionType] == "T")
          {
            m_MachineStatus.HandleTeamReserve(true,true);
            string msg;
            m_MachineStatus.ToString(msg);
            StateChanged(msg);
            SendInfoMessage();
          }
        }
        break;
        case UI_RESERVE_RIGHT:
        bOKToSend = true;
        {
          if(m_MachineStatus[CompetitionType] == "T")
          {
            m_MachineStatus.HandleTeamReserve(false,true);
            string msg;
            m_MachineStatus.ToString(msg);
            StateChanged(msg);
            SendInfoMessage();
          }
        }
        break;

        case  UI_ABANDON_LEFT :
        bOKToSend = true;
        {
          m_MachineStatus[LeftStatus] = "A";
          string msg;
          m_MachineStatus.ToString(msg);
          StateChanged(msg);
          SendInfoMessage();
        }

        case  UI_ABANDON_RIGHT :
        bOKToSend = true;
        {
          m_MachineStatus[RightStatus] = "A";
          string msg;
          m_MachineStatus.ToString(msg);
          StateChanged(msg);
          SendInfoMessage();
        }

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

    case EVENT_TIMER_STATE:
    if((m_MachineStatus[State] == "E") || (m_MachineStatus[State] == "W"))
      break;
      if(eventtype & DATA_24BIT_MASK)
      {
        //Message2.SetTimerStatus('R');
        m_MachineStatus[State] = "F";
        StateChanged(EVENT_CYRANO_STATE_F);

      }
      else
      {
        //Message2.SetTimerStatus('N');
        m_MachineStatus[State] = "H";
        StateChanged(EVENT_CYRANO_STATE_H);

      }
    break;
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

    case EVENT_BLACK_CARD_LEFT:
    {
      if(event_data)
        m_MachineStatus[RightStatus] = "E";
      else
        m_MachineStatus[RightStatus] = "U";
    }
    //cout << "Right competitor status = " << m_MachineStatus[RightStatus] << endl;
    break;

    case EVENT_BLACK_CARD_RIGHT:
    {
      if(event_data)
        m_MachineStatus[LeftStatus] = "E";
      else
        m_MachineStatus[LeftStatus] = "U";
    }
    //cout << "Left competitor status = " << m_MachineStatus[LeftStatus]<< endl;
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
    SendInfoMessage();
  }
}

void ProcessCyranoPacket (AsyncUDPPacket packet)
{

  // If Software is live, we know the IP address. If the received packet does
  // come from the Software, we can ignore it.
  CyranoHandler &MyCyranoHandler = CyranoHandler::getInstance();
  if(MyCyranoHandler.SoftwareIsLive())
  {
    if(MyCyranoHandler.SoftwareIPAddress() != packet.remoteIP())
      return;
  }
  else
  {
    if(!strncmp((char*)packet.data()+8,"HELLO",5))
    {
      MyCyranoHandler.SoftwareIPAddress(packet.remoteIP());
    }
    else
      return;
  }
  MyCyranoHandler.ProcessMessageFromSoftware((EFP1Message((char*)packet.data())));
}

void CyranoHandler::CheckConnection()
{
  if(bCyranoConnected){
    if(bSoftwareIsLive){
      if(LastHelloReception + 40000 < millis()){
        bSoftwareIsLive = false;
        StateChanged(EVENT_CYRANO_STATE_W);
      }
    }
    return;
  }

  /*if(millis() < NextTimeToCheckConnection)
    return;
  NextTimeToCheckConnection = millis() + 2500;*/
  //Serial.println("Checking WiFi connection");
  if(WiFi.status() == WL_CONNECTED)
  {
    if(!bWifiConnected)
    {
      bWifiConnected = true;
    }
    //Serial.println("Wifi is connected");
    if(!bCyranoConnected)
    {// Somehow we should call this only once. It will keep on trying for ever.

      if(CyranoHandlerudpRcv.listen(CyranoPort))
      {
        ESP_LOGI(CYRANO_TAG, "%s","Cyrano Listening on IP: ");
        ESP_LOGI(CYRANO_TAG, "%s",(WiFi.localIP().toString()).c_str());
        CyranoHandlerudpRcv.onPacket([](AsyncUDPPacket packet) {
          ProcessCyranoPacket (packet);
        });
      }
      bCyranoConnected = true;
    }

  }
}

void CyranoHandler::PeriodicallyBroadcastStatus()
{
  if(!bOKToSend)
    return;
  if(NextPeriodicalUpdate > millis())
    return;
  NextPeriodicalUpdate = millis() + 17000;
  SendInfoMessage();
  return;

}
