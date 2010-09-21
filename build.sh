gcc -O2 graphics.c main.c gl_shapes.c -o main -lm -lcv -lhighgui -lcvaux -lglut -lGLU $(pkg-config --libs --cflags sdl)
