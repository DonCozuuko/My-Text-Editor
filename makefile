main:
	gcc editor.c special_characters.c -o e -Iinclude -Llib -lraylib -lopengl32 -lgdi32 -lwinmm \
					-Wall -Werror -Wextra