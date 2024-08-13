CC = x86_64-w64-mingw32-gcc
CFLAGS = -I./SDL2/include -Dmain=SDL_main
LDFLAGS = -L./SDL2/lib -lSDL2main -lSDL2 -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid -static-libgcc

TARGET = quadTree.exe
SRCS = main.c

$(TARGET):$(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)
