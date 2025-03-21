#include <ti/devices/msp/msp.h>
#include "simon_setup.h"
#include "simon_av.h"



// SPI stuff
uint16_t txPacket[12];
int transmissionComplete = 0; // flag for SPI ISR wakeup
int timerTicked = 0; // flag for timer ISR wakeup
int idx = 0;
int message_len = sizeof(txPacket) / sizeof(txPacket[0]);

// Song stuff
int16_t timerCount = -1;
uint16_t frameNum = 0;
uint16_t timerLength;

// Enum for FSM
enum current_state_enum {
    LIGHTS_OFF = 0,
    BLUE = 1,
    RED = 2,
    GREEN = 3,
    YELLOW = 4,
    INTRO = 5
};



int main(void)
{
    InitializeProcessor();
    InitializeGPIO();
    InitializeSPI();

    InitializeTimerG0();
    InitializeTimerA1_PWM();

    InitializeMIDINotes();

    NVIC_EnableIRQ(TIMG0_INT_IRQn); // Enable the timer interrupt
    TIMG0->COUNTERREGS.LOAD = 163; // Set timer: approx 10ms
    TIMG0->COUNTERREGS.CTRCTL |= (GPTIMER_CTRCTL_EN_ENABLED);

    enum current_state_enum next_state;
    next_state = INTRO;

    while (1) { // this loop will execute once per timer interrupt
        switch (next_state) {
        case INTRO:
            if ( (GPIOA->DIN31_0 & (SW1 | SW2 | SW3 | SW4)) != (SW1 | SW2 | SW3 | SW4) ) // if any buttons are on
                next_state = LIGHTS_OFF;
            else
                next_state = INTRO;

            timerCount++;
            if (timerCount == 0) { // start of note, read note and play it, change lights, set length
                if (INTRO_FRAMES[frameNum].note == -1)
                    stopNote();
                else
                    startNote(INTRO_FRAMES[frameNum].note + NOTE_OFFSET);
                writeLights(txPacket, INTRO_FRAMES[frameNum].lights);
                timerLength = INTRO_FRAMES[frameNum].duration;
            }
            if (timerCount == timerLength) { // end of note, start pause
                stopNote();
            }
            if (timerCount == timerLength + PAUSE) {
                frameNum = (frameNum + 1) % INTRO_LENGTH;
                timerCount = -1;
            }

            break;

        case LIGHTS_OFF:
            writeLights(txPacket, (bool[4]){0, 0, 0, 0}); // Set what SPI message will be transmitted. (all LEDs off)
            if ( (GPIOA->DIN31_0 & SW1) != SW1 ) { // If the button is on
                next_state = BLUE;
                startNote(79 + NOTE_OFFSET);
            }
            else if ( (GPIOA->DIN31_0 & SW2) != SW2 ) {
                next_state = RED;
                startNote(88 + NOTE_OFFSET);
            }
            else if ( (GPIOA->DIN31_0 & SW3) != SW3 ) {
                next_state = GREEN;
                startNote(91 + NOTE_OFFSET);
            }
            else if ( (GPIOA->DIN31_0 & SW4) != SW4 ) {
                next_state = YELLOW;
                startNote(84 + NOTE_OFFSET);
            }
            else
                next_state = LIGHTS_OFF;
            break;

        case BLUE:
            writeLights(txPacket, (bool[4]){1, 0, 0, 0});
            if ( (GPIOA->DIN31_0 & SW1) != SW1 ) // If the button is on
                next_state = BLUE;
            else {
                next_state = LIGHTS_OFF;
                stopNote();
            }
            break;

        case RED:
            writeLights(txPacket, (bool[4]){0, 1, 0, 0});
            if ( (GPIOA->DIN31_0 & SW2) != SW2 ) // If the button is on
                next_state = RED;
            else {
                next_state = LIGHTS_OFF;
                stopNote();
            }
            break;

        case GREEN:
            writeLights(txPacket, (bool[4]){0, 0, 1, 0});
            if ( (GPIOA->DIN31_0 & SW3) != SW3 ) // If the button is on
                next_state = GREEN;
            else {
                next_state = LIGHTS_OFF;
                stopNote();
            }
            break;

        case YELLOW:
            writeLights(txPacket, (bool[4]){0, 0, 0, 1});
            if ( (GPIOA->DIN31_0 & SW4) != SW4 ) // If the button is on
                next_state = YELLOW;
            else {
                next_state = LIGHTS_OFF;
                stopNote();
            }
            break;

        default:
            next_state = INTRO;

        }

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
