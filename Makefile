SRC_DIR := src/RaspberryLatte
OBJ_DIR := obj/RaspberryLatte
BIN_DIR := bin

EXE := $(BIN_DIR)/RaspberryLatte
SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

$(info $(EXE))
$(info $(SRC))
$(info $(OBJ))

CXXPPFLAGS := -std=c++11 -Iinclude/RaspberryLatte -MMD -MP 
CXXFLAGS   := -Wall -Wno-psabi -lpaho-mqtt3c
LDFLAGS  := -Llib
LDLIBS   := -lpigpio -lrt -lpaho-mqtt3c -lpaho-mqttpp3

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJ) | $(BIN_DIR)
	g++ -std=c++11 $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	g++ $(CXXPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)

-include $(OBJ:.o=.d)
