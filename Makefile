CC = clang
CFLAGS = -std=c99 -Wall -Wextra -O1
SDL = -lSDL2
EXEC_NAME = ch8
TEST_NAME = test-chip8-op
EXEC_SOURCES = src/main.c src/chip8.c src/peripheral.c
TEST_SOURCES = test/test-chip8-op.c
INCLUDE = -Iinclude

.PHONY: all debug test clean

all:
	${CC} ${EXEC_SOURCES} ${INCLUDE} ${SDL} ${CFLAGS} -o ${EXEC_NAME}
	
debug:
	${CC} -D DEBUG ${EXEC_SOURCES} ${INCLUDE} ${SDL} ${CFLAGS} -o ${EXEC_NAME}

test:
	${CC} ${TEST_SOURCES} ${INCLUDE} -o ${TEST_NAME}

clean:
	rm -f ${EXEC_NAME}
	rm -f ${TEST_NAME}