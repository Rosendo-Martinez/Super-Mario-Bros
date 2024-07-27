CXX := g++
OUTPUT := sfmlgame

CXX_FLAGS := -O3 -std=c++17
LDFLAGS := -O3 -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

# Directories
SRCDIR := ./src
BINDIR := ./bin

# Source and object files
SOURCES := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS := $(patsubst $(SRCDIR)/%.cpp,$(BINDIR)/%.o,$(SOURCES))

# Target
all: $(BINDIR)/game.exe

# Link the final executable
$(BINDIR)/game.exe: $(OBJECTS)
	$(CXX) $(CXX_FLAGS) $(OBJECTS) $(LDFLAGS) -o $@

# Compile the .cpp files into .o files
$(BINDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXX_FLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(BINDIR)/*.o $(BINDIR)/*.exe
