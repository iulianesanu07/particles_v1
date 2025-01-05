TARGET = particles
SRC = code.c 

C = clang
CFLAGS = -Wall -I/opt/homebrew/include
LFLAGS = -L/opt/homebrew/lib -lglut -lGL -lGLU -framework OpenGL -framework Cocoa -framework IOKit

all: $(TARGET)

$(TARGET): $(SRC)
	$(C) $(CFLAGS) $(SRC) -o $(TARGET) $(LFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
