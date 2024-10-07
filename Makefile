cc := g++
sourceDir := src
binaryDir := bin
objectDir := $(binaryDir)/obj
includeDir := include
libraryDir := lib
libraries := raylib gdi32 winmm
executable := $(binaryDir)/main.exe
flags := -static -static-libgcc -static-libstdc++
warnings := all

libraries := $(addprefix -l, $(libraries))
warnings := $(addprefix -W, $(warnings))

sources := $(wildcard $(sourceDir)/*.cpp)
objects := $(patsubst $(sourceDir)/%.cpp, $(objectDir)/%.o, $(sources))

default: run

$(executable): $(objects)
	@python checkDir.py --path="$(dir $@)"
	$(cc) $(warnings) $^ -o $@ $(addprefix -L, $(includeDir)) $(addprefix -L, $(libraryDir)) $(libraries) $(flags)

$(objectDir)/%.o: $(sourceDir)/%.cpp
	@python checkDir.py --path="$(dir $@)"
	$(cc) $(warnings) -c $< -o $@ $(addprefix -I, $(includeDir)) $(flags)

run: $(executable)
	"$<"

clean:
	- del /q "$(subst /,\,$(executable))"
	- rmdir /s /q "$(objectDir)"