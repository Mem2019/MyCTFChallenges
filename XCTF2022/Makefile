all:
	python3 compiler.py > expr.h
	gcc -no-pie -fno-stack-protector -O0 -Wall -g main.c -o lambda
	rm expr.h