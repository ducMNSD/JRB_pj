default: run

main:
	clear 
	gcc -w -o Try Try.c libbt.a libfdr.a  `pkg-config gtk+-3.0 --cflags --libs`

run: main
	./Try 
clean:
	rm -f main
