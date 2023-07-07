build:
	gcc test.c -o game -lSDL2 -lm

run:
	./game

clean:
	rm -rf game
