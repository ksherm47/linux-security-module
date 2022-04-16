LIB_DIR := $(shell pwd)/lib/
SRC_DIR := $(shell pwd)/src/
INCLUDES_DIR := $(shell pwd)/include/

build:
	if [ ! -d $(LIB_DIR) ]; then \
	  mkdir -p $(LIB_DIR); \
	fi
	make LIB_DIR=$(LIB_DIR) INCLUDES_DIR=$(INCLUDES_DIR) -C $(SRC_DIR)

clean:
	make LIB_DIR=$(LIB_DIR) -C $(SRC_DIR) clean

all: build
