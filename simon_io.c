/*
 * simon_setup.h - Source file for simon project buzzer and light control
 */

#include <math.h>
#include <string.h>
#include "simon_io.h"

int MIDI_NOTE_PERIODS[127];

uint16_t OFF_BITS[] = {0x0, 0x0,
                       0xE000, 0x0000,
                       0xE000, 0x0000,
                       0xE000, 0x0000,
                       0xE000, 0x0000,
                       0xFFFF, 0xFFFF};
uint16_t BLUE_BITS[] = {0xE550, 0x1010};
uint16_t RED_BITS[] = {0xE500, 0x0050};
uint16_t GREEN_BITS[] = {0xE500, 0x5000};
uint16_t YELLOW_BITS[] = {0xE500, 0x4040};



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



void writeLights(uint16_t *packetptr, bool lights[4]) {
    // Write an OFF packet first, then populate with lit LEDs
    // lights[4]: {blue, red, green, yellow}
    memcpy(packetptr, OFF_BITS, sizeof(OFF_BITS));
    if (lights[0])
        memcpy(packetptr+2, BLUE_BITS, sizeof(BLUE_BITS));
    if (lights[1])
        memcpy(packetptr+4, RED_BITS, sizeof(RED_BITS));
    if (lights[2])
        memcpy(packetptr+6, GREEN_BITS, sizeof(GREEN_BITS));
    if (lights[3])
        memcpy(packetptr+8, YELLOW_BITS, sizeof(YELLOW_BITS));
}
