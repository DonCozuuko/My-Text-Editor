main:
	gcc editor.c special_characters.c -o editor -Iinclude -Llib -lraylib -lopengl32 -lgdi32 -lwinmm
