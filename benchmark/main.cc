#include <fstream>
#include <string>

#include "benchmark/benchmark.h"

#include "../src/json_parser.h"
#include "nlohmann/json.hpp"

static std::ifstream file("test_data/json.org/1.json");
static const std::string e((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());

static void jpParse(benchmark::State& state) {
  while (state.KeepRunning()) {
    auto val = jp::JsonParser{e}.Parse();
  }
}

static void nlohmannParse(benchmark::State& state) {
  while (state.KeepRunning()) {
    auto val = nlohmann::json::parse(e);
  }
}

BENCHMARK(jpParse);
BENCHMARK(nlohmannParse);

BENCHMARK_MAIN();
