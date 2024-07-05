# Compiler and flags
CXX = g++
CXXFLAGS = -Iinclude -Wall -Wextra -std=c++11

# Directories
SRCDIR = src
INCDIR = include
BINDIR = bin
OBJDIR = obj

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

# Target executable
TARGET = $(BINDIR)/sniffer

# Check if DEBUG is set
ifdef DEBUG
CXXFLAGS += -DDEBUG -g
endif

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(OBJECTS) snifferCPP/main.o
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) snifferCPP/main.o

# Build object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build main object file
snifferCPP/main.o: snifferCPP/main.cpp
	$(CXX) $(CXXFLAGS) -c snifferCPP/main.cpp -o snifferCPP/main.o

# Clean up
clean:
	rm -rf $(OBJDIR) $(BINDIR) snifferCPP/*.o

.PHONY: all clean
