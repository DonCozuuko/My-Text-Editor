# Matrix-Text-Editor
A basic text editor made using the raylib graphics library, written in C. 

## Features
- Reads and writes files.
- ctrl + s to save to the file while in the editor
- Scrolls up and down frames with ctrl + up/down arrow.
- Text color is hard coded green, hence MATRIX.
- Okay so at the time of writing this you CANNOT write to new files, and you CANNOT save to any file that is NOT a .txt file. Dumb bug but fear not fellow reader I will fix it eventually...

## Demo
![][res/editor_demo.png]
## Usage
 1. Editing a text file from the terminal
```
 $ ./editor.exe <file_name.file_extension>
```
2. Text will automatically save when you exit the window.

## Build from Source
```
$ make
```
