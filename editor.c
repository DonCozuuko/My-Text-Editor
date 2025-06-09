#include <stdio.h>
#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include "special_characters.h"

static const int screenWidth = 960;
static const int screenHeight = 540;
static const int fpsCap = 60;

#define MAX_INPUT_CHARS 200
#define maxNumLines 50

static const float fontSize = 25.0;
static const float fontSpacing = 2.0;
static const int blinkRate = 40;        // how fast the cursor blinks
static const int backspaceBuffer = 25;  // how long it takes to hold down backspace to rapidly backspace

#define textColor   GREEN
#define lineColor   RED
#define cursorColor GREEN

#define UPPER_SCROLL_BOUND 2
#define LOWER_SCROLL_BOUND 21

#define SAFE_FREE(ptr) do { \
    if (ptr != NULL) { \
        printf("\nFreeing pointer at %p\n", (void*)ptr); \
        free(ptr); \
        ptr = NULL; \
    } \
} while (0)

typedef struct {
    char string[MAX_INPUT_CHARS + 1];
    int numChars;
} Line;

typedef struct {
    int indexPos;
    int line;
} Cursor;

char *sliceStringByIndex(char *string, int startIndex, int endIndex) {
    char *copy = malloc(endIndex + 1);              // make copy of string
    strncpy(copy, string + startIndex, endIndex);   // slice
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

void backspaceEndOfString(Line *inputBuffer[], Cursor *cursor) {
    inputBuffer[cursor->line]->string[cursor->indexPos - 1] = '\0';
    inputBuffer[cursor->line]->numChars--;
    cursor->indexPos--;
}

void backspaceMiddleOfString(Line *inputBuffer[], Cursor *cursor) {
    char *slicedString = sliceStringByIndex(inputBuffer[cursor->line]->string, cursor->indexPos, inputBuffer[cursor->line]->numChars);
    cursor->indexPos--;
    inputBuffer[cursor->line]->string[cursor->indexPos] = '\0';  // delete rest of line from deleted index to end of line
    // Shift the sliced string 1 index left
    for (int i = 0; i < strlen(slicedString); i++) {
        inputBuffer[cursor->line]->string[cursor->indexPos + i] = slicedString[i]; 
    }
    inputBuffer[cursor->line]->string[cursor->indexPos + strlen(slicedString)] = '\0';
    inputBuffer[cursor->line]->numChars--;
}

void printInputBuffer(Line *inputBuffer[], int totalNumLines) {
    // Debuf function
    printf("----- BUFFER STATE -----\n");
    for (int i = 0; i < totalNumLines; i++) {
        printf("[%d] %p | \"%s\"\n", i, (void*)inputBuffer[i],
            inputBuffer[i] ? inputBuffer[i]->string : "NULL");
    }
    printf("------------------------\n");
}

void removeLineAndJumpUp(Line *inputBuffer[], Cursor *cursor, int *totalNumLines) {
    int lineToRemove = *totalNumLines - 1;
    Line *lineToFree = inputBuffer[lineToRemove];
    // Behaviour for when shifting lines up 1, underneath the line being deleted
    if (cursor->line != *totalNumLines - 1) {
        char string[MAX_INPUT_CHARS];
        strcpy(string, lineToFree->string);
        // printf("\n\n STRING - %s \n\n", string);
        // printf("\n\nSTART LOOP\n\n");
        for (int i = cursor->line; i < *totalNumLines - 2; i++) {
            inputBuffer[i] = inputBuffer[i + 1];
            // printInputBuffer(inputBuffer, *totalNumLines);
        }
        // printf("\n\nEXIT LOOP\n\n");
        inputBuffer[*totalNumLines - 2] = malloc(sizeof(Line));
        strcpy(inputBuffer[*totalNumLines - 2]->string, string);
        inputBuffer[*totalNumLines - 2]->numChars = strlen(string);
        // printInputBuffer(inputBuffer, *totalNumLines);
    }
    SAFE_FREE(lineToFree);  // free() and prevent dangling pointer by reassigning to NULL
    // printf("\nLINE %d HAS BEEN FREED\n\n", lineToRemove);
    // Jump up
    cursor->line--;
    (*totalNumLines)--;
    cursor->indexPos = inputBuffer[cursor->line]->numChars;
}

void formatCharsAppend(Line *inputBuffer[], char key, Cursor *cursor) {
    // Append new character to the array aswell as the NULL terminator the index right of it
    inputBuffer[cursor->line]->string[cursor->indexPos] = (char)key;
    inputBuffer[cursor->line]->string[cursor->indexPos + 1] = '\0';
    inputBuffer[cursor->line]->numChars++;
    cursor->indexPos++;
}

void handleBackspace(Line *inputBuffer[], int *totalNumLines, Cursor *cursor) {
    // Handling backspaces when the cursor's index position is at the end of the string
    // <cursor->indexPos > 0> conjunct is necessary because there are seperate behaviors for when 
    // we have an empty string and a non empty string where the cursor index position is numChars
    if (cursor->indexPos > 0 && cursor->indexPos == inputBuffer[cursor->line]->numChars) {
        backspaceEndOfString(inputBuffer, cursor);
    }
    // Handling backspaces when the cursor's index position is in the middle of the string
    else if (cursor->indexPos > 0 && cursor->indexPos < inputBuffer[cursor->line]->numChars) {
        backspaceMiddleOfString(inputBuffer, cursor);
    }
    // Handling backspaces when the cursor's index position is 0 (2 edge cases)
    // These edge cases dont apply when line 0 is the only line and there are no chars in line 0
    else if (cursor->indexPos == 0 && cursor->line != 0) {
        // When the line is empty
        if (inputBuffer[cursor->line]->numChars == 0) {
            // printInputBuffer(inputBuffer, *totalNumLines);
            removeLineAndJumpUp(inputBuffer, cursor, totalNumLines);
            // printInputBuffer(inputBuffer, *totalNumLines);
        }
        else {
            char *text = inputBuffer[cursor->line]->string;
            removeLineAndJumpUp(inputBuffer, cursor, totalNumLines);
            int newCursorPos = cursor->indexPos;
            for (int i = 0; i < strlen(text); i++) {
                formatCharsAppend(inputBuffer, (int)text[i], cursor);
            }
            cursor->indexPos = newCursorPos;
        }
    }
}


void formatCharsMiddle(Line *inputBuffer[], int key, Cursor *cursor) {
    char *slicedString = sliceStringByIndex(inputBuffer[cursor->line]->string, cursor->indexPos, inputBuffer[cursor->line]->numChars);
    inputBuffer[cursor->line]->string[cursor->indexPos] = (char)key;
    cursor->indexPos++;
    for (int i = 0; i < strlen(slicedString); i++) {
        inputBuffer[cursor->line]->string[cursor->indexPos + i] = slicedString[i];
    }
    inputBuffer[cursor->line]->string[cursor->indexPos + strlen(slicedString)] = '\0';
    inputBuffer[cursor->line]->numChars++;
}

void handleCharacterInput(int key, Line *inputBuffer[], int *shiftToggle, Cursor *cursor) {
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
        if (cursor->indexPos == inputBuffer[cursor->line]->numChars) {
            // When appending to the string array (i.e. editing at the end of the string)
            formatCharsAppend(inputBuffer, key, cursor);
        }
        else {
            // Editing the string on an index position within the string
            formatCharsMiddle(inputBuffer, key, cursor);
        }
    }
}

void addLineAndJumpDown(Line *inputBuffer[], Cursor *cursor, int *totalNumLines) {
    cursor->line++;  // Jump Down
    // When there is nothing below the current line
    if (inputBuffer[cursor->line] == NULL) {
        inputBuffer[cursor->line] = malloc(sizeof(Line));
        inputBuffer[cursor->line]->numChars = 0;
        inputBuffer[cursor->line]->string[0] = '\0';
    }
    // When there is lines below the current line
    else {
        int newLastLine = *totalNumLines;
        inputBuffer[newLastLine] = malloc(sizeof(Line));
        inputBuffer[newLastLine]->numChars = 0;
        inputBuffer[newLastLine]->string[0] = '\0';
    }
    (*totalNumLines)++;
    cursor->indexPos = 0;
}

Line *createNewLine() {
    Line *newLine = malloc(sizeof(Line));
    newLine->numChars = 0;
    newLine->string[0] = '\0';
    return newLine;
}

void enterCursorEndString(Line *inputBuffer[], int *totalNumLines, Cursor *cursor) {
    char copiedString[MAX_INPUT_CHARS];
    // Copy the string from the line below the line with the cursor before going down
    if (inputBuffer[cursor->line + 1] != NULL) {
        strcpy(copiedString, inputBuffer[cursor->line + 1]->string);
    }
    // Cursor jumps down a line and makes a new line at bottom of file for shifting
    addLineAndJumpDown(inputBuffer, cursor, totalNumLines);
    // Delete the line the cursor jumped down to
    inputBuffer[cursor->line]->string[0] = '\0';
    inputBuffer[cursor->line]->numChars = 0;
    // Only executes if the current line isnt the last line in the file
    if (cursor->line != *totalNumLines - 1) {
        // Iterate through lines, starts from the bottom line and stops at line below current jumped line
        for (int i = *totalNumLines - 1; i > cursor->line + 1; i--) {
            inputBuffer[i] = inputBuffer[i - 1];
        }
        // Now re-allocate the line below current jumped line to have the copied string -- the string which was below the initial string
        // This is to make sure that the string, twice below the current jumped line, isnt unintentionally modified (since its a pointer)
        inputBuffer[cursor->line + 1] = createNewLine();
        strcpy(inputBuffer[cursor->line + 1]->string, copiedString);
        inputBuffer[cursor->line + 1]->numChars = strlen(inputBuffer[cursor->line + 1]->string);
    }
}

void enterCursorMiddleString(Line *inputBuffer[], int *totalNumLines, Cursor *cursor) {
    char copiedString[MAX_INPUT_CHARS];
    if (inputBuffer[cursor->line + 1] != NULL) {
        strcpy(copiedString, inputBuffer[cursor->line + 1]->string);
    }
    // Slice right half of string and make adjustments to the line cursor is on
    char *slicedString = sliceStringByIndex(inputBuffer[cursor->line]->string, cursor->indexPos, inputBuffer[cursor->line]->numChars);
    inputBuffer[cursor->line]->string[cursor->indexPos] = '\0';
    inputBuffer[cursor->line]->numChars -= strlen(slicedString);

    addLineAndJumpDown(inputBuffer, cursor, totalNumLines);
    // Append sliced string to the line cursor jumped down to
    inputBuffer[cursor->line]->string[0] = '\0';
    strcpy(inputBuffer[cursor->line]->string, slicedString);
    inputBuffer[cursor->line]->numChars = strlen(slicedString);
    
    if (cursor->line != *totalNumLines - 1) {
        for (int i = *totalNumLines - 1; i > cursor->line + 1; i--) {
            inputBuffer[i] = inputBuffer[i - 1];
        }
        inputBuffer[cursor->line + 1] = createNewLine();
        strcpy(inputBuffer[cursor->line + 1]->string, copiedString);
        inputBuffer[cursor->line + 1]->numChars = strlen(inputBuffer[cursor->line + 1]->string);
    }
}

void enterBeginningNonEmptyString(Line *inputBuffer[], int *totalNumLines, Cursor *cursor) {
    char copiedString[MAX_INPUT_CHARS];
    strcpy(copiedString, inputBuffer[cursor->line]->string);
    addLineAndJumpDown(inputBuffer, cursor, totalNumLines);
    // Go back to the initial line the cursor was on before jumping down and delete it
    inputBuffer[cursor->line - 1]->string[0] = '\0';
    inputBuffer[cursor->line - 1]->numChars = 0;
    if (cursor->line != *totalNumLines - 1) {
        for (int i = *totalNumLines - 1; i > cursor->line; i--) {
            inputBuffer[i] = inputBuffer[i - 1];
        }
        inputBuffer[cursor->line] = createNewLine();
    }
    strcpy(inputBuffer[cursor->line]->string, copiedString);
    inputBuffer[cursor->line]->numChars = strlen(inputBuffer[cursor->line]->string);
}

void handleEnter(Line *inputBuffer[], int *totalNumLines, Cursor *cursor) {
    // Handling enter when cursor is at the end of the line's string
    if (cursor->indexPos > 0 && cursor->indexPos == inputBuffer[cursor->line]->numChars) {
        enterCursorEndString(inputBuffer, totalNumLines, cursor);
    }
    // Handling enter when cursor is in the middle of the line's string
    else if (cursor->indexPos > 0 && cursor->indexPos < inputBuffer[cursor->line]->numChars){
        enterCursorMiddleString(inputBuffer, totalNumLines, cursor);
    }
    // Handling enter when the cursor is at index 0 of the line's string
    else if (cursor->indexPos == 0) {
        // Empty Line
        if (inputBuffer[cursor->line]->numChars == 0) {
            enterCursorEndString(inputBuffer, totalNumLines, cursor);
        }
        // Non-empty Line
        else {
            enterBeginningNonEmptyString(inputBuffer, totalNumLines, cursor);
        }
    }
}

void handleTabs(Line *inputBuffer[], Cursor *cursor) {
    int spaceKey = 32;
    for (int i = 0; i < 4; i++) {
        inputBuffer[cursor->line]->string[cursor->indexPos] = (char)spaceKey;
        inputBuffer[cursor->line]->numChars++;
        cursor->indexPos++;
    }
    inputBuffer[cursor->line]->string[cursor->indexPos] = '\0';
}

void handleKeyPress(int key, Line *inputBuffer[], int *beginDisplay, int *shiftToggle,  int delTime[], int *totalNumLines, Cursor *cursor, int *defaultViewingLine) {
    // Handles rapid deleting with a counter buffer
    if (IsKeyDown(KEY_BACKSPACE)) {
        delTime[0]++;
        if (delTime[0] > backspaceBuffer) {
            handleBackspace(inputBuffer, totalNumLines, cursor);
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
        if (cursor->line < 0 || cursor->line >= maxNumLines) {
            fprintf(stderr, "\nINVALID LINE INDEX BITCH\n\n");
            return ;
        }
        // Everytime a key is pressed, we check to see if shift is held down
        // and update the shiftToggle flag accordingly
        updateShiftState(key, shiftToggle);
        if (key == KEY_BACKSPACE) {
            // Single Delete
            handleBackspace(inputBuffer, totalNumLines, cursor);
            if ((cursor->line - *defaultViewingLine) == UPPER_SCROLL_BOUND) {
                printf("\nACTIVATED\n");
                (*defaultViewingLine)--;
                // To prevent seg faults, since if *DFV is -1, there is no index -1 in line buffers array
                if (*defaultViewingLine < 0) {
                    *defaultViewingLine = 0;
                }
            }
        }
        else if (key == KEY_ENTER) {
            if (*totalNumLines + 1 <= maxNumLines) {
                handleEnter(inputBuffer, totalNumLines, cursor);
                if ((cursor->line - *defaultViewingLine) == LOWER_SCROLL_BOUND) {
                    (*defaultViewingLine)++;
                }
            }
            else {
                fprintf(stderr, "\nYOU HAVE REACHED THE MAX NUMBER OF LINES\n\n");
                return ;    
            }
        }
        else if (key == KEY_LEFT) {
            cursor->indexPos--;
            if (cursor->indexPos < 0) {
                cursor->indexPos = 0;
            }
        }
        else if (key == KEY_RIGHT) {
            cursor->indexPos++;
            if (cursor->indexPos > inputBuffer[cursor->line]->numChars) {
                cursor->indexPos = inputBuffer[cursor->line]->numChars;
            }
        }
        else if (key == KEY_UP) {
            // To avoid seg faults since line -1 can't exist
            if (cursor->line != 0) {
                int prevPos = cursor->indexPos;
                cursor->line--;
                if (prevPos > inputBuffer[cursor->line]->numChars) {
                    cursor->indexPos = inputBuffer[cursor->line]->numChars;
                }
                else {
                    cursor->indexPos = prevPos;
                }
                if ((cursor->line - *defaultViewingLine) == UPPER_SCROLL_BOUND) {
                    if (*defaultViewingLine != 0) {
                        (*defaultViewingLine)--;
                    }
                }
            }
        }
        else if (key == KEY_DOWN) {
            if (cursor->line < *totalNumLines - 1) {
                cursor->line++;
                int prevPos = cursor->indexPos;
                if (prevPos > inputBuffer[cursor->line]->numChars) {
                    cursor->indexPos = inputBuffer[cursor->line]->numChars;
                }
                else {
                    cursor->indexPos = prevPos;
                }
                if ((cursor->line - *defaultViewingLine) == LOWER_SCROLL_BOUND) {
                    (*defaultViewingLine)++;
                }
            }
        }
        else if (key == KEY_TAB) {
            handleTabs(inputBuffer, cursor);
        }
        else if (key != KEY_LEFT_SHIFT && key != KEY_RIGHT_SHIFT){
            handleCharacterInput(key, inputBuffer, shiftToggle, cursor);
        }
        // printf("%s - Line %d - Position %d - NumChars %d\n", inputBuffer[cursor->line]->string, cursor->line, cursor->indexPos, inputBuffer[cursor->line]->numChars);  // debug printf
        // printf("%d\n", inputBuffer[cursor->line]->numChars);
        // printf("%d\n", key);  // debug printf
        // printf("Indexpos - %d\n", cursor->indexPos);
        // printf("LineNum - %d, ViewingLine - %d\n", cursor->line, *defaultViewingLine);
        // printf("Total Lines = %d\n", *totalNumLines);
        // printf("VIEWING LINE - %d\n", *defaultViewingLine);
        // printf("CursorLine - %d, DVL - %d\n", cursor->line, *defaultViewingLine);
    }
    // printf("\nIndexpos is %d\n", cursor->indexPos);

}

void DrawCursor(Vector2 startingPos, Line *inputBuffer[], int key, int blinkingClock[], Font font, Cursor *cursor, int *defaultViewingLine) {
    char *lineText = inputBuffer[cursor->line]->string;
    int cursorIndex = cursor->indexPos;
    Vector2 textSize = {0};

    if (cursorIndex > 0) {
        char *sliceString = sliceStringByIndex(lineText, 0, cursorIndex);
        textSize = MeasureTextEx(font, sliceString, fontSize, fontSpacing);
        free(sliceString);
    }
    int drawnline = cursor->line - *defaultViewingLine;

    float cursorX = startingPos.x + textSize.x;
    float cursorY = startingPos.y + (fontSize * drawnline);
    // printf("CursorLine - %d, DrawnLine - %d, DVL - %d, CursorX - %f, CursorY - %f\n", cursor->line, drawnline, *defaultViewingLine, cursorX, cursorY);

    Vector2 startPos = { cursorX, cursorY };
    Vector2 endPos = { cursorX, cursorY + fontSize};
    // startPos = (Vector2){ inputBox.x, inputBox.y + (fontSize * cursor->line)};
    // endPos = (Vector2){ inputBox.x, inputBox.y + (fontSize * (cursor->line + 1)) };
    
    if (key > 0) {
        // Display solid cursor when typing
        blinkingClock[0] = 0;
        DrawLineV(startPos, endPos, cursorColor);
    }
    else {
        // Display a blinking cursor when idling (not typing) and have a slight buffer
        // from when where is no key input to blinking state
        // modulo arithmeitc is fucking magic yo
        blinkingClock[0]++;
        if ((blinkingClock[0] / blinkRate) % 2 == 0) {
            DrawLineV(startPos, endPos, cursorColor);
        }
    }
}

void DrawInput(Vector2 startingPos, Line *inputBuffer[], int *beginDisplay, Font font, int key, int blinkingClock[], int totalNumLines, Cursor *cursor, int *defaultViewingLine) {
    if (*beginDisplay == 1) {
        // Draw text input, flag conditional is for not drawing garbage values
        Vector2 pos = { startingPos.x, startingPos.y };
        for (int i = *defaultViewingLine; i < totalNumLines; i++) {
            // printf("\n%s\n\n", inputBuffer[i]->string);
            DrawTextEx(font, inputBuffer[i]->string, pos, fontSize, fontSpacing, textColor);
            pos.y += fontSize;
        }
    }
    DrawText("CTRL + O to save", 860, 520, 10, textColor);
    DrawCursor(startingPos, inputBuffer, key, blinkingClock, font, cursor, defaultViewingLine);
}

void DrawTextLines(Vector2 startingPos, Font font, int totalNumLines, int *defaultViewingLine) {
    int index = 0;
    for (int i = *defaultViewingLine; i < totalNumLines; i++) {
        int lineNum = i;
        if (i < 10) {
            char text[2];
            snprintf(text, 2, "%d", lineNum);
            Vector2 pos = { (startingPos.x / 2), startingPos.y + (fontSize * index)};
            DrawTextEx(font, text, pos, fontSize, fontSpacing, lineColor);
        } else { 
            char text[3];
            snprintf(text, 3, "%d", lineNum);
            Vector2 pos = { (startingPos.x / 5), startingPos.y + (fontSize * index)};
            DrawTextEx(font, text, pos, fontSize, fontSpacing, lineColor);
        }
        index++;
    }
}

void saveFunc(Line *inputBuffer[], int totalNumLines) {
    FILE *pf = fopen("test.txt", "w");
    // new lines appended automatically to each line string
    for (int i = 0; i < totalNumLines; i++) {
        fprintf(pf, "%s\n", inputBuffer[i]->string);
    }
    fclose(pf);
}

void shortCuts(Line *inputBuffer[], int *totalNumLines, Cursor *cursor, int *key, int *defaultViewingLine) {
    if (IsKeyDown(KEY_LEFT_CONTROL)) {
        if (IsKeyPressed(KEY_O)) {
            saveFunc(inputBuffer, *totalNumLines);
            *key = KEY_NULL;
        }
        else if (IsKeyPressed(KEY_UP)) {
            if (*defaultViewingLine > 0) {
                (*defaultViewingLine)--;
                *key = KEY_NULL;
            }
        }
        else if (IsKeyPressed(KEY_DOWN)) {
            if (*defaultViewingLine < *totalNumLines - LOWER_SCROLL_BOUND) {
                (*defaultViewingLine)++;
                *key = KEY_NULL;
            }
        }
    }
}

void freeCleanUp(Line *inputBuffer[], int totalNumLines) {
    for (int i = 0; i < totalNumLines; i++) {
        if (inputBuffer[i] != NULL) {
            free(inputBuffer[i]);
            inputBuffer[i] = NULL;
            printf("\nLine %d has been freed\n\n", i + 1);
    
        }
    }
}

int main() {
    // argc is argument count, argv is argument vector
    InitWindow(screenWidth, screenHeight, "text-editor-bitch!");
    SetTargetFPS(fpsCap);
    
    Texture2D img = LoadTexture("resources/hands.jpg");
    Font font = LoadFont("resources/Inter-Bold-slnt=0.ttf");

    if (font.baseSize == 0) {
        fprintf(stderr, "\nFailed to load font!\n\n");
    }
    Line *inputBuffer[maxNumLines] = { NULL };

    Vector2 startingPos = { 35.0, 0.0 };

    int totalNumLines = 1;       // counter
    int beginDisplay = 0;        // flag
    int shiftFlag = 0;           // flag
    int delTime[1] = {0};        // counter
    int blinkingClock[1] = {0};  // counter
    int defaultViewingLine = 0;  // counter
    
    Cursor cursor = { 0, 0 };

    // Allocating memory and initializing for the first line (line 0)
    inputBuffer[cursor.line] = malloc(sizeof(Line));
    inputBuffer[cursor.line]->numChars = 0;
    inputBuffer[cursor.line]->string[0] = '\0';

    while (!WindowShouldClose()) {
        int key = GetKeyPressed();
        shortCuts(inputBuffer, &totalNumLines, &cursor, &key, &defaultViewingLine);
        handleKeyPress(key, inputBuffer, &beginDisplay, &shiftFlag, delTime, &totalNumLines, &cursor, &defaultViewingLine);

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(img, (Rectangle){ 0,0,(float)img.width,(float)img.height }, (Rectangle){ 0,0,screenWidth,screenHeight}, (Vector2){0,0}, 0.0f, WHITE);
        DrawInput(startingPos, inputBuffer, &beginDisplay, font, key, blinkingClock, totalNumLines, &cursor, &defaultViewingLine);
        DrawTextLines(startingPos, font, totalNumLines, &defaultViewingLine);
        EndDrawing();
    }
    // If accidently closed, then automatically write input buffer contents to the file
    saveFunc(inputBuffer, totalNumLines);
    // Cleaning up
    freeCleanUp(inputBuffer, totalNumLines);
    CloseWindow();
    return 0;
}
