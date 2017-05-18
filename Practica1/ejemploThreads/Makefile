OBJS = main.o threads.o
EXE = ejemploThreads
GCC = g++
INCLUDES = -I.
CFLAGS = -g -pg -pthread
CXXFLAGS =
all: $(OBJS)
	$(GCC) $(OBJS) $(LDFLAGS) $(CFLAGS) $(CXXFLAGS) -o $(EXE)

%.o: %.cpp
	$(GCC) -c $< $(CFLAGS) -o $@

clean:
	rm *.o $(EXE)
