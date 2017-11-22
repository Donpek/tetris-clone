OUT=tetris-clone

all:
	make build;
	make run;

build: *.h
	gcc *.c -o $(OUT) -lSDL2 -I/usr/include/SDL2;

run:
	./$(OUT);

clean:
	rm $(OUT);

windows: *.h
	/usr/mingw64/bin/x86_64-w64-mingw32-gcc *.c -o $(OUT).exe -I/usr/include/SDL2 -lSDL2main -lSDL2 -lmingw32 -mwindows;
