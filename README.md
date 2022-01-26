# Morse-Code-Decoder-Library
Using this library, you can type complete letters, words or sentences, using a single button input, or get full use of your telegraph key! Once the user finishes typing the message, the user's input will be decoded and translated to an ASCII character array that the user can do anything with, such as execute a command based on the decoded phrase, or just print the text to the screen.

This is compatible with ANY microcontroller that you write C++ code for, not specifically Arduino, though it will work just 
fine with all Arduino-based boards/microprocessors.

This library does NOT handle any buzzers, it only deals with recording user input, and decoding the message. A buzzer could 
be wired up on a breadboard, and does not need any code to operate.

Feel free to use, modify, or do whatever you want with this library. If you've done something cool with it, feel free to 
show me!

## How to use it?!

### Constructor fields
To use this, one must understand the FIVE values that must be set before your program begins looping. TWO of these values are 
set in the constructor, and cannot be changed at any point in the program:
- `userInputMax` - The MAXIMUM number of times you can press/release the button/telegraph key. A press and then release counts as TWO user input values.
- `decodedMessageMax` - The MAXIMUM number of characters the final decoded message can be. This includes the null terminator at the end of the character array. For example, if you want to limit a message to being `5` characters, then you must set this to `6`.

### Optional fields
The remaining THREE values have default values, are optional, and can be set/changed at any point of the program's execution.
- `timeUnitUpperLimitMs` - If your message ONLY contains "dits" or "dahs," then the program uses this value to differentiate between a "dit" and a "dah."
  - Default value: 100ms
  - Setter method: `setTimeUnitUpperLimitMs`
- `finishedTypingMs` - The duration of inactivity before the program assumes you are done typing.
  - Default value: 1500ms
  - Setter method: `setFinishedTypingMs`
- `debounceIntervalMs` - Any user input (button presses or releases) that are less or equal to this value will be rejected.
  - Default value: 20ms
  - Setter method: `setDebounceIntervalMs`

### `monitorUserInput` function
Once these values are set, you can call `monitorUserInput`, which takes in two arguments: A boolean value representing whether 
the button/telegraph key is being pressed, and the current time (in milliseconds) that your microcontroller is at. `monitorUserInput` 
will do as advertised: It will monitor any button presses/releases, and save them as user input. It will return `false` if a new 
complete message is not ready, but will return `true` as soon as there is a decoded message that is ready for the user to do 
something with. For this reason, it is best to use this function's return value in the condition of an `if` block, so you can 
execute the code in that `if` block ONLY when a message is ready.

At the end of the `if` block that `monitorUserInput` brings you into, it is important to call `acknwoledgeMessage`. This function
tells the library that the main code has acted on the message, and will no longer be considered a "new" message. If this isn't called,
then the code within the `if` block will be executed repeatedly after the first message is decoded!

There are three methods that can be used within the `monitorUserInput` `if` block, and they are as follows:
- `getDecodedMessage` - The most-recent decoded message, as a character array.
- `getDecodedMessageSize` - The number of characters the most-recent decoded message has (NOT including the null terminator).
- `getUserInputSize` - The number of times the user pressed and released the button/telegraph key. This does NOT include the last release, so this will always be an odd number.

### `getListeningStatus` function
This function will return `true` if the library is currently in "listening mode," otherwise it will return `false`. What does this mean? 
In telegraphy, both telegraph keys are connected in series with each other, meaning both of them would need to be shorted in order for
a "beep" to be produced. To do this, one side would close the circuit by moving the circuit closer to the center, and the other side
would OPEN their circuit closer by moving it to the right (or left). This would allow the side with the open circuit to short the connection
to the morse code intervals using the knob, while the operator on the side with the CLOSED circuit would listen (hence this mode being 
called "listening").

Specifically, how the code behind `getListeningStatus` operates is that, after the user holds the button/telegraph key for the duration 
of time defined by `finishedTypingMs`, then we enter "listening mode." This can be used to signify that you are TOTALLY finished with a message,
as in the Keyboard Example below.

Is this a bit confusing? Possibly! Check out the code examples for some more clarity!

## General Example
The general example included will run on any microcontroller that you can write C++ for; it showcases all of the functions library 
contains, and explanations of what they do.

## Keyboard Example
I used an Arduino Micro with this example, because it is the Arduino board that is most-easily usable as a USB interface device (such as a keyboard).

For this, the Arduino is seen as a USB keyboard by whatever device it is plugged into, and the button/telegraph key can be used to enter
messages in morse code. These messages are then converted to ASCII characters, and typed on the screen.

To use the enter key, hold the button/telegraph key for the duration defined in `finishedTypingMs`.

## Morse Code International Alphabet
This library uses the INTERNATIONAL alphabet, as well as some special characters I threw in there. Check out 
`MorseCodeDecoder/alphabets/MorseCodeAlphabet.h` to see all the characters and the corresponding morse code.

## Morse Code Timing Conventions
Dit/dot ("."): 1 time unit

Dah/dash ("-"): 3 time units

Space between "dits"/"dahs" in one character: 1 time unit

Space between characters: 3 time units

Space between words: 7 time units

Time units are relative to the SHORTEST tap in the code stream, though this rule has one exception:
If only "dits" or only "dahs" are present in the code stream, then an interval, defined by TIME_UNIT_UPPER_LIMIT_MS, 
is included into the decoding algorithm, which differentiates a "dit"/"dah."

This is the part where this algorithm falls short, because if there are ONLY "dits" or "dahs,"" then we have nothing
to compare against. We impose the timeUnitUpperLimitMs variable to attempt to differentiate the two, but this is not
a perfect approach. In an ideal scenario, the user would have typed several words, which will contain their
fair share of "dits" and "dahs."
