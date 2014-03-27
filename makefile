CC = gcc
EXEC = wave

${EXEC}: wave.c wave.h
	${CC} -Wall wave.c -o ${EXEC}

clean:
	rm -f ${EXEC}
