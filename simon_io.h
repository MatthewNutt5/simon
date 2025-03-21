/*
 * simon_setup.h - Header file for simon project buzzer and light control
 */

#ifndef simon_io_include
#define simon_io_include

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

/*
 * @brief Writes packet to turn LEDs on/off, depending on bool value.
 */
void writeLights(uint16_t *packetptr, bool lights[4]);



#ifdef __cplusplus
}
#endif

#endif // simon_buzzer_include
