#include "MorseCodeDecoder.h"
#include "alphabets/MorseCodeAlphabet.h"

MorseCodeDecoder::MorseCodeDecoder(uint16_t userInputMax, uint16_t decodedMessageMax) {

    /* Set default values for user-settable fields */
    this->timeUnitUpperLimitMs = 100;
    this->debounceIntervalMs = 20;
    this->finishedTypingMs = 1500;

    /* Set default values for all other fields */
    this->newMessageIndex = 0; // in bytes
    this->messageDecoded = true;
    this->decodedMessageMax = decodedMessageMax;
    this->userInputCounter = 0;
    this->lastUserInputMs = __LONG_MAX__;
    this->keyHoldCounterMs = 0;
    this->keyReleaseCounterMs = 0;
    this->lowestInputHoldMs = UINT16_MAX;
    this->highestInputHoldMs = 0;
    this->lowestInputReleaseMs = UINT16_MAX;
    this->userInputMax = userInputMax;

    // We are safe to use malloc here because this constructor should only be called at the beginning of
    // the program.
    this->userInput = (uint16_t*)malloc(userInputMax * sizeof(uint16_t));
    this->decodedMessage = (char*)malloc(decodedMessageMax * sizeof(char));
}

void MorseCodeDecoder::setTimeUnitUpperLimitMs(uint16_t timeUnitUpperLimitMs) {
    this->timeUnitUpperLimitMs = timeUnitUpperLimitMs;
}

void MorseCodeDecoder::setDebounceIntervalMs(uint8_t debounceIntervalMs) {
    this->debounceIntervalMs = debounceIntervalMs;
}

void MorseCodeDecoder::setFinishedTypingMs(uint16_t finishedTypingMs) {
    this->finishedTypingMs = finishedTypingMs;
}

void MorseCodeDecoder::acknowledgeMessage() {
    this->userInputCounter = 0;
    this->newMessageReady = false;
}

char* MorseCodeDecoder::getDecodedMessage() {
    return this->decodedMessage;
}

uint16_t MorseCodeDecoder::getDecodedMessageSize() {
    return this->newMessageIndex;
}

uint16_t MorseCodeDecoder::getUserInputSize() {
    return this->userInputCounter;
}

bool MorseCodeDecoder::monitorUserInput(bool sensorStatus, long currMillis) {

    // Only monitor user input if 1ms has elapsed
    if(currMillis != lastMillis) {
        bool currentlyTyping = currMillis - this->lastUserInputMs <= this->finishedTypingMs;

        // Count the number of milliseconds that the user held the telegraph key
        if(sensorStatus) {
            this->lastUserInputMs = currMillis;
            this->messageDecoded = false; // If the user is typing, then the message has not been decoded yet

            // keyReleaseCounterMs being 0 indicates we are starting a new message, or it has already been added to the array.
            // Items will ONLY continue to be added to the array if there is space
            if(this->keyReleaseCounterMs > this->debounceIntervalMs && this->userInputCounter < this->userInputMax) {
                this->userInput[this->userInputCounter] = this->keyReleaseCounterMs;

                if(this->keyReleaseCounterMs < this->lowestInputReleaseMs) this->lowestInputReleaseMs = this->keyReleaseCounterMs;

                this->userInputCounter++;
                this->keyReleaseCounterMs = 0;
            }

            this->keyHoldCounterMs++;
        }
        // Count the number of milliseconds that the user released the telegraph key
        else if (currentlyTyping) {

            // keyHoldCounterMs being 0 indicates the last key hold value was already added to the array
            // Items will ONLY continue to be added to the array if there is space
            if(this->keyHoldCounterMs > this->debounceIntervalMs && this->userInputCounter < this->userInputMax) {
                this->userInput[this->userInputCounter] = this->keyHoldCounterMs;

                if(this->keyHoldCounterMs > this->highestInputHoldMs) this->highestInputHoldMs = this->keyHoldCounterMs;
                if(this->keyHoldCounterMs < this->lowestInputHoldMs) this->lowestInputHoldMs = this->keyHoldCounterMs;

                this->userInputCounter++;
                this->keyHoldCounterMs = 0;
            }

            this->keyReleaseCounterMs++;
        }
        else if(!this->messageDecoded) {
            // The user finished typing and we can decode the message
            this->decodeMessage();
            this->newMessageReady = true;

            // Reset everything back to being blank
            this->keyReleaseCounterMs = 0;
            this->keyHoldCounterMs = 0;
            this->messageDecoded = true;
            this->lowestInputHoldMs = UINT16_MAX;
            this->highestInputHoldMs = 0;
            this->lowestInputReleaseMs = UINT16_MAX;
        }

        this->lastMillis = currMillis;
    }

    return this->newMessageReady;
}

void MorseCodeDecoder::decodeMessage() {
    // If the lowest/highest input HOLD counters are too close together, and they exceed the upper limit, then we got all "dahs" and must set the lower
    // counter to 0 so the next function calculates EVERY bit as a 1
    if(this->lowestInputHoldMs >= this->timeUnitUpperLimitMs && (this->highestInputHoldMs == this->lowestInputHoldMs || this->lowestInputHoldMs << 1 >= this->highestInputHoldMs)) {
        lowestInputHoldMs = 0;
    }

    // These two variables are used to build the individual characters that make up a message.
    // We are only interested in using the 6 least-significant bits of the byte.
    uint8_t charBuilderIndex = 5;
    uint8_t charBuilder = 0b00000000;
    
    this->newMessageIndex = 0;

    // Traverse through the user input and set each bit in the charBuilder equal to the user input that the index corresponds to
    for(uint16_t i = 0; i < this->userInputCounter && this->newMessageIndex < this->decodedMessageMax; i += 2) {

        // Determine whether the bit should be a 0 or a 1, based off of the user input
        // Set 1 ONLY IF the curr value is greater than the lowest overall * 2. Otherwise, do nothing, because
        // the bit is already 0.
        if (this->lowestInputHoldMs << 1 < this->userInput[i]) 
            charBuilder |= 1UL << charBuilderIndex; 

        // If next bit is a character separator (3 time units) OR the end of the user input, 
        // then we must finish off the rest of the binary data for this single character
        if (i == (this->userInputCounter - 1) || this->lowestInputReleaseMs << 1 < this->userInput[i + 1] || this->newMessageIndex == this->decodedMessageMax) {

            // Set remaining bits to the OPPOSITE of the last bit, only if it was 0
            // If the last bit was 1, then the rest of the bits should be 0 (and they already are!)
            if(!((charBuilder >> charBuilderIndex) & 0x01)) {
                switch(charBuilderIndex) {
                    case 5: 
                        charBuilder |= 0b00011111;
                        break;
                    case 4:
                        charBuilder |= 0b00001111;
                        break;
                    case 3:
                        charBuilder |= 0b00000111;
                        break;
                    case 2:
                        charBuilder |= 0b00000011;
                        break;
                    case 1:
                        charBuilder |= 0b00000001;
                }
            }

            // This character is complete, and we may set the next character in the decoded message
            // We are overwriting the user input array to store the decoded message because it is already allocated,
            // and we do not need to use the values that have already been decoded.
            this->decodedMessage[this->newMessageIndex] = morseCodeInternational[charBuilder];
            this->newMessageIndex++;
            charBuilderIndex = 6; // One higher than 5 because we decrement the index at the end of the loop
            charBuilder = 0b00000000;

            // If the next character is a space (7 time units), then set that byte and increment the message index.
            // Greater than or equal to 6 time units (in relation to lowest input) will result in a space.
            // TODO: Figure out how to not use the multiplication operator here.
            if(i < this->userInputCounter - 1 && this->newMessageIndex < this->decodedMessageMax && this->lowestInputReleaseMs * 6 < this->userInput[i + 1]) {
                this->decodedMessage[this->newMessageIndex] = ' ';
                this->newMessageIndex++;
            }
        }

        charBuilderIndex--;
    }

    // If we are NOT all the way at the end of the array, then we can safely append the null terminator.
    if(this->newMessageIndex < this->decodedMessageMax)
        this->decodedMessage[this->newMessageIndex] = '\0';

    // Otherwise, we need to overwrite the last character.
    else {
        this->decodedMessage[this->newMessageIndex - 1] = '\0';
        this->newMessageIndex--;
    }
}