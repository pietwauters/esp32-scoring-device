//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#ifndef EVENTDEFINITIONS_H
#define EVENTDEFINITIONS_H
// Typedefinition to access individual bytes of a 32 bit unsigned
union mix_t
{
    std::uint32_t theDWord;
    std::uint8_t theBytes[4];
};

// An event is represented as a 32 bit unsigned integer
// The most significant 8 bits represent the Main Type
// The least significant 24 bits represent either
// A subtype of the Main Type or
// The data corresponding to the event of Main Type

#define MAIN_TYPE_MASK  0xff000000
#define SUB_TYPE_MASK  0x00ffffff
#define DATA_24BIT_MASK  0x00ffffff
#define DATA_BYTE0_MASK  0x000000ff
#define DATA_BYTE1_MASK  0x0000ff00
#define DATA_BYTE2_MASK  0x00ff0000
#define EVENT_ALL 0xffffffff

// The next section contains all the main event types

#define EVENT_LIGHTS 0x00000000
#define EVENT_TIMER 0x01000000
#define EVENT_ROUND 0x02000000
#define EVENT_COMPETITOR_STATUS 0x03000000
#define EVENT_WEAPON 0x04000000
#define EVENT_COMPETITOR 0x05000000
#define EVENT_UI_INPUT 0x06000000

#define EVENT_SCORE_LEFT 0x07000000
#define EVENT_SCORE_RIGHT 0x08000000
#define EVENT_PRIO 0x09000000
#define EVENT_YELLOW_CARD_LEFT 0x0a000000
#define EVENT_YELLOW_CARD_RIGHT 0x0b000000
#define EVENT_RED_CARD_LEFT 0x0c000000
#define EVENT_RED_CARD_RIGHT 0x0d000000
#define EVENT_BLACK_CARD_LEFT 0x0e000000
#define EVENT_BLACK_CARD_RIGHT 0x0f000000
#define EVENT_TIMER_STATE 0x10000000
#define EVENT_IDLE_100_MS 0x11000000
#define EVENT_P_CARD 0x12000000
#define EVENT_UW2F_TIMER 0x13000000
#define EVENT_TOGGLE_BUZZER 0x14000000

// Below are mask definitions to get the actual weapon from the weapon event
#define WEAPON_MASK_FOIL 0x00000001
#define WEAPON_MASK_EPEE 0x00000002
#define WEAPON_MASK_SABRE 0x00000004
#define WEAPON_MASK_UNKNOWN 0x00000008

// Below are event definitions related to User Interface (leds, displays, ... keys, remote control, ...)
// Events generated by "input" devices e.g. keys, remote control, BLYNK io, ... (input towards the state machine)
// These are subtypes

#define UI_INPUT_START_TIMER 0x00000001
#define UI_INPUT_STOP_TIMER 0x00000002
#define UI_INPUT_RESET 0x00000003
#define UI_INPUT_INCR_SCORE_LEFT 0x00000005
#define UI_INPUT_DECR_SCORE_LEFT 0x00000006
#define UI_INPUT_INCR_SCORE_RIGHT 0x00000007
#define UI_INPUT_DECR_SCORE_RIGHT 0x00000008
#define UI_INPUT_ROUND 0x0000000a
#define UI_INPUT_PRIO  0x00000010
#define UI_INPUT_TOGGLE_TIMER 0x00000011
#define UI_INPUT_CYCLE_WEAPON 0x00000012
#define UI_INPUT_YELLOW_CARD_LEFT 0x00000013
#define UI_INPUT_YELLOW_CARD_RIGHT 0x00000014
#define UI_INPUT_RED_CARD_LEFT 0x00000015
#define UI_INPUT_RED_CARD_RIGHT 0x00000016
#define UI_INPUT_P_CARD 0x00000017
#define UI_BUZZ 0x00000018



// Below are the subtype definitions for P-Card related events
#define EVENT_NO_P_CARD 0x00000000
#define EVENT_YELLOW_P_CARD 0x00000001
#define EVENT_RED_P_CARD1 0x00000002
#define EVENT_RED_P_CARD2 0x00000003
#define EVENT_BLACK_P_CARD 0x00000004
#define EVENT_BLACK_P_CARD2 0x00000005


#endif // EVENTDEFINITIONS_H
