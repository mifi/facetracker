gcc -O2 main.c opt_flow.c -o main -lm -lcv -lGLU $(pkg-config --libs --cflags sdl opencv)
