main:
	gcc editor.c special_characters.c -o e -IC:\raylib\raylib\src -LC:\raylib\raylib\src -lraylib -lopengl32 -lgdi32 -lwinmm \
					-Wall -Werror -Wextra