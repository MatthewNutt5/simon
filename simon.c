#include <ti/devices/msp/msp.h>
#include "simon_setup.h"
#include "simon_av.h"
#include "simon_random.h"



// SPI stuff
uint16_t txPacket[12];
int transmissionComplete = 0; // flag for SPI ISR wakeup
int timerTicked = 0; // flag for timer ISR wakeup
int idx = 0;
int message_len = sizeof(txPacket) / sizeof(txPacket[0]);

// Animation stuff
int16_t timerCount = -1; // Keeps track of number of 10ms interrupts
uint16_t frameNum = 0;
uint16_t timerLength;

// Game stuff
#define MAX_PAT_LENGTH 5 /* MODIFIES # OF STEPS REQUIRED TO WIN */
#define TIMEOUT 299

#define PLAYBACK_HIGH 50 // High time during pattern playback
#define PLAYBACK_LOW 49 // Low time during pattern playback
#define WAIT_TIME 99 // Delay before starting/resuming pattern playback
uint16_t patLength; // Keeps track of current pattern length - starts at 1, goes to max
uint16_t currentSeed; // Seed of PRNG for current round
uint16_t currentRand; // Used to temporarily hold result of PRNG

// Enum for FSM
enum current_state_enum {
    INTRO,              // Power-up animation, waits for button press to begin game
    WAIT_START,         // Interval between starting game and first pattern, to make the distinction clearer
    CALL,               // Displays randomly-generated sequence with lights and buzzer tones
    RESPONSE_DEPRESS,   // Waits for next response from the user, checks for correctness, end of pattern, and timeout
                        // Also enables lights and buzzer on each response
    RESPONSE_PRESS,     // Waits for user to depress the button, restarts timeout counter, stops light/buzzer when button is depressed
    WAIT_CONTINUE,      // Interval between final correct response and win animation
    LOSS,               // Plays loss animation when wrong button is pressed or when timeout elapses, returns to INTRO
    WIN                 // Plays win animation when pattern of length MAX_PAT_LENGTH is correctly recited, returns to INTRO
};



int main(void)
{
    InitializeProcessor();
    InitializeGPIO();
    InitializeSPI();

    InitializeTimerG0();
    InitializeTimerA1_PWM();

    InitializeMIDINotes();

    NVIC_EnableIRQ(TIMG0_INT_IRQn); // enable the timer interrupt
    TIMG0->COUNTERREGS.LOAD = 163; // set timer: approx 10ms
    TIMG0->COUNTERREGS.CTRCTL |= (GPTIMER_CTRCTL_EN_ENABLED);

    enum current_state_enum next_state;
    next_state = INTRO;

    while (1) { // this loop will execute once per timer interrupt
        timerCount++; // counting # of interrupts
        // Begin FSM ===========================================================
        switch (next_state) {
        case INTRO:
            if ( (GPIOA->DIN31_0 & (SW1 | SW2 | SW3 | SW4)) !=
                    (SW1 | SW2 | SW3 | SW4) ) { // if any buttons are on
                next_state = WAIT_START;
                stopNote();
                writeLights(txPacket, (bool[4]){0, 0, 0, 0});
                timerCount = -1;
                frameNum = 0;
                patLength = 1;
                currentSeed = GenerateRandomNumber() >> 16; // gets upper 16 bits of TRNG result
                srand(currentSeed);
            } else {
                // start of frame: set length, play note, change lights
                if (timerCount == 0) {
                    if (INTRO_FRAMES[frameNum].note == -1)
                        stopNote();
                    else
                        startNote(INTRO_FRAMES[frameNum].note);
                    writeLights(txPacket, INTRO_FRAMES[frameNum].lights);
                    timerLength = INTRO_FRAMES[frameNum].duration;
                }
                // end of note, start pause
                if (timerCount == timerLength) {
                    stopNote();
                    // Can comment this out to make the animation smoother
                    writeLights(txPacket, (bool[4]){0, 0, 0, 0});
                }
                // end of frame, set up next frame
                if (timerCount == timerLength + PAUSE) {
                    frameNum = (frameNum + 1) % INTRO_LENGTH; // loops frames
                    timerCount = -1;
                }
            }

            break;

        case WAIT_START:
            if (timerCount == WAIT_TIME) {
                next_state = CALL;
                timerCount = -1;
            }

            break;

        case CALL:
            // use same system as animations for timing lights and buttons
            // start of pattern frame: play next note/light
            if (timerCount == 0) {
                currentRand = rand();
                if (currentRand == 0) {
                    startNote(79 + NOTE_OFFSET);
                    writeLights(txPacket, (bool[4]){1, 0, 0, 0});
                }
                if (currentRand == 1) {
                    startNote(88 + NOTE_OFFSET);
                    writeLights(txPacket, (bool[4]){0, 1, 0, 0});
                }
                if (currentRand == 2) {
                    startNote(91 + NOTE_OFFSET);
                    writeLights(txPacket, (bool[4]){0, 0, 1, 0});
                }
                if (currentRand == 3) {
                    startNote(84 + NOTE_OFFSET);
                    writeLights(txPacket, (bool[4]){0, 0, 0, 1});
                }
            }
            // end of pulse, start pause
            if (timerCount == PLAYBACK_HIGH) {
                stopNote();
                writeLights(txPacket, (bool[4]){0, 0, 0, 0});
            }
            // end of frame, set up next frame, or go to RESPONSE if pattern is complete
            if (timerCount == PLAYBACK_HIGH + PLAYBACK_LOW) {
                if (frameNum == patLength-1) {
                    next_state = RESPONSE_DEPRESS;
                    frameNum = 0;
                    srand(currentSeed);
                } else {
                    frameNum++;
                }
                timerCount = -1;
            }

            break;

        case RESPONSE_DEPRESS:
            if (frameNum == patLength) { // if pattern is complete, either iterate length in WAIT_CONTINUE or go to WIN
                if (patLength == MAX_PAT_LENGTH)
                    next_state = WIN;
                else
                    next_state = WAIT_CONTINUE;
                timerCount = -1;
                frameNum = 0;
            } else if (timerCount == TIMEOUT) { // if pattern isn't done yet, check for timeout
                next_state = LOSS;
                timerCount = -1;
                frameNum = 0;
            } else if ( (GPIOA->DIN31_0 & (SW1 | SW2 | SW3 | SW4)) !=
                    (SW1 | SW2 | SW3 | SW4) ) { // otherwise, check input - if any button is on, then get rand and check
                currentRand = rand();
                if ( (GPIOA->DIN31_0 & SW1) != SW1 ) { // if the button is on
                    if (currentRand == 0) { // check if next pattern number matches
                        next_state = RESPONSE_PRESS;
                        startNote(79 + NOTE_OFFSET);
                        writeLights(txPacket, (bool[4]){1, 0, 0, 0});
                        timerCount = -1;
                    } else {
                        next_state = LOSS;
                        timerCount = -1;
                        frameNum = 0;
                    }
                }
                if ( (GPIOA->DIN31_0 & SW2) != SW2 ) {
                    if (currentRand == 1) {
                        next_state = RESPONSE_PRESS;
                        startNote(88 + NOTE_OFFSET);
                        writeLights(txPacket, (bool[4]){0, 1, 0, 0});
                        timerCount = -1;
                    } else {
                        next_state = LOSS;
                        timerCount = -1;
                        frameNum = 0;
                    }
                }
                if ( (GPIOA->DIN31_0 & SW3) != SW3 ) {
                    if (currentRand == 2) {
                        next_state = RESPONSE_PRESS;
                        startNote(91 + NOTE_OFFSET);
                        writeLights(txPacket, (bool[4]){0, 0, 1, 0});
                        timerCount = -1;
                    } else {
                        next_state = LOSS;
                        timerCount = -1;
                        frameNum = 0;
                    }
                }
                if ( (GPIOA->DIN31_0 & SW4) != SW4 ) {
                    if (currentRand == 3) {
                        next_state = RESPONSE_PRESS;
                        startNote(84 + NOTE_OFFSET);
                        writeLights(txPacket, (bool[4]){0, 0, 0, 1});
                        timerCount = -1;
                    } else {
                        next_state = LOSS;
                        timerCount = -1;
                        frameNum = 0;
                    }
                }
            }

            break;

        case RESPONSE_PRESS:
            if (timerCount == TIMEOUT) {
                next_state = LOSS;
                timerCount = -1;
                frameNum = 0;
            } else if ( (GPIOA->DIN31_0 & (SW1 | SW2 | SW3 | SW4)) ==
                    (SW1 | SW2 | SW3 | SW4) ) { // if all buttons are off
                next_state = RESPONSE_DEPRESS;
                stopNote();
                writeLights(txPacket, (bool[4]){0, 0, 0, 0});
                frameNum++;
            }

            break;

        case WAIT_CONTINUE:
            if (timerCount == WAIT_TIME) {
                next_state = CALL;
                timerCount = -1;
                patLength++;
                srand(currentSeed);
            }

            break;

        case WIN:
            // start of frame: set length, play note, change lights
            if (timerCount == 0) {
                if (WIN_FRAMES[frameNum].note == -1)
                    stopNote();
                else
                    startNote(WIN_FRAMES[frameNum].note);
                writeLights(txPacket, WIN_FRAMES[frameNum].lights);
                timerLength = WIN_FRAMES[frameNum].duration;
            }
            // end of note, start pause
            if (timerCount == timerLength) {
                stopNote();
                writeLights(txPacket, (bool[4]){0, 0, 0, 0});
            }
            // end of frame, set up next frame
            if (timerCount == timerLength + PAUSE) {
                frameNum++;
                if (frameNum == WIN_LENGTH) {
                    next_state = INTRO;
                    frameNum = 0;
                }
                timerCount = -1;
            }

            break;

        case LOSS:
            // start of frame: set length, play note, change lights
            if (timerCount == 0) {
                if (LOSS_FRAMES[frameNum].note == -1)
                    stopNote();
                else
                    startNote(LOSS_FRAMES[frameNum].note);
                writeLights(txPacket, LOSS_FRAMES[frameNum].lights);
                timerLength = LOSS_FRAMES[frameNum].duration;
            }
            // end of note, start pause
            if (timerCount == timerLength) {
                stopNote();
                writeLights(txPacket, (bool[4]){0, 0, 0, 0});
            }
            // end of frame, set up next frame
            if (timerCount == timerLength + PAUSE) {
                frameNum++;
                timerCount = -1;
                if (frameNum == LOSS_LENGTH) {
                    next_state = INTRO;
                    frameNum = 0;
                }
            }

            break;

        default:
            next_state = INTRO;

            break;

        }
        // End FSM =============================================================

        // Clear pending SPI interrupts
        NVIC_ClearPendingIRQ(SPI0_INT_IRQn);
        NVIC_EnableIRQ(SPI0_INT_IRQn);
        transmissionComplete = 0; // reset flag
        idx = 1; // reset pointer to point at the second element of the SPI message
        SPI0->TXDATA = *txPacket; // This will start TX ISR running.
        // It will stop itself at the end of the message, and disable SPI interrupts.

        while (!timerTicked) // Wait for timer wake up
            __WFI();

        timerTicked = 0; // reset timer interrupt flag
    }
}



void SPI0_IRQHandler(void)
{
    switch (SPI0->CPU_INT.IIDX) {
        case SPI_CPU_INT_IIDX_STAT_TX_EVT: // SPI interrupt index for transmit FIFO
            SPI0->TXDATA = txPacket[idx];
            idx++;
            if (idx == message_len) {
               transmissionComplete = 1; 
               NVIC_DisableIRQ(SPI0_INT_IRQn);
            }
            break;
        default:
            break;
    }
}

void TIMG0_IRQHandler(void)
{
    // This wakes up the processor!

    switch (TIMG0->CPU_INT.IIDX) {
        case GPTIMER_CPU_INT_IIDX_STAT_Z: // Counted down to zero event.
            timerTicked = 1; // set a flag so we can know what woke us up.
            break;
        default:
            break;
    }
}

/*
 * Copyright (c) 2020, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
