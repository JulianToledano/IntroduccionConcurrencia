OBJS = cuda_multiplicacion.o
EXE = multiplicar
GCC = nvcc
INCLUDES = -I.
CFLAGS = -g -Wno-deprecated-gpu-targets
CXXFLAGS =
all: $(OBJS)
	$(GCC) $(OBJS) $(LDFLAGS) $(CFLAGS) $(CXXFLAGS) -o $(EXE)

%.o: %.cu
	$(GCC) -c $< $(CFLAGS) -o $@

clean:
	rm *.o $(EXE)
