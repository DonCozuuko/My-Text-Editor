#include <stdio.h>
#include <raylib.h>

static const int screenWidth = 960;
static const int screenHeight = 540;
static const int fpsLim = 60;
static const int maxInputChars = 100;

void displayInput(int key, int *keyPressed, char inputBuffer[], int *numChars, int *beginDisplay, int *shiftFlag) {
    // if key is pressed; not 0
    if (key > 0) {
        if (key >= 65 && key <= 90) {
            key += 32;
        }
        *keyPressed = 1;
    }
    
    if (*keyPressed != 0) {
        // handling the logic for backspace
        if (key == 259) {
            inputBuffer[*numChars - 1] = '\0';
            if (*numChars > 0) {
                (*numChars)--;
            }
        }
        // any other key
        else {
            inputBuffer[*numChars] = (char)key;
            inputBuffer[(*numChars) + 1] = '\0';
            (*numChars)++;
            printf("%s\n", inputBuffer);
        }
        printf("%d\n", *numChars);
        *keyPressed = 0;  
        
        if (*beginDisplay == 0) {
            *beginDisplay = 1;
        }
    }
}

int main() {
    InitWindow(screenWidth, screenHeight, "text-editor-bitch!");
    SetTargetFPS(fpsLim);

    char inputBuffer[maxInputChars + 1];

    Rectangle inputBox = { 10, 0, 300, 25 };

    int numChars = 0;      // counter
    int keyPressed = 0;    // flag
    int beginDisplay = 0;  // flag
    int shiftFlag = 0;     // flag

    while (!WindowShouldClose()) {
        // SetMouseCursor(MOUSE_CURSOR_IBEAM);
        int key = GetKeyPressed();
        displayInput(key, &keyPressed, inputBuffer, &numChars, &beginDisplay, &shiftFlag);
        BeginDrawing();
            ClearBackground(BLACK);
            DrawRectangleRec(inputBox, BLACK);
            DrawRectangleLines(inputBox.x, inputBox.y, inputBox.width, inputBox.height, WHITE);
            if (beginDisplay == 1) {
                DrawText(inputBuffer, inputBox.x, inputBox.y, 20, WHITE);
            }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}