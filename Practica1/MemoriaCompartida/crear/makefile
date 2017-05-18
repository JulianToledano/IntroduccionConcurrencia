EXEC=crearMatriz
OBJ= crearMatrix.o

#compilador
CC=gcc
#flags de compilacion
CFLAGS=
#directorios de inclusion con -I
INCLUDE=-I./
#linkado
LDFLAGS=-lpthread


all: $(OBJ)
	$(CC) $(OBJ) $(CFLAGS) $(LDFLAGS) -o $(EXEC)


%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@

clean:
	rm $(OBJ)
	rm $(EXEC)
