all: simulador monitor

simulador: simulador.o
	gcc -lpthread -g simulador.o -o simulador

simulador.o: simulador.c util.h
	gcc -g -c simulador.c

monitor: monitor.o
	gcc -lpthread -g monitor.o -o monitor

monitor.o: monitor.c util.h
	gcc -g -c monitor.c

clean:
	rm -f *.o simulador monitor
