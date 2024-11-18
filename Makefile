CC = clang
CFLAGS = -std=c99 -Wall -Wextra -O1
SDL = -lSDL2
EXEC_NAME = ch8
CHIP8_TEST_NAME = test-chip8-op
SCHIP_TEST_NAME = test-schip-op
EXEC_SOURCES = src/main.c src/chip8.c src/peripheral.c
CHIP8_TEST_SOURCES = test/test-chip8-op.c
SCHIP_TEST_SOURCES = test/test-schip-op.c
INCLUDE = -Iinclude

.PHONY: all debug test clean

all:
	${CC} ${EXEC_SOURCES} ${INCLUDE} ${SDL} ${CFLAGS} -o ${EXEC_NAME}
	
debug:
	${CC} -D DEBUG ${EXEC_SOURCES} ${INCLUDE} ${SDL} ${CFLAGS} -o ${EXEC_NAME}

test:
	${CC} ${CHIP8_TEST_SOURCES} ${INCLUDE} -o ${CHIP8_TEST_NAME}
	${CC} ${SCHIP_TEST_SOURCES} ${INCLUDE} -o ${SCHIP_TEST_NAME}

clean:
	rm -f ${EXEC_NAME}
	rm -f ${CHIP8_TEST_NAME}
	rm -f ${SCHIP_TEST_NAME}
	rm -f rpl-flags.bin