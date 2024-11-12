CC = clang
SDL = -lsdl2
EXEC_NAME = ch8
TEST_NAME = test-chip8-op

.PHONY: all test clean

all:
	${CC} src/main.c ${SDL} -o ${EXEC_NAME}

test:
	${CC} test/test-chip8-op.c -o ${TEST_NAME}

clean:
	rm -f ${EXEC_NAME}
	rm -f ${TEST_NAME}