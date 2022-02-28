.PHONY: all

dev: all
	embeddedLang

all: main embeddedLang
	gcc -o embeddedLang build/main build/embeddedLang

main:
	gcc -c -o build/main src/main.c

embeddedLang:
	gcc -c -o build/embeddedLang src/embeddedLang.c