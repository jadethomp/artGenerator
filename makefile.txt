artGenerator: main.o binary_sem.o
	gcc main.o binary_sem.o -o artGenerator

main.o: main.c
	gcc -c main.c

binary_sem.o: binary_sem.c binary_sem.h
	gcc -c binary_sem.c

clean:
	rm artGenerator