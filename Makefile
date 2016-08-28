all:
	g++ -std=c++14 src/json_parser.cc src/json_parser_test.cc -lgtest -lboost_system-mt -lboost_filesystem-mt -o json_parser_test -Wall -Werror

benchmark_main: benchmark/main.cc src/json_parser.cc src/json_parser.h
	g++ -std=c++14 -O3 -DNDEBUG benchmark/main.cc src/json_parser.cc -lbenchmark -o benchmark_main
