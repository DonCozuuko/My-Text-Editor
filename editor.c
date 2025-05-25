#include <stdio.h>
#include <raylib.h>
#include "special_characters.h"

static const int screenWidth = 960;
static const int screenHeight = 540;
static const int fpsCap = 60;
static const int maxInputChars = 100;
static const float fontSize = 25.0;
static const float fontSpacing = 2.0;
static const int blinkRate = 40;            // how fast the cursor blinks
static const int backspaceBuffer = 25;      // how long it takes to hold down backspace to rapidly backspace

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
        // Handles keys not on the shift layer
        if (key >= KEY_A && KEY_Z <= 90) {
            key += 32;
        }
    }
    else {
        // If shift button is being held
        handleSpecialChars(&key);
    }
    // Add the keys to the string array, but the keys must be printable ascii keys which excludes shift and cntrl
    if (key >= 32 && key <= 127) {
        formatChars(inputBuffer, numChars, key);
    }
}

void handleKeyPress(int key, char inputBuffer[], int *numChars, int *beginDisplay, int *shiftToggle,  int delTime[]) {
    // Handles rapid deleting with a counter buffer
    if (IsKeyDown(KEY_BACKSPACE)) {
        delTime[0]++;
        if (delTime[0] > backspaceBuffer) {
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
        if (key == KEY_BACKSPACE) {
            // Single Delete
            handleBackspace(inputBuffer, numChars);
        }
        // else if (key == KEY_ENTER) {
        //     TODO: for some reason if the first key pressed is enter then the cursor stops blinking in idle after
        // }
        else {
            handleCharacterInput(key, inputBuffer, numChars, shiftToggle);
        }
        // printf("%s\n", inputBuffer);  // debug printf
        printf("%d\n", key);  // debug printf
        // edge case for first time displaying a character
        if (*beginDisplay == 0) {
            *beginDisplay = 1;
        }
    }
}

void drawInput(Rectangle inputBox, char inputBuffer[], int *beginDisplay, Font font, int key, int blinkingClock[]) {
    if (*beginDisplay == 1) {
        // Draw text input
        Vector2 pos = { inputBox.x, inputBox.y };
        DrawTextEx(font, inputBuffer, pos, fontSize, fontSpacing, WHITE);
    }

    Vector2 startPos, endPos;

    if (*beginDisplay == 1) {
        // Start and end positions for the cursor when there is text on screen
        Vector2 textSize = MeasureTextEx(font, inputBuffer, fontSize, fontSpacing);
        startPos = (Vector2){ inputBox.x + textSize.x, inputBox.y };
        endPos = (Vector2){ inputBox.x + textSize.x, inputBox.y + textSize.y };
    }
    else {
        // Default position when there hasnt been any text on screen yet
        startPos = (Vector2){ inputBox.x, inputBox.y };
        endPos = (Vector2){ inputBox.x, inputBox.y + fontSize };
    }

    if (key > 0) {
        // Display solid cursor when typing
        blinkingClock[0] = 0;
        DrawLineV(startPos, endPos, WHITE);
    }
    else {
        // Display a blinking cursor when idling (not typing) and have a slight buffer
        // from when where is no key input to blinking state
        // modulo arithmeitc is fucking magic yo
        blinkingClock[0]++;
        if ((blinkingClock[0] / blinkRate) % 2 == 0) {
            DrawLineV(startPos, endPos, WHITE);
        }
    }
}

int main() {
    InitWindow(screenWidth, screenHeight, "text-editor-bitch!");
    SetTargetFPS(fpsCap);

    Font font = LoadFont("resources/Inter-Regular-slnt=0.ttf");

    char inputBuffer[maxInputChars + 1];

    Rectangle inputBox = { 5, 0, 300, 25 };

    int numChars = 0;      // counter
    int beginDisplay = 0;  // flag
    int shiftFlag = 0;     // flag
    int delTime[1] = {0};  // counter
    int blinkingClock[1] = {0};  // counter

    while (!WindowShouldClose()) {
        SetMouseCursor(MOUSE_CURSOR_IBEAM);
        int key = GetKeyPressed();
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_O)) {
            // Open a text file in write mode
            FILE *pf = fopen("test.txt", "w");
            fprintf(pf, "%s\n", inputBuffer);
            fclose(pf);
        }
        handleKeyPress(key, inputBuffer, &numChars, &beginDisplay, &shiftFlag, delTime);
        BeginDrawing();
        ClearBackground(BLACK);
        drawInput(inputBox, inputBuffer, &beginDisplay, font, key, blinkingClock);
        EndDrawing();
    }
    // If accidently closed, then automatically write input buffer contents to the file
    FILE *pf = fopen("test.txt", "w");
    fprintf(pf, "%s", inputBuffer);
    fclose(pf);
    CloseWindow();
    return 0;
}