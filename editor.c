#include <stdio.h>
#include <raylib.h>
#include "special_characters.h"

static const int screenWidth = 960;
static const int screenHeight = 540;
static const int fpsLim = 60;
static const int maxInputChars = 100;
static const int fontSize = 35;

void updateShiftState(int key, int *shiftToggle) {
    if (key == 340) {
        *shiftToggle = 1;
    }
    if (!(IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
        *shiftToggle = 0;
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

void handleKeyPress(int key, char inputBuffer[], int *numChars, int *beginDisplay, int *shiftToggle,  int delTime[]) {
    // Handles rapid deleting with a counter buffer
    if (IsKeyDown(KEY_BACKSPACE)) {
        delTime[0]++;
        if (delTime[0] > 25) {
            handleBackspace(inputBuffer, numChars);
        }
    }
    else {
        delTime[0] = 0;
    }

    if (key > 0) {
        // Everytime a key is pressed, we check to see if shift is held down
        // and update the shiftToggle flag accordingly
        updateShiftState(key, shiftToggle);
        if (key == 259) {
            // Single Delete
            handleBackspace(inputBuffer, numChars);
        }
        else {
            handleCharacterInput(key, inputBuffer, numChars, shiftToggle);
        }
        // printf("%s\n", inputBuffer);  // debug printf
        // printf("%d\n", key);  // debug printf
        // edge case for first time displaying a character
        if (*beginDisplay == 0) {
            *beginDisplay = 1;
        }
    }
}

void drawInput(Rectangle inputBox, char inputBuffer[], int *beginDisplay, Font font) {
    // DrawRectangleRec(inputBox, BLACK);
    // DrawRectangleLines(inputBox.x, inputBox.y, inputBox.width, inputBox.height, WHITE);
    if (*beginDisplay == 1) {
        Vector2 pos = { inputBox.x, inputBox.y };
        DrawTextEx(font, inputBuffer, pos, fontSize, 2.0, WHITE);
    }
}

int main() {
    InitWindow(screenWidth, screenHeight, "text-editor-bitch!");
    SetTargetFPS(fpsLim);

    Font font = LoadFont("resources/Inter-Regular-slnt=0.ttf");

    char inputBuffer[maxInputChars + 1];

    Rectangle inputBox = { 5, 0, 300, 25 };

    int numChars = 0;      // counter
    int beginDisplay = 0;  // flag
    int shiftFlag = 0;     // flag
    int delTime[1] = {0};  // counter

    while (!WindowShouldClose()) {
        // SetMouseCursor(MOUSE_CURSOR_IBEAM);
        int key = GetKeyPressed();
        handleKeyPress(key, inputBuffer, &numChars, &beginDisplay, &shiftFlag, delTime);
        BeginDrawing();
            ClearBackground(BLACK);
            drawInput(inputBox, inputBuffer, &beginDisplay, font);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}