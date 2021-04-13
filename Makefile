SRC_DIR := src/RaspberryLatte
OBJ_DIR := obj/RaspberryLatte
BIN_DIR := .

EXE := $(BIN_DIR)/RaspberryLatte
SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

$(info $(EXE))
$(info $(SRC))
$(info $(OBJ))

CXXPPFLAGS := -Iinclude/RaspberryLatte -MMD -MP
CXXFLAGS   := -Wall -Wno-psabi
LDFLAGS  := -Llib
LDLIBS   := -lpigpio -lrt

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJ) | $(BIN_DIR)
	g++ $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	g++ $(CXXPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)

-include $(OBJ:.o=.d)
