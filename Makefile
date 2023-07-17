build:
	gcc *.c -o game -lSDL2 -lm -lSDL2_image

run:
	./game

clean:
	rm -rf game
