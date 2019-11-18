HEADERS = functions.h macros.h
OBJECTS = main.o functions.o

default: download

%.o: %.c $(HEADERS)
	gcc -c $< -o $@

download: $(OBJECTS)
	gcc $(OBJECTS) -o $@

clean:
	-rm -f $(OBJECTS)
	-rm -f download
