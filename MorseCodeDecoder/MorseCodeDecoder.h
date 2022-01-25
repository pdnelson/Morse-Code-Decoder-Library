// MORSE CODE TIMING CONVENTIONS:
// Dit/dot ("."): 1 time unit
// Dah/dash ("-"): 3 time units
// Space between "dits"/"dahs" in one character: 1 time unit
// Space between characters: 3 time units
// Space between words: 7 time units

// Using this library, you can type complete letters, words or sentences, using a telegraph key. Once the user finishes typing the message,
// the user's input will be decoded and translated to an ASCII character array (string).

// Time units are relative to the SHORTEST tap in the code stream, though this rule has one exception:
// If only "dits" or only "dahs" are present in the code stream, then an interval, defined by TIME_UNIT_UPPER_LIMIT_MS, 
// is included into the decoding algorithm, which differentiates a "dit"/"dah."

// This is the part where this algorithm falls short, because if there are ONLY "dits" or "dahs,"" then we have nothing
// to compare against. We impose the timeUnitUpperLimitMs variable to attempt to differentiate the two, but this is not
// a perfect approach. In an ideal scenario, the user would have typed several words, which will contain their
// fair share of "dits" and "dahs"

// This library does NOT handle any buzzers, it only deals with recording user input, and decoding the message.

// Feel free to use, modify, or do whatever you want with this library. If you've done something cool with it, feel free to show me!

// Written because I just got a telegraph key at an auction, and I want to learn to use it.
// - Patrick Nelson (2022)

#ifndef MorseCodeDecoder_h
#define MorseCodeDecoder_h

#include <stdint.h>
#include <stdlib.h>

class MorseCodeDecoder {
    public:
        // Constructor that takes the MAXIMUM user input allowed, which corresponds to the maximum number of combined key presses and releases.
        // The decoded message max is the maximum number of characters that the decoded message is allowed to be. This INCLUDES the null terminator at the
        // end of the string.
        MorseCodeDecoder(uint16_t userInputMax, uint16_t decodedMessageMax);

        // "Time Units" in morse code don't have a particular number of milliseconds attached to them, but there exists an edge case where the message
        // may only contain "dits" or only "dahs," in which case, we have nothing to compare against. To attempt to counteract this, we are establishing an
        // upper limit for them here, so that ANY user input that is less than this limit will be registered as a "dit," while any user input equal to
        // or above this limit will be registered as a "dah"
        //
        // Default value: 100 milliseconds
        void setTimeUnitUpperLimitMs(uint16_t timeUnitUpperLimitMs);

        // Any taps that are equal to or below this number will be rejected.
        //
        // Default value: 20 milliseconds
        void setDebounceIntervalMs(uint8_t debounceIntervalMs);

        // The duration of inactivity before it is assumed that the user is done typing, and the message is ready to be decoded.
        //
        // Default value: 1500 milliseconds
        void setFinishedTypingMs(uint16_t finishedTypingMs);

        // This will take in the value of the sensor connected to the telegraph key, and the current time in milliseconds.
        // This function will, essentially, monitor when the user presses or releases the telegraph key, and then decode the message
        // upon inactivity.
        //
        // Returns true if there is a new message ready to be read by the code implementing this library, or false if there is currently no
        // decoded message ready.
        bool monitorUserInput(bool sensorStatus, long currMillis);

        // Tell the library that you've read the message, so that it can set the newMessageReady field to false.
        // This will also reset other fields that are used by the decoder, and is necessary to call when you are ready to read in a new message.
        void acknowledgeMessage();

        // Returns the final decoded message that contains a null terminator at the end.
        char* getDecodedMessage();

        // Returns the size of the most-recently-decoded message. This size does NOT include the null terminator.
        uint16_t getDecodedMessageSize();

        // Return the number of times that the user pressed (and then released) the telegraph key.
        uint16_t getUserInputSize();

        // Returns whether we are in "listening" mode or not.
        // In telegraphy, both telegraph keys are connected in series with each other, meaning both of them would need to be shorted in order for
        // a "beep" to be produced. To do this, one side would close the circuit by moving the circuit closer to the center, and the other side
        // would OPEN their circuit closer by moving it to the right (or left). This would allow the side with the open circuit to short the connection
        // to the morse code intervals using the knob.
        //
        // For this value, we are checking that the circuit has been closed for the duration set by "setFinishedTypingMs."
        bool getListeningStatus();


    private:
        void decodeMessage();
        char* decodedMessage; // Read above description (getDecodedMessage)
        bool messageDecoded; // Tell us whether the user is currently typing a new message, or if there are no messages to be decoded (and none in the works)
        uint16_t decodedMessageMax; // Read above description (constructor)

        uint16_t newMessageIndex; // The index used so the program knows where to insert decoded characters into the userInput array.
        bool newMessageReady; // Turns true after a message is decoded, and turns false as soon as getLatestMessage is called.

        uint16_t timeUnitUpperLimitMs; // Read above description (setTimeUnitUpperLimitMs)
        uint8_t debounceIntervalMs; // Read above description (setDebounceIntervalMs)
        uint16_t finishedTypingMs; // Read above description (setFinishedTypingMs)

        // The user input is being stored as an array of telegraph key presses. After the input is finished, THEN everything is parsed and printed.
        // Every EVEN index contains the amount of time the key was held down.
        // Every ODD index contains the amount of time the key was released between digits.
        uint16_t* userInput;
        uint16_t userInputCounter; // The number of times the key was pressed (and released).
        uint16_t userInputMax; // Read above description (constructor)

        unsigned long lastMillis; // The millis counter on the last iteration, used to check user input only once per millisecond
        unsigned long lastUserInputMs; // The last time there was any user activity on the key.

        uint16_t keyHoldCounterMs; // How many milliseconds the key was held.
        uint16_t lowestInputHoldMs; // The LOWEST duration that the key was held.
        uint16_t highestInputHoldMs; // The HIGHEST duration that the key was held.

        uint16_t keyReleaseCounterMs; // How many milliseconds the key was released IN BETWEEN times held.
        uint16_t lowestInputReleaseMs; // THe LOWEST duration that the key was released IN BETWEEN times held.

        bool listening; // Read above description (getListeningStatus)

};
#endif