#include "CyranoHandler.h"
#include "EFP1Message.h"
#include <sstream>

CyranoHandler::CyranoHandler()
{
    //ctor
    Serial.println("CyranoHandler created");
    m_MachineStatus[PisteId]="1";
    m_MachineStatus[State] ="W";
}

CyranoHandler::~CyranoHandler()
{
    //dtor
}



void CyranoHandler::SendInfoMessage()
{
  string TheMessage;
  m_MachineStatus[Command] = "INFO";
  TheMessage = m_MachineStatus.ToString(TheMessage);
  CyranoHandlerudp.broadcastTo((uint8_t*)TheMessage.c_str(),TheMessage.length(), CYRANO_PORT,TCPIP_ADAPTER_IF_STA);
  cout << "Sending info message: " << TheMessage << endl;

  return;
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
            //m_MachineStatus.CopyIfNotEmpty(input);
            //m_MachineStatus[Command] = "INFO";
        }
        break;

        case ACK :
        if(WAITING != m_State)
        {
            // Initialize with the received values
            m_State = WAITING;
            m_MachineStatus[State] = "W";
            SendInfoMessage();
        }
        break;

        case NAK :
        // The software doesn't accept the "END" message
        break;

    }

}

void CyranoHandler::ProcessUIEvents(uint32_t const event)
{
    uint32_t event_data = event & SUB_TYPE_MASK;
    switch(event_data)
    {
        case UI_INPUT_CYRANO_NEXT :
        if(WAITING == m_State)
        {
            string TheMessage = m_MachineStatus.MakeNextMessageString();
            CyranoHandlerudp.broadcastTo((uint8_t*)TheMessage.c_str(),TheMessage.length(), CYRANO_PORT,TCPIP_ADAPTER_IF_STA);
            cout << "Sending NEXT message: " << TheMessage << endl;
        }
        break;

        case UI_INPUT_CYRANO_PREV :
        if(WAITING == m_State)
        {
          string TheMessage = m_MachineStatus.MakePrevMessageString();
          CyranoHandlerudp.broadcastTo((uint8_t*)TheMessage.c_str(),TheMessage.length(), CYRANO_PORT,TCPIP_ADAPTER_IF_STA);
          cout << "Sending NEXT message: " << TheMessage << endl;
        }
        break;

        case UI_INPUT_CYRANO_BEGIN :
        if(WAITING == m_State)
        {
          m_State = HALT;
          m_MachineStatus[State] = "H";
          SendInfoMessage();
        }
        break;

        case UI_INPUT_CYRANO_END :
        if(WAITING != m_State)
        {
          m_State = ENDING;
          m_MachineStatus[State] = "E";
          SendInfoMessage();
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

void CyranoHandler::ProcessDisplayMessage (const EFP1Message &input)
{

  if(m_MachineStatus[CompetitionId] != input[CompetitionId])
  {
  	if(input[CompetitionId] != "")
  		m_MachineStatus[CompetitionId] = input[CompetitionId];
  }

  if(m_MachineStatus[PhaseNumber] != input[PhaseNumber])
  {
  	if(input[PhaseNumber] != "")
  		m_MachineStatus[PhaseNumber] = input[PhaseNumber];
  }

  if(m_MachineStatus[Poule_Tableau_Id] != input[Poule_Tableau_Id])
  {
  	if(input[Poule_Tableau_Id] != "")
  		m_MachineStatus[Poule_Tableau_Id] = input[Poule_Tableau_Id];
  }

  if(m_MachineStatus[MatchNumber] != input[MatchNumber])
  {
  	if(input[MatchNumber] != "")
  		m_MachineStatus[MatchNumber] = input[MatchNumber];
  }

  if(m_MachineStatus[RoundNumber] != input[RoundNumber])
  {
  	if(input[RoundNumber] != "")
    {
      uint32_t temp = 0;
      sscanf(input[RoundNumber].c_str(),"%d",temp);
      StateChanged(EVENT_ROUND | temp);
    }
  		//m_MachineStatus[RoundNumber] = input[RoundNumber];
      // Generate Event

  }

  if(m_MachineStatus[Start_time] != input[Start_time])
  {
  	if(input[Start_time] != "")
  		m_MachineStatus[Start_time] = input[Start_time];
  }

  if(m_MachineStatus[StopWatch] != input[StopWatch])
  {
  	/*if(input[StopWatch] != "")
  		m_MachineStatus[StopWatch] = input[StopWatch];*/
      // Generate Event
  }

  if(m_MachineStatus[CompetitionType] != input[CompetitionType])
  {
  	if(input[CompetitionType] != "")
  		m_MachineStatus[CompetitionType] = input[CompetitionType];
  }

  if(m_MachineStatus[Weapon] != input[Weapon])
  {
  	if(input[Weapon] != "")
    {

      //StateChanged(EVENT_ROUND | temp);
    }
  		//m_MachineStatus[Weapon] = input[Weapon];
      // Generate Event
  }

  if(m_MachineStatus[Priority] != input[Priority])
  {
  	/*if(input[Priority] != "")
  		m_MachineStatus[Priority] = input[Priority];*/
  }

  /*if(m_MachineStatus[State] != input[State])
  {
  	if(input[State] != "")
  		m_MachineStatus[State] = input[State];
  }*/

  /*if(m_MachineStatus[RefereeId] != input[RefereeId])
  {
  	if(input[RefereeId] != "")
  		m_MachineStatus[RefereeId] = input[RefereeId];
  }

  if(m_MachineStatus[RefereeName] != input[RefereeName])
  {
  	if(input[RefereeName] != "")
  		m_MachineStatus[RefereeName] = input[RefereeName];
  }

  if(m_MachineStatus[RefereeNation] != input[RefereeNation])
  {
  	if(input[RefereeNation] != "")
  		m_MachineStatus[RefereeNation] = input[RefereeNation];
  }*/

  if(m_MachineStatus[RightFencerId] != input[RightFencerId])
  {
  	if(input[RightFencerId] != "")
  		m_MachineStatus[RightFencerId] = input[RightFencerId];
  }

  if(m_MachineStatus[RightFencerName] != input[RightFencerName])
  {
  	if(input[RightFencerName] != "")
  		m_MachineStatus[RightFencerName] = input[RightFencerName];
  }

  if(m_MachineStatus[RightFencerNation] != input[RightFencerNation])
  {
  	if(input[RightFencerNation] != "")
  		m_MachineStatus[RightFencerNation] = input[RightFencerNation];
  }

  if(m_MachineStatus[RightScore] != input[RightScore])
  {
  	/*if(input[RightScore] != "")
  		m_MachineStatus[RightScore] = input[RightScore];*/
  }

  if(m_MachineStatus[RightStatus] != input[RightStatus])
  {
  	/*if(input[RightStatus] != "")
  		m_MachineStatus[RightStatus] = input[RightStatus];*/
  }

  if(m_MachineStatus[RightYCard] != input[RightYCard])
  {
  	/*if(input[RightYCard] != "")
  		m_MachineStatus[RightYCard] = input[RightYCard];*/
  }

  if(m_MachineStatus[RightRCard] != input[RightRCard])
  {
  	/*if(input[RightRCard] != "")
  		m_MachineStatus[RightRCard] = input[RightRCard];*/
  }
/*

  if(m_MachineStatus[RightMedicalIntervention] != input[RightMedicalIntervention])
  {
  	if(input[RightMedicalIntervention] != "")
  		m_MachineStatus[RightMedicalIntervention] = input[RightMedicalIntervention];
  }

  if(m_MachineStatus[RightReserveIntroduction] != input[RightReserveIntroduction])
  {
  	if(input[RightReserveIntroduction] != "")
  		m_MachineStatus[RightReserveIntroduction] = input[RightReserveIntroduction];
  }
*/
  if(m_MachineStatus[RightPCards] != input[RightPCards])
  {
  	/*if(input[RightPCards] != "")
  		m_MachineStatus[RightPCards] = input[RightPCards];*/
  }

  if(m_MachineStatus[LeftFencerId] != input[LeftFencerId])
  {
  	if(input[LeftFencerId] != "")
  		m_MachineStatus[LeftFencerId] = input[LeftFencerId];
  }

  if(m_MachineStatus[LeftFencerName] != input[LeftFencerName])
  {
  	if(input[LeftFencerName] != "")
  		m_MachineStatus[LeftFencerName] = input[LeftFencerName];
  }

  if(m_MachineStatus[LeftFencerNation] != input[LeftFencerNation])
  {
  	if(input[LeftFencerNation] != "")
  		m_MachineStatus[LeftFencerNation] = input[LeftFencerNation];
  }

  if(m_MachineStatus[LeftScore] != input[LeftScore])
  {
  	/*if(input[LeftScore] != "")
  		m_MachineStatus[LeftScore] = input[LeftScore];*/
  }

  if(m_MachineStatus[LeftStatus] != input[LeftStatus])
  {
  	/*if(input[LeftStatus] != "")
  		m_MachineStatus[LeftStatus] = input[LeftStatus];*/
  }

  if(m_MachineStatus[LeftYCard] != input[LeftYCard])
  {
  	/*if(input[LeftYCard] != "")
  		m_MachineStatus[LeftYCard] = input[LeftYCard];*/
  }

  if(m_MachineStatus[LeftRCard] != input[LeftRCard])
  {
  	/*if(input[LeftRCard] != "")
  		m_MachineStatus[LeftRCard] = input[LeftRCard];*/
  }

/*
  if(m_MachineStatus[LeftMedicalIntervention] != input[LeftMedicalIntervention])
  {
  	if(input[LeftMedicalIntervention] != "")
  		m_MachineStatus[LeftMedicalIntervention] = input[LeftMedicalIntervention];
  }

  if(m_MachineStatus[LeftReserveIntroduction] != input[LeftReserveIntroduction])
  {
  	if(input[LeftReserveIntroduction] != "")
  		m_MachineStatus[LeftReserveIntroduction] = input[LeftReserveIntroduction];
  }
*/
  if(m_MachineStatus[LeftPCards] != input[LeftPCards])
  {
  	/*if(input[LeftPCards] != "")
  		m_MachineStatus[LeftPCards] = input[LeftPCards];*/
  }

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

void ProcessCyranoPacket (AsyncUDPPacket packet)
{
  Serial.println("received some data");
  Serial.println((char*)packet.data());
  MyCyranoHandler.ProcessMessageFromSoftware((EFP1Message((char*)packet.data())));
}

void CyranoHandler::CheckConnection()
{
  if(bCyranoConnected)
    return;
  if(millis() < NextTimeToCheckConnection)
    return;
  NextTimeToCheckConnection = millis() + 2500;
  Serial.println("Checking WiFi connection");
  if(WiFi.status() == WL_CONNECTED)
  {
    if(!bWifiConnected)
    {
      bWifiConnected = true;
    }
    Serial.println("Wifi is connected");
    if(!bCyranoConnected)
    {// Somehow we should call this only once. It will keep on trying for ever.

      if(CyranoHandlerudpRcv.listen(50100))
      //if(CyranoHandlerudpRcv.connect(IPAddress(10,154,1,109), 50101))
      {
        Serial.print("Cyrano Listening on IP: ");
        Serial.println(WiFi.localIP());
        CyranoHandlerudpRcv.onPacket([](AsyncUDPPacket packet) {
          ProcessCyranoPacket (packet);
        });

      }
      bCyranoConnected = true;
    }

  }
}
