CC = clang
SDL = -lsdl2
EXEC_NAME = ch8

all:
	${CC} src/main.c ${SDL} -o ${EXEC_NAME}

clean:
	rm -r ${EXEC_NAME}
