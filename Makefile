rsi: main.c
	clang -Wall main.c -lreadline -o rsi

.PHONY: clean
clean:
	rm rsi
