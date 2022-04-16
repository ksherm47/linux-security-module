CC := gcc

ALL_SRC := $(wildcard *.c)
ALL_OBJECTS := $(ALL_SRC:.c=.o)
INCLUDES := $(INCLUDES_DIR)
LIB_NAME := $(LIB_DIR)libkenlex.so

%.o: %.c
	$(CC) -c -I $(INCLUDES) -o $@ $<

$(LIB_NAME): $(ALL_OBJECTS)
	$(CC) -shared $(ALL_OBJECTS) -o $(LIB_NAME)

clean:
	rm -f $(ALL_OBJECTS) $(LIB_NAME)

all: $(LIB_NAME)


