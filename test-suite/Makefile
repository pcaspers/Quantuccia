
cc = g++ -Wall -Werror -std=c++03 -I..

targets = clean test

all: ${targets}

clean:
	rm -f *.o quantlibtestsuite

test: quantlibtestsuite.cpp
	${cc} $< -o quantlibtestsuite
	./quantlibtestsuite --log_level=message

