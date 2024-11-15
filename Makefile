CC = clang
CFLAGS = -std=c99 -Wall -Wextra -O1
SDL = -lSDL2
EXEC_NAME = ch8
TEST_NAME = test-chip8-op
EXEC_INCLUDE = -Iinclude src/chip8.c src/peripheral.c
TEST_INCLUDE = -Iinclude

.PHONY: all test clean

all:
	${CC} src/main.c ${EXEC_INCLUDE} ${SDL} ${CFLAGS} -o ${EXEC_NAME}

test:
	${CC} test/test-chip8-op.c ${TEST_INCLUDE} -o ${TEST_NAME}

clean:
	rm -f ${EXEC_NAME}
	rm -f ${TEST_NAME}