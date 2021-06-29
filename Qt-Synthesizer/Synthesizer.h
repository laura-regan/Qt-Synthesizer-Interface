#ifndef SYNTHESIZER_H
#define SYNTHESIZER_H


const char NOTE_OFF                = 0x80;
const char NOTE_ON                 = 0x90;
const char POLYPHONIC_KEY_PRESSURE = 0xA0;
const char CONTROL_CHANGE          = 0xB0;
const char PROGRAM_CHANGE          = 0xC0;
const char CHANNEL_PRESSURE        = 0xD0;
const char PITCH_BEND              = 0xE0;


enum MESSAGES
{
    MIDI,
    OSC_A_WAVE_TYPE,
    OSC_A_DETUNE,
    OSC_A_SQUARE_PW,
    OSC_A_MIX,
    OSC_B_WAVE_TYPE,
    OSC_B_DETUNE,
    OSC_B_SQUARE_PW,
    OSC_B_MIX,
    OSC_C_WAVE_TYPE,
    OSC_C_DETUNE,
    OSC_C_SQUARE_PW,
    OSC_C_MIX,
    LFO_A_WAVE_TYPE,
    LFO_A_RATE,
    LFO_A_AMOUNT,
    LFO_B_WAVE_TYPE,
    LFO_B_RATE,
    LFO_B_AMOUNT,
    LFO_C_WAVE_TYPE,
    LFO_C_RATE,
    LFO_C_AMOUNT,
    ADSR_ATTACK,
    ADSR_DECAY,
    ADSR_SUSTAIN,
    ADSR_RELEASE,
    FILTER_TYPE,
    FILTER_CUTOFF,
    FILTER_RESONANCE,
    FILTER_ENVELOPE,
    FILTER_ATTACK,
    FILTER_DECAY,
    FILTER_SUSTAIN,
    FILTER_RELEASE,
    SEQUENCER_RECORD,
    SEQUENCER_STOP,
    SEQUENCER_PLAY_PAUSE,
    SEQUENCER_TEMPO,
    SEQUENCER_TIME_DIV,
    SEQUENCER_GATE

};

typedef struct
{
    int16_t data;
} STD_MESSAGE;

typedef struct
{
    uint8_t status;
    uint8_t key;
    uint8_t velocity;
} MESSAGE_MIDI;

#endif // SYNTHESIZER_H
