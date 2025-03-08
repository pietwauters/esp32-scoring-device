#include "EFP1Message.h"
#include <sstream>
#include <iostream>

EFP1Message::EFP1Message()
{
    //ctor
    for(int i=0; i < GetNrOfGeneralFields(); i++)
        mGeneralFields.push_back("");
    mGeneralFields[Protocol] = "EFP1.1";
    mGeneralFields[Command] = "INFO";
    for(int i=0; i < GetNrOfFencerFields(); i++)
        mLeftFencerFields.push_back("");
    for(int i=0; i < GetNrOfFencerFields(); i++)
        mRightFencerFields.push_back("");
}

EFP1Message::~EFP1Message()
{
    //dtor

}


EFP1Message& EFP1Message::operator=(const EFP1Message& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    //assignment operator
    for(int i=0; i < MAX_NR_FIELDS; i++)
    {
        (*this)[i] = rhs[i];
    }
    return *this;
}

int const EFP1Message::GetNrOfFencerFields() const
{
   if(mGeneralFields[Protocol] == "EFP1.1")
        return 12;
   else
        return 11;
}

std::string EFP1Message::ToString(std::string & Buffer)
{
    Buffer = "|";
    for(int i = 0; i < GetNrOfGeneralFields() ; i++)
    {
        Buffer = Buffer +  mGeneralFields[i] + "|";
    }
    Buffer += "%|";

    for(int i = 0; i < GetNrOfFencerFields() ; i++)
    {
        Buffer = Buffer +  mRightFencerFields[i] + "|";
    }
    Buffer +=  "%|";

    for(int i = 0; i < GetNrOfFencerFields() ; i++)
    {
         Buffer = Buffer +  mLeftFencerFields[i] + "|";
    }
    Buffer +=  "%|";
    /*if(!(Buffer.Len() < 211))
        wxLogError("Aha! This is not good: Buffer should never exceed 210");*/
    return Buffer;
}

std::string & EFP1Message::operator [](int i)
{
    // Todo: if you use the current field names, it is assumed that the 1.1 version is used, (So 41 fields: 17 general and 12 for right and 12 for left.
    // So for the Left fencer
    if((i < 0) || (i > MAX_NR_FIELDS -1))
        return mGeneralFields[0];
    else
    {
        if(i < GetNrOfGeneralFields())
        {
            return mGeneralFields[i];
        }
        else
        {
            if(i >= GetNrOfFencerFields() + GetNrOfGeneralFields())
            {
                int correction = 0;
                if(mGeneralFields[0] == "EFP1")
                    correction = 1;
                return mLeftFencerFields[i-GetNrOfFencerFields() - GetNrOfGeneralFields() - correction];
            }
            else
            {
                return mRightFencerFields[i - GetNrOfGeneralFields()];
            }
        }
    }

}

const std::string & EFP1Message::operator [](int i) const
{

    if((i < 0) || (i > MAX_NR_FIELDS -1))
        return mGeneralFields[0];
    else
    {
        if(i < GetNrOfGeneralFields())
        {
            return mGeneralFields[i];
        }
        else
        {
            if(i >= GetNrOfFencerFields() + GetNrOfGeneralFields())
            {
                return mLeftFencerFields[i-GetNrOfFencerFields() - GetNrOfGeneralFields()];
            }
            else
            {
                return mRightFencerFields[i - GetNrOfGeneralFields()];
            }
        }
    }
}

EFP1Message::EFP1Message(const std::string &Buffer)
{

    // Vector of string to save tokens
    char dummy;
    std::vector <std::string> main_areas;
    std::stringstream BufferStream(Buffer);
    BufferStream >> dummy; // used to ignore the initial '|' character

    std::string intermediate;

    //Start by finding the 3 main areas
    while(getline(BufferStream, intermediate, '%'))
    {
        main_areas.push_back(intermediate);

    }


    std::stringstream GeneralFields(main_areas[0]);
    while(getline(GeneralFields, intermediate, '|'))
    {
        mGeneralFields.push_back(intermediate);

    }
    for(int i = mGeneralFields.size(); i < GetNrOfGeneralFields(); i++)
    {
      mGeneralFields.push_back("");
    }
    mGeneralFields[0] = "EFP1.1";

    if(main_areas.size() > 2)
    {
        std::stringstream LeftFencerFields(main_areas[2]);
        LeftFencerFields >> dummy; // used to ignore the initial '|' character
        while(getline(LeftFencerFields, intermediate, '|'))
        {
            mLeftFencerFields.push_back(intermediate);

        }
        if(mLeftFencerFields.size() != 12)
          mLeftFencerFields.push_back("");

        std::stringstream RightFencerFields(main_areas[1]);
        RightFencerFields >> dummy; // used to ignore the initial '|' character
        while(getline(RightFencerFields, intermediate, '|'))
        {
            mRightFencerFields.push_back(intermediate);

        }
        if(mRightFencerFields.size() != 12)
          mRightFencerFields.push_back("");

    }


    //check here if the length is OK for this field
    /*for(int i=0; i < MAX_NR_FIELDS; i++)
    {
        Fields[i].Truncate(EPF1SubMessageLengths[i]);
    }*/

}


string EFP1Message::MakeNextMessageString()
{
    string message = "|" + (*this)[Protocol] + "|NEXT|" ;
    message = message + (*this)[PisteId] + "|" + (*this)[CompetitionId] + "|%|";
    return message;

}
string EFP1Message::MakePrevMessageString()
{
    string message = "|" + (*this)[Protocol] + "|PREV|" ;
    message = message + (*this)[PisteId] + "|" + (*this)[CompetitionId] + "|%|";
    return message;
}

MessageType EFP1Message::GetType() const
{
    if("HELLO" == (*this)[Command])
        return HELLO;

    if("DISP" == (*this)[Command])
        return DISP;

    if("ACK" == (*this)[Command])
        return ACK;

    if("NAK" == (*this)[Command])
        return NAK;

    if("INFO" == (*this)[Command])
        return INFO;

    if("NEXT" == (*this)[Command])
        return NEXT;

    if("PREV" == (*this)[Command])
        return PREV;
    return ERROR;
}


void EFP1Message::CopyIfNotEmpty(const EFP1Message &Source)
{// we should check if the versions are equal
    for(int i=0; i < MAX_NR_FIELDS; i++)
    {
        if(Source[i] != "")
        {
            (*this)[i] = Source[i];
        }
    }
}

void EFP1Message::Prune(const EFP1Message &Source)
{// we should check if the versions are equal

    for(int i=CompetitionId; i < MAX_NR_FIELDS; i++)
    {
        if((*this)[i] == Source[i])
        {
            (*this)[i] = "";

        }
        else
        {
          (*this)[i] = Source[i];
        }

    }
}


void EFP1Message::SwapFencersInclScoreCardsEtc()
{
  std::vector<std::string> temp = mRightFencerFields;
  mRightFencerFields = mLeftFencerFields;
  mLeftFencerFields = temp;
}

void EFP1Message::HandleTeamReserve(bool left, bool value)
{

  if(left)
  {
    if(value)
    {
      (*this)[LeftReserveIntroduction] = "R";
    }
    else
    {
      (*this)[LeftReserveIntroduction] = "N";
    }

  }
  else
  {
    if(value)
    {
      (*this)[RightReserveIntroduction] = "R";
    }
    else
    {
      (*this)[RightReserveIntroduction] = "N";
    }

  }
}

uint8_t EFP1Message::EFP1StatusString2Type10MessageStatus()
{
  std::string EFP1Status = (*this)[State];
  //cout << "EFP1Status: "<< EFP1Status<< endl;
  if("F" == EFP1Status)
    return 'F';
  if("H" == EFP1Status)
    return 'H';
  if("P" == EFP1Status)
    return 'P';
  if("W" == EFP1Status)
    return 'W';
  if("E" == EFP1Status)
    return 'E';
  // none of the above, retrun unknown
  return 'U';
}

/*
void const EFP1Message::print()const {

  std::cout << " Protocol: " << (*this)[Protocol] << std::endl;
std::cout << " Command: " << (*this)[Command] << std::endl;
std::cout << " PisteId: " << (*this)[PisteId] << std::endl;
std::cout << " CompetitionId: " << (*this)[CompetitionId] << std::endl;
std::cout << " PhaseNumber: " << (*this)[PhaseNumber] << std::endl;
std::cout << " Poule_Tableau_Id: " << (*this)[Poule_Tableau_Id] << std::endl;
std::cout << " MatchNumber: " << (*this)[MatchNumber] << std::endl;
std::cout << " RoundNumber: " << (*this)[RoundNumber] << std::endl;
std::cout << " Start_time: " << (*this)[Start_time] << std::endl;
std::cout << " StopWatch: " << (*this)[StopWatch] << std::endl;
std::cout << " CompetitionType: " << (*this)[CompetitionType] << std::endl;
std::cout << " Weapon: " << (*this)[Weapon] << std::endl;
std::cout << " Priority: " << (*this)[Priority] << std::endl;
std::cout << " State: " << (*this)[State] << std::endl;
std::cout << " RefereeId: " << (*this)[RefereeId] << std::endl;
std::cout << " RefereeName: " << (*this)[RefereeName] << std::endl;
std::cout << " RefereeNation: " << (*this)[RefereeNation] << std::endl;
std::cout << " RightFencerId: " << (*this)[RightFencerId] << std::endl;
std::cout << " RightFencerName: " << (*this)[RightFencerName] << std::endl;
std::cout << " RightFencerNation: " << (*this)[RightFencerNation] << std::endl;
std::cout << " RightScore: " << (*this)[RightScore] << std::endl;
std::cout << " RightStatus: " << (*this)[RightStatus] << std::endl;
std::cout << " RightYCard: " << (*this)[RightYCard] << std::endl;
std::cout << " RightRCard: " << (*this)[RightRCard] << std::endl;
std::cout << " RightLight: " << (*this)[RightLight] << std::endl;
std::cout << " RightWhiteLight: " << (*this)[RightWhiteLight] << std::endl;
std::cout << " RightMedicalIntervention: " << (*this)[RightMedicalIntervention] << std::endl;
std::cout << " RightReserveIntroduction: " << (*this)[RightReserveIntroduction] << std::endl;
std::cout << " RightPCards: " << (*this)[RightPCards] << std::endl;
std::cout << " LeftFencerId: " << (*this)[LeftFencerId] << std::endl;
std::cout << " LeftFencerName: " << (*this)[LeftFencerName] << std::endl;
std::cout << " LeftFencerNation: " << (*this)[LeftFencerNation] << std::endl;
std::cout << " LeftScore: " << (*this)[LeftScore] << std::endl;
std::cout << " LeftStatus: " << (*this)[LeftStatus] << std::endl;
std::cout << " LeftYCard: " << (*this)[LeftYCard] << std::endl;
std::cout << " LeftRCard: " << (*this)[LeftRCard] << std::endl;
std::cout << " LeftLight: " << (*this)[LeftLight] << std::endl;
std::cout << " LeftWhiteLight: " << (*this)[LeftWhiteLight] << std::endl;
std::cout << " LeftMedicalIntervention: " << (*this)[LeftMedicalIntervention] << std::endl;
std::cout << " LeftReserveIntroduction: " << (*this)[LeftReserveIntroduction] << std::endl;
std::cout << " LeftPCards: " << (*this)[LeftPCards] << std::endl;
}
*/
