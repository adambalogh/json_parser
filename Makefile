all:
	g++ -std=c++14 json_parser.cc json_parser_test.cc -lgtest -o json_parser_test
