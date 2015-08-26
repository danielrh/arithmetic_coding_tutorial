CC=gcc
CFLAGS=-I.
DEPS = bitreader.h bitwriter.h test_harness.h dyn_prob.h
OBJ = bitreader.o bitwriter.o test_harness.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: lesson0 lesson1 lesson2 lesson3 lesson4 lesson5
lesson0: $(OBJ) lesson0.o
	gcc -o $@ $^ $(CFLAGS)

lesson1: $(OBJ) lesson1.o
	gcc -o $@ $^ $(CFLAGS)

lesson2: $(OBJ) lesson2.o
	gcc -o $@ $^ $(CFLAGS)

lesson3: $(OBJ) lesson3.o
	gcc -o $@ $^ $(CFLAGS)

lesson4: $(OBJ) lesson4.o
	gcc -o $@ $^ $(CFLAGS)

lesson5: $(OBJ) lesson5.o
	gcc -o $@ $^ $(CFLAGS)
