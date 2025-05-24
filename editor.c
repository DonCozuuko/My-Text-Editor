#include <stdio.h>
#include <raylib.h>
#include "special_characters.h"

static const int screenWidth = 960;
static const int screenHeight = 540;
static const int fpsLim = 60;
static const int maxInputChars = 100;

void updateShiftState(int key, int *shiftToggle) {
    if (key == 340) {
        *shiftToggle = 1;
    }
}

void handleBackspace(char inputBuffer[], int *numChars) {
    inputBuffer[*numChars - 1] = '\0';
    if (*numChars > 0) {
        (*numChars)--;
    }
}

void formatChars(char inputBuffer[], int *numChars, int key) {
    inputBuffer[*numChars] = (char)key;
    inputBuffer[(*numChars) + 1] = '\0';
    (*numChars)++;
}

void handleCharacterInput(int key, char inputBuffer[], int *numChars, int *shiftToggle) {
    if (*shiftToggle == 0) {
        // handles lower-case letters
        if (key >= 65 && key <= 90) {
            key += 32;
        }
    }
    else {
        handleSpecialChars(&key);
    }
    // Add the keys to the string array
    if (key != 340) {
        formatChars(inputBuffer, numChars, key);
    }
}

void handleKeyPress(int key, char inputBuffer[], int *numChars, int *beginDisplay, int *shiftToggle) {
    // if key is pressed; not 0
    if (key > 0) {
        updateShiftState(key, shiftToggle);
        if (key == 259) {
            handleBackspace(inputBuffer, numChars);
        }
        else {
            handleCharacterInput(key, inputBuffer, numChars, shiftToggle);
        }
        printf("%s\n", inputBuffer);  // debug printf
        // printf("%d\n", key);  // debug printf
        // edge case for first time displaying a character
        if (*beginDisplay == 0) {
            *beginDisplay = 1;
        }
    }
    if (!(IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
        *shiftToggle = 0;
    }
    // if (*shiftToggle == 1) {
    //     printf("shifting\n");
    // }
}

void drawInputBox(Rectangle inputBox, char inputBuffer[], int *beginDisplay) {
    DrawRectangleRec(inputBox, BLACK);
    DrawRectangleLines(inputBox.x, inputBox.y, inputBox.width, inputBox.height, WHITE);
    if (*beginDisplay == 1) {
        DrawText(inputBuffer, inputBox.x, inputBox.y, 20, WHITE);
    }
}

int main() {
    InitWindow(screenWidth, screenHeight, "text-editor-bitch!");
    SetTargetFPS(fpsLim);

    char inputBuffer[maxInputChars + 1];

    Rectangle inputBox = { 10, 0, 300, 25 };

    int numChars = 0;      // counter
    // int keyPressed = 0;    // flag
    int beginDisplay = 0;  // flag
    int shiftFlag = 0;     // flag

    while (!WindowShouldClose()) {
        // SetMouseCursor(MOUSE_CURSOR_IBEAM);
        int key = GetKeyPressed();
        handleKeyPress(key, inputBuffer, &numChars, &beginDisplay, &shiftFlag);
        BeginDrawing();
            ClearBackground(BLACK);
            drawInputBox(inputBox, inputBuffer, &beginDisplay);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}