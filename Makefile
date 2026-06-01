PROJECT = MeuTCC
CC = gcc
CFLAGS = -O2 -Wall -std=c99

# Detecta se é Windows ou Linux
ifeq ($(OS),Windows_NT)
    INCLUDES = -I include
    LDFLAGS = -L lib -lraylib -lopengl32 -lgdi32 -lwinmm
    TARGET = build/$(PROJECT).exe
else
    INCLUDES = 
    LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    TARGET = build/$(PROJECT)
endif

all:
	mkdir -p build
	$(CC) src/*.c $(CFLAGS) $(INCLUDES) -o $(TARGET) $(LDFLAGS)

clean:
	rm -rf build