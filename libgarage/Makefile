all: $(BUILD_DIR)/libgarage.a

$(BUILD_DIR)/%.o: %.cpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CCFLAGS) -c $< -o $@

LIBGARAGE_OBJS = $(BUILD_DIR)/main.o \
		 $(BUILD_DIR)/memory.o \
		 $(BUILD_DIR)/jack.o \
		 $(BUILD_DIR)/portaudio.o \
		 $(BUILD_DIR)/sndfile.o
$(BUILD_DIR)/libgarage.a: $(LIBGARAGE_OBJS)
	$(AR) $(ARFLAGS) $@ $(LIBGARAGE_OBJS)

clean:
	rm -rf build

rebuild: clean all

