default:
	g++ -o bin/main src/*.cpp -Iinclude -Llib -Llib/raylib -lraylib -lgdi32 -lwinmm -static-libgcc -static-libstdc++ -static
	bin\main