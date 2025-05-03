#ifndef EFP1Message_H
#define EFP1Message_H
#include <vector>
#include <string>

#define MAX_NR_FIELDS 41
// using namespace std;


enum EPF1SubMessage
{
    Protocol = 0,
    Command,
    PisteId,
    CompetitionId,
    PhaseNumber,
    Poule_Tableau_Id,
    MatchNumber,
    RoundNumber,
    Start_time,
    StopWatch,
    CompetitionType,
    Weapon,
    Priority,
    State,
    RefereeId,
    RefereeName,
    RefereeNation,
    RightFencerId,
    RightFencerName,
    RightFencerNation,
    RightScore,
    RightStatus,
    RightYCard,
    RightRCard,
    RightLight,
    RightWhiteLight,
    RightMedicalIntervention,
    RightReserveIntroduction,
    RightPCards,
    LeftFencerId,
    LeftFencerName,
    LeftFencerNation,
    LeftScore,
    LeftStatus,
    LeftYCard,
    LeftRCard,
    LeftLight,
    LeftWhiteLight,
    LeftMedicalIntervention,
    LeftReserveIntroduction,
    LeftPCards
};

enum MessageType
{
    HELLO,
    DISP,
    ACK,
    NAK,
    INFO,
    NEXT,
    PREV,
    ERROR
};

class EFP1Message
{
    public:
        /** Default constructor */
        EFP1Message();
        /** Constructor from string*/
        EFP1Message(const std::string &Buffer);
        /** Default destructor */
        virtual ~EFP1Message();

        /** Assignment operator
         *  \param other Object to assign from
         *  \return A reference to this
         */
        EFP1Message& operator=(const EFP1Message& other);
        std::string ToString(std::string & Buffer);
        std::string & operator[](int i);
        const std::string & operator[](int i) const;
        void CopyIfNotEmpty(const EFP1Message &Source);
        void Prune(const EFP1Message &Source);
        void TruncateToMaxLength(void){return;}  // ToDo make sure the length of the fields is conform to the specification. I don't think I really need it.
        MessageType GetType() const;
        std::string MakeNextMessageString();
        std::string MakePrevMessageString();
        void SetRed(bool value){if(value){(*this)[LeftLight]= '1';}else{(*this)[LeftLight]= '0';}};
        void SetGreen(bool value){if(value){(*this)[RightLight]= '1';}else{(*this)[RightLight]= '0';}};
        void SetWhiteLeft(bool value){if(value){(*this)[LeftWhiteLight]= '1';}else{(*this)[LeftWhiteLight]= '0';}};
        void SetWhiteRight(bool value){if(value){(*this)[RightWhiteLight]= '1';}else{(*this)[RightWhiteLight]= '0';}};

        void SwapFencersInclScoreCardsEtc();
        void HandleTeamReserve(bool left, bool value);
        uint8_t EFP1StatusString2Type10MessageStatus();
        void const print() const;


    protected:

    private:
        std::vector<std::string> mGeneralFields; //!< Member variable "mGeneralFields"
        std::vector<std::string> mLeftFencerFields; //!< Member variable "mLeftFencerFields"
        std::vector<std::string> mRightFencerFields; //!< Member variable "mRightFencerFields"
        int const GetNrOfGeneralFields()const {return 17;}
        int const GetNrOfFencerFields()const ;
        // Overloaded helpers for const and non-const access
    static std::string& accessField(std::vector<std::string>& general,
                                    std::vector<std::string>& left,
                                    std::vector<std::string>& right, int i);

    static const std::string& accessField(const std::vector<std::string>& general,
                                          const std::vector<std::string>& left,
                                          const std::vector<std::string>& right, int i);
};

#endif // EFP1Message_H
