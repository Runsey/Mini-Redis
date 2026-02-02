# Compiler
CXX := g++

# Target binary
TARGET := mini_redis

# Source files (all .cpp in directory)
SRCS := $(wildcard *.cpp)

# Object files
OBJS := $(SRCS:.cpp=.o)

# Compile flags
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic \
            -g -O0 \
            -fsanitize=address,undefined \
            -fno-omit-frame-pointer

# Link flags (sanitizers must be here too)
LDFLAGS := -fsanitize=address,undefined

# Default target
all: $(TARGET)

# Link
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Compile
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(OBJS) $(TARGET)

# Run with ASan enabled
run: all
	./$(TARGET)
