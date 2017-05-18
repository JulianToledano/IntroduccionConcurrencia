OBJS = Division_estatica.o
EXE = multiplicar
GCC = g++
INCLUDES = -I.
CFLAGS = -g -pthread
CXXFLAGS =
all: $(OBJS)
	$(GCC) $(OBJS) $(LDFLAGS) $(CFLAGS) $(CXXFLAGS) -o $(EXE)

%.o: %.cpp
	$(GCC) -c $< $(CFLAGS) -o $@

clean:
	rm *.o $(EXE)
