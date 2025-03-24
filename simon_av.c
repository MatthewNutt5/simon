/*
 * simon_setup.h - Source file for simon project buzzer and light control
 */

#include <math.h>
#include <string.h>
#include "simon_av.h"

uint32_t MIDI_NOTE_PERIODS[127];

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
    for (uint8_t i = 0; i < 128; i++) {
        MIDI_NOTE_PERIODS[i] = (uint32_t) (8000000.0f / (8.1758f * powf(2.0f,(i/12.0f)))); // Math to determine note periods
    }
    return;
}



void startNote(int8_t midi_note) {
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



// Arrays for animations (lights and sound)
// {duration, midi note (-1 for silence), {blue, red, green, yellow}}
/*
struct frame INTRO_FRAMES[] = {
                               (struct frame){HALF, 73, {0, 0, 1, 0}},
                               (struct frame){QUARTER, 71, {0, 1, 0, 0}},
                               (struct frame){QUARTER, 69, {0, 0, 0, 1}},

                               (struct frame){DOUBLE_WHOLE, 68, {1, 1, 1, 1}},
                               (struct frame){WHOLE, -1, {0, 0, 0, 0}},

                               (struct frame){QUARTER, 66, {0, 0, 0, 1}},
                               (struct frame){QUARTER, 68, {1, 0, 0, 0}},
                               (struct frame){QUARTER, 69, {0, 0, 1, 0}},
                               (struct frame){QUARTER, 71, {0, 1, 0, 0}},

                               (struct frame){DOUBLE_WHOLE, 68, {1, 1, 1, 1}},
                               (struct frame){WHOLE, -1, {0, 0, 0, 0}},

                               (struct frame){QUARTER, 66, {1, 0, 0, 0}},
                               (struct frame){QUARTER, 68, {0, 0, 0, 1}},
                               (struct frame){QUARTER, 69, {0, 1, 0, 0}},
                               (struct frame){QUARTER, 71, {0, 0, 1, 0}},

                               (struct frame){DOUBLE_WHOLE, 68, {1, 1, 1, 1}},
                               (struct frame){HALF, -1, {0, 0, 0, 0}},

                               (struct frame){QUARTER, 63, {0, 0, 0, 1}},
                               (struct frame){QUARTER, 64, {0, 0, 1, 0}},
                               (struct frame){HALF, 66, {0, 1, 0, 0}},
                               (struct frame){QUARTER, 64, {0, 0, 1, 0}},
                               (struct frame){QUARTER, 63, {0, 0, 0, 1}},

                               (struct frame){HALF, 61, {1, 0, 0, 1}},

                               (struct frame){DOUBLE_WHOLE, 68, {1, 1, 1, 1}},
                               (struct frame){WHOLE, -1, {0, 0, 0, 0}},
};
*/
struct frame INTRO_FRAMES[] = {
                               (struct frame){EIGHTH, 52, {1, 0, 0, 0}},
                               (struct frame){EIGHTH, 52, {1, 0, 0, 0}},
                               (struct frame){EIGHTH, 64, {0, 1, 0, 0}},
                               (struct frame){EIGHTH, 52, {1, 0, 0, 0}},
                               (struct frame){EIGHTH, 52, {1, 0, 0, 0}},
                               (struct frame){EIGHTH, 62, {0, 0, 1, 0}},
                               (struct frame){EIGHTH, 52, {1, 0, 0, 0}},
                               (struct frame){EIGHTH, 52, {1, 0, 0, 0}},
                               (struct frame){EIGHTH, 60, {0, 0, 0, 1}},
                               (struct frame){EIGHTH, 52, {1, 0, 0, 0}},
                               (struct frame){EIGHTH, 52, {1, 0, 0, 0}},
                               (struct frame){EIGHTH, 58, {0, 0, 0, 1}},
                               (struct frame){EIGHTH, 52, {1, 0, 0, 0}},
                               (struct frame){EIGHTH, 52, {1, 0, 0, 0}},
                               (struct frame){EIGHTH, 59, {0, 0, 0, 1}},
                               (struct frame){EIGHTH, 60, {0, 0, 1, 0}},

                               (struct frame){EIGHTH, 52, {1, 0, 0, 0}},
                               (struct frame){EIGHTH, 52, {1, 0, 0, 0}},
                               (struct frame){EIGHTH, 64, {0, 1, 0, 0}},
                               (struct frame){EIGHTH, 52, {1, 0, 0, 0}},
                               (struct frame){EIGHTH, 52, {1, 0, 0, 0}},
                               (struct frame){EIGHTH, 62, {0, 0, 1, 0}},
                               (struct frame){EIGHTH, 52, {1, 0, 0, 0}},
                               (struct frame){EIGHTH, 52, {1, 0, 0, 0}},
                               (struct frame){EIGHTH, 60, {0, 0, 0, 1}},
                               (struct frame){EIGHTH, 52, {1, 0, 0, 0}},
                               (struct frame){EIGHTH, 52, {1, 0, 0, 0}},
                               (struct frame){HALF_AND_EIGHTH, 58, {0, 1, 0, 1}}
};
uint16_t INTRO_LENGTH = sizeof(INTRO_FRAMES) / sizeof(struct frame);

struct frame WIN_FRAMES[] = {
                             (struct frame){QUARTER, -1, {0, 0, 0, 0}},

                             (struct frame){EIGHTH, 72, {0, 0, 0, 1}},
                             (struct frame){EIGHTH, 76, {1, 0, 0, 0}},
                             (struct frame){EIGHTH, 79, {0, 1, 0, 0}},
                             (struct frame){EIGHTH, -1, {0, 0, 0, 0}},

                             (struct frame){EIGHTH, 76, {1, 0, 0, 0}},
                             (struct frame){EIGHTH, 79, {0, 0, 0, 1}},
                             (struct frame){EIGHTH, 84, {0, 0, 1, 0}},
                             (struct frame){EIGHTH, -1, {0, 0, 0, 0}},

                             (struct frame){EIGHTH, 79, {0, 0, 1, 1}},
                             (struct frame){EIGHTH, 84, {1, 1, 0, 0}},
                             (struct frame){EIGHTH, 91, {0, 1, 1, 0}},
                             (struct frame){EIGHTH, -1, {0, 0, 0, 0}},

                             (struct frame){EIGHTH, 96, {1, 1, 1, 1}},
                             (struct frame){QUARTER, 96, {1, 1, 1, 1}},

                             (struct frame){WHOLE, -1, {0, 0, 0, 0}}
};
uint16_t WIN_LENGTH = sizeof(WIN_FRAMES) / sizeof(struct frame);

struct frame LOSS_FRAMES[] = {
                              (struct frame){EIGHTH, 60, {0, 0, 0, 1}},
                              (struct frame){EIGHTH, 63, {1, 0, 0, 0}},
                              (struct frame){EIGHTH, 60, {0, 0, 0, 1}},
                              (struct frame){EIGHTH, 63, {1, 0, 0, 0}},
                              (struct frame){EIGHTH, 60, {0, 0, 0, 1}},
                              (struct frame){EIGHTH, 63, {1, 0, 0, 0}},
                              (struct frame){QUARTER, 60, {0, 0, 0, 1}},

                              (struct frame){WHOLE, -1, {0, 0, 0, 0}},
};
uint16_t LOSS_LENGTH = sizeof(LOSS_FRAMES) / sizeof(struct frame);
