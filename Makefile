all:
	g++ -std=c++14 src/json_parser.cc src/json_parser_test.cc -lgtest -o json_parser_test
