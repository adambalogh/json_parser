all: test benchmark_main

test: src/json_parser_test.cc src/json_parser.cc src/json_parser.h src/json_value.h
	clang++ -std=c++14 src/json_parser.cc src/json_parser_test.cc -lgtest -lboost_system-mt -lboost_filesystem-mt -o json_parser_test -Wall -Werror

benchmark_main: benchmark/main.cc src/json_parser.cc src/json_parser.h src/json_value.h
	clang++ -std=c++14 -O3 -DNDEBUG benchmark/main.cc src/json_parser.cc -lbenchmark -lboost_system-mt -lboost_thread-mt -lboost_chrono-mt -lboost_date_time-mt -lcpprest -ljsoncpp -o benchmark_main

clean:
	rm benchmark_main json_parser_test
