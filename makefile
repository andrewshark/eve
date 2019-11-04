BIN = bin/$(CXX)

ifeq ($(BUILD), release)

BIN := $(BIN)/$(BUILD)
COMPILER_FLAGS += -O3 -flto -DDISABLE_ASSERT

else ifeq ($(BUILD), debug)

BIN := $(BIN)/$(BUILD)
COMPILER_FLAGS += -g

else

COMPILER_FLAGS += -Os

endif

ifeq ($(CXX), g++)

COMPILER_FLAGS += -Wno-unused-but-set-variable
LINKER_FLAGS += -lrt

endif

ifeq ($(TARGET), test)

BIN := $(BIN)/$(TARGET)
EXE = $(BIN)/test
OBJS = $(BIN)/test.o $(BIN)/foundation.o $(BIN)/file.o $(BIN)/input.o $(BIN)/console.o $(BIN)/main.o

else

EXE = $(BIN)/ev
OBJS = $(BIN)/editor.o $(BIN)/foundation.o $(BIN)/file.o $(BIN)/application.o \
	$(BIN)/input.o $(BIN)/console.o $(BIN)/graphics.o $(BIN)/main.o

endif

build: $(BIN) $(EXE)

$(BIN):
	mkdir -p $(BIN)

rebuild: clean build

clean:
	-rm -rf $(BIN) *.log

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(LINKER_FLAGS)

$(BIN)/%.o: %.cpp
	$(CXX) -c -std=gnu++14 -Wall -I. -MMD -Wno-unused-variable -o $@ $< $(COMPILER_FLAGS)

-include $(BIN)/*.d
