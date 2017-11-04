
cc = g++ -Wall -Werror -std=c++03 -I..

targets = clean check

all: ${targets}

clean:
	rm -f *.o main

check: checkduplicatesymbols1.cpp checkduplicatesymbols2.cpp main.cpp
	${cc} checkduplicatesymbols1.cpp checkduplicatesymbols2.cpp main.cpp -o main
