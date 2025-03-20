/*
 * simon_setup.h - Header file for simon project buzzer control
 */

#ifndef simon_buzzer_include
#define simon_buzzer_include

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
void startNote(char midi_note);

/*
 * @brief Stops the piezo buzzer.
 */
void stopNote(void);



#ifdef __cplusplus
}
#endif

#endif // simon_buzzer_include
