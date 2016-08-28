all:
	g++ -std=c++14 src/json_parser.cc src/json_parser_test.cc -lgtest -lboost_system-mt -lboost_filesystem-mt -o json_parser_test
