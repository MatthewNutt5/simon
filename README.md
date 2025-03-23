# simon

Midterm project for ELEC 327 - Simon memory game implemented on MSPM0G3507 microcontroller

### Description

This project is based around a board provided by the ELEC 327 instructor that integrates the microcontroller, four RGB LEDs, four pushbuttons, and a buzzer. With these components, the classic Simon memory game can be programmed.

## Main Program

The `simon.c` file uses the included libraries to play the game.

### Setup

In `simon.c`, necessary setup includes defining global constants, declaring global variables, and running initialization routines (described in the Libraries section).

### Loop

After the setup in `simon.c`, the main loop consists of a persistent interrupt timer with a ~10ms period that iterates on an FSM. The FSM manages the state of the game - initialized, blinking pattern, waiting for response, etc. The FSM also keeps track of the number of interrupts. Based on the current pushbutton inputs, number of interrupts, and randomly generated numbers, the FSM will move between different game states according to the rules of Simon. In certain game states, the FSM will play animations defined in the `simon_av` library.

The 10ms period is specifically useful for providing the timing of the animations and the timeout feature. Additionally, it provides de-facto debouncing, as the pushbutton bouncing lasts less than 10ms.

At the end of a loop iteration, the program configures the SPI system to send the next packet to the LEDs, and waits for the next interrupt.

### Additional Functions

Outside of the loop, two functions for handling interrupt requests are implemented - one for iterating the SPI packet, and one for the 10ms timer. These functions are provided by the instructor.

## Libraries

### `simon_av`

The `simon_av` (av = audiovisual) library contains basic functions for managing the Simon board's LEDs and buzzer. For the buzzer, the library uses the MIDI standard of matching numbers to notes/pitches:

`InitializeMIDINotes`: This initialization function that calculates the period of each note frequency and stores them in a lookup array.

`startNote`: This function takes a MIDI note as input and configures the `TIMA1` module to play this note, which outputs to the buzzer.

`stopNote`: This function simply disables the `TIMA1` counter in order to stop any note that is playing.

For the LEDs, the library has one function.

`writeLights`: This function writes an SPI packet at the given pointer that corresponds to the given light settings, i.e. which lights are on and off. The packet is actually sent in the interrupt loop of `simon.c`.

The library additionally defines a few arrays containing animations for the board to play. Animations consist of frames, where each frame has a duration, note for the buzzer (or silence), and an array indicating which LEDs should be enabled or disabled. Timing constants are given in the `.h` header file, while animations are defined in the `.c` source file.

### `simon_setup`, `simon_random`

These libraries are given by the instructor. `simon_setup` provides boilerplate functions for initializing outputs, timers, etc., and `simon_random` provides a function for generating a random number with the board's TRNG module, as well as functions for managing a LFSR-based PRNG that can be seeded with the TRNG.

## Rubric/Score

Based on the provided rubric, this project should earn 150/150 points.

1) 10pts: *"When powered on, does the game display an animation that involves changing patterns of the lights?"* Yes, the board displays an animation that includes changing lights and musical tones. Do you know the tune?

2) 10pts: *"Does the power on animation also include changing sounds?"* Yes; see above.

3) 10pts: *"Does the power-on animation transition to game play when a button is pressed?"* Yes, the FSM stops the idle animation when a button is pressed, and after a ~1s interval, begins the first pattern.

4) 10pts: *"Is the first element displayed random, including between power cycles?"* Yes, the PRNG is seeded with the board's TRNG module, so the first element (and the whole sequence) is randomized between rounds and power cycles.

5) 10pts: *"When a button is pressed during play, does the light asssociated with that button track the button press, i.e., turn on when it is depressed and turn off when it is released?"* Yes.

6) 10pts: *"When a button is pressed during play, does the sound asssociated with that button track the button press, i.e., turn on when it is depressed and turn off when it is released?"* Yes.

7) 10pts: *"Does the game transition to a “loss” state if no button is pressed within some period during the response?"* Yes: The FSM keeps track of the number of interrupts between new button presses during the response phase, so if a certain interval passes (currently ~3s), the game goes to the loss state.

8) 10pts: *"Does the game transition to a “loss” state if a wrong button is pressed during a response?"* Yes.

9) 10pts: *"If the player wins (correctly responds to a sequence of length 5), does the game display an animated sequence of lights?"* Yes.

10) 10pts: *"Does the win animation also include sounds?"* Yes, the animation includes a short fanfare.

11) 10pts: *"If the player loses (times out or responds incorrectly), does the game display an animated sequence of lights?"* Yes.

12) 10pts: *"Does the lose animation also include sounds?"* Yes.

13) 10pts: *"Can the length of a sequence required to win be changed by changing no more than one line of code?"* Yes; this can be done with the `MAX_PAT_LENGTH` constant, defined on line 21 of `simon.c`.

14) 20pts: *"Well-commented, logical code architecture (does the architecture make sense? are there good comments?, functions (libraries?) used intelligently?)"* Yes, comments provide context for each section of code, and anything not explained by comments is self-explained by well-named variables/functions. The architecture, especially with regards to the animations, lends itself to quick modification. Functions are used to encapsulate certain low-level operations and make them easy to understand and use.

## Known Issues

The code structure of this project is imperfect. The most glaring issue is the complexity of the main loop: Ideally, counter variables and such for playing the animations would be hidden away in a separate library. However, differences in each use-case necessitate individualized code: The `CALL` state uses randomly-generated sequences instead of predefined animations, the `INTRO` state loops while the `WIN` and `LOSS` states don't, etc. Off-by-one errors are therefore easy to come by, but all (known) errors have been found and resolved.
Another issue is the lack of strict debouncing. As previously described, the 10ms timer period effectively provides debouncing, and testing hasn't revealed any flaws, but either a legitimate debouncing scheme or characterization of the given pushbuttons' bouncing behavior (proof that they always bounce for less than 10ms) would provide ease-of-mind.
