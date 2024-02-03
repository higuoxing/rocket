rsi: main.c vector.c
	clang -Wall main.c vector.c -lreadline -o rsi

.PHONY: clean
clean:
	rm rsi
