#include <MorseCodeDecoder.h>

#define SERIAL_SPEED 115200
#define TELEGRAPH_KEY 2
#define BUZZER 7

// This number will be the maximum number of key down-presses and releases combined.
// The user input max is NOT the max number of characters.
// The user input max should ALWAYS be greater than the DECODED_MESSAGE_MAX_CHARS constant below.
#define USER_INPUT_MAX 100

// Size INCLUDES null terminator at the end. For example, if you want exactly 20 characters (at max) to be displayed to the end user,
// this size should be set to 21. You are usually safe making this constant approximately 1/3 of the USER_INPUT_MAX
#define DECODED_MESSAGE_MAX_CHARS 34

MorseCodeDecoder decoder = MorseCodeDecoder(USER_INPUT_MAX, DECODED_MESSAGE_MAX_CHARS);

bool lastListeningStatus = false;

void setup() {
    Serial.begin(SERIAL_SPEED);
    pinMode(TELEGRAPH_KEY, INPUT);

    // All of these fields are optional, and contain default values. If you aren't sure what they do, leaving them as their
    // default values (by not calling the methods) should be satisfactory for most users. See MorseCodeDecoder.h for more details.
    //
    // These values can be changed at any point while the program is running. One possible use case for this would be having buttons
    // to increment/decrement these values, though, to me, they are mostly "set and forget" values.
    decoder.setDebounceIntervalMs(20);
    decoder.setTimeUnitUpperLimitMs(100);
    decoder.setFinishedTypingMs(2000);
}

void loop() {
    bool currTelegraphKeyStatus = digitalRead(TELEGRAPH_KEY);
    long currMillis = millis();

    // If the user input is inactive for the amount of time determined by finishedTypingMs, then the message will be decoded.
    // As soon as the message is finished being decoded, this method returns true, and we can get the message.
    if(decoder.monitorUserInput(currTelegraphKeyStatus, currMillis)) {

        // These functions don't do anything special, they only return the desired memory addresses' contents, so they can be
        // called/used directly instead of being stored in a variable.
        Serial.print("Message sent: ");
        Serial.println(decoder.getDecodedMessage());

        Serial.print("Message size: ");
        Serial.println(decoder.getDecodedMessageSize());

        Serial.print("User input size: ");
        Serial.println(decoder.getUserInputSize());

        // Tell the decoder you've read the message so that we don't continuously execute the code in this 'if' block.
        decoder.acknowledgeMessage(); 
    }

    bool listeningStatus = decoder.getListeningStatus();

    // The "listening" field monitors whether the user held the key down for the duration of finishedTypingMs
    // For a full description of what this field represents, see MorseCodeDecoder.h
    if(listeningStatus && listeningStatus != lastListeningStatus) {
        Serial.println("Listening...");
    }
    else if (!listeningStatus && listeningStatus != lastListeningStatus) {
        Serial.println("No longer listening.");
    }

    lastListeningStatus = listeningStatus;
}