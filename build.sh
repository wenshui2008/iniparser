gcc -c -O2 -o iniparser.o iniparser.c
gcc -c -O2 -o main.o main.c
gcc -o test main.o iniparser.o

