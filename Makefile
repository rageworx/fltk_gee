# Makefile for fltk-imgtk-gee tech-demo, testing app.
# ----------------------------------------------------------------------
# Written by Raph.K.
#

GCC = gcc
GPP = g++
AR  = ar
FCG = fltk-config --use-images

# FLTK configs 
FLTKCFG_CXX := $(shell ${FCG} --cxxflags)
FLTKCFG_LFG := $(shell ${FCG} --ldflags)

# Base PATH
BASE_PATH = .
SRC_PATH  = $(BASE_PATH)/src

# TARGET settings
TARGET_PKG =  fltk_gee
TARGET_DIR = ./bin/macOS
TARGET_OBJ = ./obj
TARGET_OBJFL = ./obj/fl

# DEFINITIONS
#DEFS += -DUNICODE -D_UNICODE -DSUPPORT_WCHAR

# Compiler optiops 
COPTS += -std=c++11
COPTS += -O3

# Where is fl_imgtk?
FLIMGTK_DIR = ../fl_imgtk/lib

# CC FLAGS
CFLAGS  = -I$(SRC_PATH) -I$(SRC_PATH)/fl
CFLAGS += -Ires
CFLAGS += -I$(FLIMGTK_DIR)
CFLAGS += $(FLTKCFG_CXX)
CFLAGS += $(DEFS)
CFLAGS += $(COPTS)

# Windows Resource Flags
WFLGAS  = $(CFLAGS)

# LINK FLAG
LFLAGS += -L$(FLIMGTK_DIR)
LFLAGS += -lfl_imgtk
LFLAGS += $(FLTKCFG_LFG)

# Sources
SRCS = $(wildcard $(SRC_PATH)/*.cpp)
FLSRCS = $(wildcard $(SRC_PATH)/fl/*.cpp)

# Make object targets from SRCS.
OBJS = $(SRCS:$(SRC_PATH)/%.cpp=$(TARGET_OBJ)/%.o)
FLOBJS = $(FLSRCS:$(SRC_PATH)/fl/%.cpp=$(TARGET_OBJ)/fl/%.o)

.PHONY: prepare clean

all: prepare continue

continue: $(TARGET_DIR)/$(TARGET_PKG)

prepare:
	@mkdir -p $(TARGET_DIR)
	@mkdir -p $(TARGET_OBJ)
	@mkdir -p $(TARGET_OBJ)/fl

clean:
	@echo "Cleaning built targets ..."
	@rm -rf $(TARGET_DIR)/$(TARGET_PKG).*
	@rm -rf $(TARGET_INC)/*.h
	@rm -rf $(TARGET_OBJ)/*.o
	@rm -rf $(FLFTRNDR_OBJ)

$(OBJS): $(TARGET_OBJ)/%.o: $(SRC_PATH)/%.cpp
	@echo "Building $@ ... "
	@$(GPP) $(CFLAGS) -c $< -o $@

$(FLOBJS): $(TARGET_OBJ)/fl/%.o: $(SRC_PATH)/fl/%.cpp
	@echo "Building fl/$@ ... "
	@$(GPP) $(CFLAGS) -c $< -o $@

$(TARGET_DIR)/$(TARGET_PKG): $(OBJS) $(FLOBJS)
	@echo "Generating $@ ..."
	@$(GPP) $^ $(CFLAGS) $(LFLAGS) -o $@
	@echo "done."
