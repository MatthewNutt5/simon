/*
 * simon_setup.h - Source file for simon project buzzer control
 */

#include <math.h>
#include "simon_buzzer.h"

int MIDI_NOTE_PERIODS[127];



void InitializeMIDINotes(void) {
    for (char i = 0; i < 128; i++) {
        MIDI_NOTE_PERIODS[i] = (int) (8000000.0f / (8.1758f * powf(2.0f,(i/12.0f))));
    }
    return;
}



void startNote(char midi_note) {
    TIMA1->COUNTERREGS.LOAD = (MIDI_NOTE_PERIODS[midi_note] - 1); // freq = 8000000/(LOAD+1)
    TIMA1->COUNTERREGS.CC_01[0] = (TIMA1->COUNTERREGS.LOAD  + 1) / 8; // 12.5% duty cycle because it sounds cool
    TIMA1->COUNTERREGS.CTRCTL |= (GPTIMER_CTRCTL_EN_ENABLED); // Enable the buzzer
    return;
}



void stopNote(void) {
    TIMA1->COUNTERREGS.CTRCTL &= ~(GPTIMER_CTRCTL_EN_ENABLED); // Disable the buzzer
    return;
}
