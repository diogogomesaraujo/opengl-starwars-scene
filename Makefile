# Compiler and flags
CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra
INCLUDES = -I/opt/homebrew/include -I$(PWD)/include -I$(PWD)/glad/include -I$(PWD)/headers -I/opt/homebrew/include/assimp -I/opt/homebrew/include/freetype2 -I/opt/homebrew/include/GL
LDFLAGS = -L/opt/homebrew/lib
LIBS = -lglfw -framework OpenGL -lassimp -lfreetype -framework GLUT -lsfml-audio

# Directories
SRC_DIR = $(PWD)
OBJ_DIR = $(PWD)/obj
HEADERS_DIR = $(PWD)/headers
GLAD_DIR = $(PWD)/glad
INCLUDE_DIR = $(PWD)/include
SHADERS_DIR = $(PWD)/shaders

# Source files
SOURCES = $(SRC_DIR)/main.cpp $(GLAD_DIR)/src/glad.c
HEADERS = $(wildcard $(HEADERS_DIR)/*.h) $(wildcard $(INCLUDE_DIR)/*.h)

# Object files
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(filter %.cpp, $(SOURCES))) \
          $(patsubst $(GLAD_DIR)/src/%.c, $(OBJ_DIR)/%.o, $(filter %.c, $(SOURCES)))

# Target executable
TARGET = app

# Main target
$(TARGET): $(OBJECTS)
	@echo "Linking target: $@"
	$(CXX) $(OBJECTS) $(LDFLAGS) $(LIBS) -o $@

# Compile C++ source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Compile C source files (for glad)
$(OBJ_DIR)/%.o: $(GLAD_DIR)/src/%.c
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean target
clean:
	@echo "Cleaning up..."
	rm -rf $(OBJ_DIR) $(TARGET)

# Run target
run: $(TARGET)
	@echo "Running target..."
	./$(TARGET)

.PHONY: clean run
