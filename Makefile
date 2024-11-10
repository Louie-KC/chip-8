CC = clang
EXEC_NAME = ch8

all:
	${CC} src/main.c -o ${EXEC_NAME}

clean:
	rm -r ${EXEC_NAME}
