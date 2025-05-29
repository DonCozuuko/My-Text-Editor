#include <stdio.h>
#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include "special_characters.h"

static const int screenWidth = 960;
static const int screenHeight = 540;
static const int fpsCap = 60;
#define MAX_INPUT_CHARS 100
#define maxNumLines 10
static const float fontSize = 25.0;
static const float fontSpacing = 2.0;
static const int blinkRate = 40;            // how fast the cursor blinks
static const int backspaceBuffer = 25;      // how long it takes to hold down backspace to rapidly backspace

typedef struct {
    char string[MAX_INPUT_CHARS + 1];
    int numChars;
} Line;

typedef struct {
    int indexPos;
    int line;
} Cursor;

char *sliceStringByIndex(char *string, int startIndex, int endIndex) {
    char *copy = malloc(endIndex + 1);
    strncpy(copy, string + startIndex, endIndex);
    copy[endIndex] = '\0';
    return copy;
}

void updateShiftState(int key, int *shiftToggle) {
    if (key == KEY_LEFT_SHIFT || key == KEY_RIGHT_SHIFT) {
        *shiftToggle = 1;
    }
    if (!(IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
        *shiftToggle = 0;
    }
}

void backspaceEndOfString(Line *inputBuffer[], int currentLine, Cursor *cursor) {
    inputBuffer[currentLine]->string[cursor->indexPos - 1] = '\0';
    inputBuffer[currentLine]->numChars--;
    cursor->indexPos--;
}

void backspaceMiddleOfString(Line *inputBuffer[], int currentLine, Cursor *cursor) {
    char *slicedString = sliceStringByIndex(inputBuffer[currentLine]->string, cursor->indexPos, inputBuffer[currentLine]->numChars);
    cursor->indexPos--;
    inputBuffer[currentLine]->string[cursor->indexPos] = '\0';
    for (int i = 0; i < strlen(slicedString); i++) {
        inputBuffer[currentLine]->string[cursor->indexPos + i] = slicedString[i]; 
    }
    inputBuffer[currentLine]->string[cursor->indexPos + strlen(slicedString)] = '\0';
    inputBuffer[currentLine]->numChars--;
}

void handleBackspace(Line *inputBuffer[], int *currentLine, int *totalNumLines, Cursor *cursor, int *beginDisplay) {
    // Handling backspaces when the cursor's index position is at the end of the string
    if (cursor->indexPos > 0 && cursor->indexPos == inputBuffer[*currentLine]->numChars) {
        // Handling backspaces when the string is only 1 character long
        if (inputBuffer[*currentLine]->numChars == 1) {
            // Handling backspaces when the currentline is not the first line (0)
            if (*currentLine != 0) {
                free(inputBuffer[*currentLine]);
                inputBuffer[*currentLine] = NULL;
                (*currentLine)--;
                (*totalNumLines)--;
                cursor->line = 0;
                cursor->indexPos = inputBuffer[*currentLine]->numChars;
            }
            else {
                inputBuffer[*currentLine]->string[0] = '\0';
                inputBuffer[*currentLine]->numChars = 0;
                cursor->indexPos--;
                *beginDisplay = 0;
            }
        }
        else {
            backspaceEndOfString(inputBuffer, *currentLine, cursor);
        }
    }
    // 
    else if (cursor->indexPos > 0 && cursor->indexPos < inputBuffer[*currentLine]->numChars) {
        backspaceMiddleOfString(inputBuffer, *currentLine, cursor);
    }
    // else if (cursor->indexPos == 0) {
    //     ;
    // }
}

void formatCharsAppend(Line *inputBuffer[], int key, int currentLine, Cursor *cursor) {
    inputBuffer[currentLine]->string[cursor->indexPos] = (char)key;
    inputBuffer[currentLine]->string[cursor->indexPos + 1] = '\0';
    inputBuffer[currentLine]->numChars++;
    cursor->indexPos++;
}

void formatCharsMiddle(Line *inputBuffer[], int key, int currentLine, Cursor *cursor) {
    char *slicedString = sliceStringByIndex(inputBuffer[currentLine]->string, cursor->indexPos, inputBuffer[currentLine]->numChars);
    inputBuffer[currentLine]->string[cursor->indexPos] = (char)key;
    cursor->indexPos++;
    for (int i = 0; i < strlen(slicedString); i++) {
        inputBuffer[currentLine]->string[cursor->indexPos + i] = slicedString[i];
    }
    inputBuffer[currentLine]->string[cursor->indexPos + strlen(slicedString)] = '\0';
    inputBuffer[currentLine]->numChars++;
}

void handleCharacterInput(int key, Line *inputBuffer[], int *shiftToggle, int currentLine, Cursor *cursor) {
    if (*shiftToggle == 0) {
        // Handles keys not on the shift layer
        if (key >= KEY_A && key <= KEY_Z) {
            key += 32;
        }
    }
    else {
        // If shift button is being held
        handleSpecialChars(&key);
    }
    // Add the keys to the string array, but the keys must be printable ascii keys which excludes shift and cntrl
    if (key >= 32 && key <= 127) {
        if (cursor->indexPos == inputBuffer[currentLine]->numChars) {
            // When appending to the string array (i.e. editing at the end of the string)
            formatCharsAppend(inputBuffer, key, currentLine, cursor);
        }
        else {
            // Editing the string on an index position within the string
            formatCharsMiddle(inputBuffer, key, currentLine, cursor);
        }
    }
}

void handleEnter(Line *inputBuffer[], int *currentLine, int *totalNumLines, Cursor *cursor) {
    if (*totalNumLines < maxNumLines) {
        (*currentLine)++;
        (*totalNumLines)++;

        if (inputBuffer[*currentLine] == NULL) {
            inputBuffer[*currentLine] = malloc(sizeof(Line));
            inputBuffer[*currentLine]->numChars = 0;
            inputBuffer[*currentLine]->string[0] = '\0';
        }
        cursor->line++;
        cursor->indexPos = 0;

    } else {
        // stderr is an error stream
        fprintf(stderr, "\nYOU HAVE REACHED THE MAX NUMBER OF LINES\n\n");
        return ;
    }
}

void handleKeyPress(int key, Line *inputBuffer[], int *beginDisplay, int *shiftToggle,  int delTime[], int *currentLine, int *totalNumLines, Cursor *cursor) {
    // Handles rapid deleting with a counter buffer
    if (IsKeyDown(KEY_BACKSPACE)) {
        delTime[0]++;
        if (delTime[0] > backspaceBuffer) {
            handleBackspace(inputBuffer, currentLine, totalNumLines, cursor, beginDisplay);
        }
    }
    else {
        delTime[0] = 0;
    }

    if (key > 0) {
        // edge case for first time displaying a character
        if (*beginDisplay == 0) {
            *beginDisplay = 1;
        }
        if (*currentLine < 0 || *currentLine >= maxNumLines) {
            fprintf(stderr, "\nINVALID LINE INDEX BITCH\n\n");
            return ;
        }
        // Everytime a key is pressed, we check to see if shift is held down
        // and update the shiftToggle flag accordingly
        updateShiftState(key, shiftToggle);
        if (key == KEY_BACKSPACE) {
            // Single Delete
            handleBackspace(inputBuffer, currentLine, totalNumLines, cursor, beginDisplay);
        }
        else if (key == KEY_ENTER) {
            handleEnter(inputBuffer, currentLine, totalNumLines, cursor);
        }
        else if (key == KEY_LEFT) {
            cursor->indexPos--;
            if (cursor->indexPos < 0) {
                cursor->indexPos = 0;
            }
        }
        else if (key == KEY_RIGHT) {
            cursor->indexPos++;
            if (cursor->indexPos > inputBuffer[*currentLine]->numChars) {
                cursor->indexPos = inputBuffer[*currentLine]->numChars;
            }
        }
        else if (key != KEY_LEFT_SHIFT && key != KEY_RIGHT_SHIFT){
            handleCharacterInput(key, inputBuffer, shiftToggle, *currentLine, cursor);
        }
        printf("%s\n", inputBuffer[*currentLine]->string);  // debug printf
        // printf("%d\n", key);  // debug printf
        // printf("Indexpos - %d\n", cursor->indexPos);
        // printf("LineNum - %d\n", cursor->line);
    }
    // printf("\nIndexpos is %d\n", cursor->indexPos);
}

void drawCursor(int *beginDisplay, Rectangle inputBox, Line *inputBuffer[], int key, int blinkingClock[], Font font, Cursor *cursor, int currentLine) {
    Vector2 startPos, endPos;
    if (*beginDisplay == 1) {
        // Start and end positions for the cursor when there is text on screen
        int startIndex = 0;
        char *slicedString = sliceStringByIndex(inputBuffer[currentLine]->string, startIndex, cursor->indexPos);
        Vector2 textSize = MeasureTextEx(font, slicedString, fontSize, fontSpacing);
        startPos = (Vector2){ inputBox.x + textSize.x, inputBox.y + (textSize.y * (currentLine))};
        endPos = (Vector2){ inputBox.x + textSize.x, inputBox.y + (textSize.y * (currentLine + 1))};
        free(slicedString);
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

void drawInput(Rectangle inputBox, Line *inputBuffer[], int *beginDisplay, Font font, int key, int blinkingClock[], int currentLine, int totalNumLines, Cursor *cursor) {
    if (*beginDisplay == 1) {
        // Draw text input
        Vector2 pos = { inputBox.x, inputBox.y };
        for (int i = 0; i < totalNumLines; i++) {
            DrawTextEx(font, inputBuffer[i]->string, pos, fontSize, fontSpacing, WHITE);
            pos.y += fontSize;
        }
    }

    drawCursor(beginDisplay, inputBox, inputBuffer, key, blinkingClock, font, cursor, currentLine);
}

int main() {
    InitWindow(screenWidth, screenHeight, "text-editor-bitch!");
    SetTargetFPS(fpsCap);

    Font font = LoadFont("resources/Inter-Regular-slnt=0.ttf");

    if (font.baseSize == 0) {
        fprintf(stderr, "\nFailed to load font!\n\n");
    }
    Line *inputBuffer[maxNumLines] = { NULL };

    Rectangle inputBox = { 5, 0, 300, 25 };

    int totalNumLines = 1;
    int beginDisplay = 0;  // flag
    int shiftFlag = 0;     // flag
    int delTime[1] = {0};  // counter
    int blinkingClock[1] = {0};  // counter
    int currentLine = 0;    // the current line number that cursor is

    inputBuffer[currentLine] = malloc(sizeof(Line));
    inputBuffer[currentLine]->numChars = 0;
    inputBuffer[currentLine]->string[0] = '\0';

    Cursor cursor = { 0, 1 };
    // TODO: HAVE A KEYBIND TO DISPLAY A GRID LIKE IN EXAMPLE VID
    // TODO: BE ABLE TO CTRL +, CTRL -, TO ZOOM IN AND ZOOM OUT TO SHRINK AND ENLARGE TEXT/GRID
    while (!WindowShouldClose()) {
        int key = GetKeyPressed();
        // if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_O)) {
        //     // Open a text file in write mode
        //     // FILE *pf = fopen("test.txt", "w");
        //     // for (int i = 0; i < totalNumLines; i++) {
        //     //     fprintf(pf, "%s\n", inputBuffer[i]->string);
        //     // }
        //     // fclose(pf);
        // }
        handleKeyPress(key, inputBuffer, &beginDisplay, &shiftFlag, delTime, &currentLine, &totalNumLines, &cursor);
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("CTRL + O to save", 20, 520, 10, WHITE);
        drawInput(inputBox, inputBuffer, &beginDisplay, font, key, blinkingClock, currentLine, totalNumLines, &cursor);
        EndDrawing();
    }
    // If accidently closed, then automatically write input buffer contents to the file
    // FILE *pf = fopen("test.txt", "w");
    // // new lines appended automatically to each line string
    // for (int i = 0; i < totalNumLines; i++) {
    //     fprintf(pf, "%s\n", inputBuffer[i]->string);
    // }
    // fclose(pf);
    // Cleaning up
    for (int i = 0; i < totalNumLines; i++) {
        if (inputBuffer[i] != NULL) {
            free(inputBuffer[i]);
            inputBuffer[i] = NULL;
            printf("\nLine %d has been freed\n\n", i);

        }
    }

    CloseWindow();
    return 0;
}