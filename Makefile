### Toolchain ###
CXX       := g++
CXXFLAGS  := -std=c++17 -O2 -Wall -lstdc++ -g
LDFLAGS 	:=
### Directories ###
SOURCEDIR	:= src
BUILDDIR	:= build
### Sources and objects ###
SOURCES		:= $(wildcard $(SOURCEDIR)/*.cpp)
OBJECTS		:= $(patsubst $(SOURCEDIR)/%.cpp, $(BUILDDIR)/%.o, $(SOURCES))
### Target executable (compiled gen.cpp) ###
TARGET		:= build/gen.exe

# make all - Compile all .cpp files in ./src to ./build
all: $(BUILDDIR) $(TARGET)

# Create build directory if it doesn't exist yet
$(BUILDDIR):
	@if not exist "$(BUILDDIR)" mkdir "$(BUILDDIR)"

# Compile .cpp -> .o
$(BUILDDIR)/%.o: $(SOURCEDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link .o -> .exe
$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) $^ -o $@

# make clean - Remove all exe files from ./build
clean:
	@if exist "$(BUILDDIR)" rmdir /S /Q "$(BUILDDIR)"
	@if exist "$(TARGET)" del /Q "$(TARGET)"

# make genpy - Run only the python program at ./src/gen.py
genpy:
	python src\gen.py

# make genpyall - Compile all .cpp files and run gen.py
genpyall: all genpy

.PHONY: all clean genpy genpyall

test:
	g++ archive/test.cpp -o archive/test
	./archive/test

server:
	node ./frontend/server.js