/*
 * simon_setup.h - Header file for simon project buzzer and light control
 */

#ifndef simon_av_include
#define simon_av_include

#include <ti/devices/msp/msp.h>

#ifdef __cplusplus
extern "C" {
#endif



/*
 * @brief Calculates MIDI notes.
 */
void InitializeMIDINotes(void);

/*
 * @brief Starts playing piezo buzzer at the frequency of given MIDI note.
 */
void startNote(int8_t midi_note);

/*
 * @brief Stops the piezo buzzer.
 */
void stopNote(void);

/*
 * @brief Writes packet to turn LEDs on/off, depending on bool value.
 */
void writeLights(uint16_t *packetptr, bool lights[4]);

// Note durations are in multiples of 10ms
// 10ms are always added to the end of each note
// PAUSE is added on top
// So quarter notes will last 10ms * (QUARTER + PAUSE + 1)
#define EIGHTH 20
#define QUARTER 45
#define HALF 95
#define WHOLE 195
#define PAUSE 4
// Used for playing the game
#define GAME_PULSE 50
#define GAME_PAUSE 49

#define NOTE_OFFSET 0 // Offset game note pitches by semitones

// Struct representing each frame of animation (duration, note, lights)
struct frame {
    uint16_t duration;
    int8_t note;
    bool lights[4];
};

// Arrays for animations (lights and sound)
extern struct frame INTRO_FRAMES[];
extern uint16_t INTRO_LENGTH;



#ifdef __cplusplus
}
#endif

#endif // simon_av_include
