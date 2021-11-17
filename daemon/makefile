FRAMEWORKS     = -framework Cocoa -framework Carbon -framework CoreServices
BUILD_PATH     = ./bin
DATA_PATH      = ./data
BUILD_FLAGS    = -std=c++17 -Wall -g -O0
SRCS           = main.cpp time_utils.cpp string_utils.cpp log_utils.cpp keymapping.cpp
BINS           = $(BUILD_PATH)/main
LOCAL_BIN      = ~/bin/keylogger/

.PHONY: all clean install

all: clean $(BINS)

install: BUILD_FLAGS=-std=c++17 -Wall -O2
install: clean $(BINS)

clean:
	rm -rf $(BUILD_PATH)
	rm -rf $(DATA_PATH)

$(BUILD_PATH)/main: $(SRCS)
	mkdir -p $(BUILD_PATH)
	clang++ $^ $(BUILD_FLAGS) $(FRAMEWORKS) -o $@

run:
	$(BUILD_PATH)/main -o var/

deploy: $(BUILD_PATH)/main
	# Need to export KEYLOGGER_PLIST first!
	launchctl unload -w ${KEYLOGGER_PLIST}
	cp $(BUILD_PATH)/main $(LOCAL_BIN)
	launchctl load -w ${KEYLOGGER_PLIST}
