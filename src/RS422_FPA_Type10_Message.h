//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#ifndef RS422_FPA_TYPE10_MESSAGE_H
#define RS422_FPA_TYPE10_MESSAGE_H

#include "RS422_FPA_Message.h"


class RS422_FPA_Type10_Message : public RS422_FPA_Message
{
    public:
        RS422_FPA_Type10_Message();
        virtual ~RS422_FPA_Type10_Message();
        RS422_FPA_Type10_Message(const RS422_FPA_Type10_Message& other);
        void SetCyranoStatus(const uint8_t TheStatus);
        void SetMachineStatus( const char statevalue);
        uint8_t GetMachineStatus(){return m_message[7];};
        void SetPiste(int PisteNr);
        void SetIPAddress(uint32_t IPAddress);

    protected:

    private:
};

#endif // RS422_FPA_TYPE10_MESSAGE_H
