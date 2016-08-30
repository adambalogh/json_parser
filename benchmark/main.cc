#include <fstream>
#include <string>

#include "benchmark/benchmark.h"

#include "../src/json_parser.h"
#include "nlohmann/json.hpp"
#include "rapidjson/document.h"

static std::ifstream file("test_data/benchmark/twitter.json");
static const std::string e((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());

static void jpParse(benchmark::State& state) {
  while (state.KeepRunning()) {
    jp::JsonParser{e}.Parse();
  }
}

static void nlohmannParse(benchmark::State& state) {
  while (state.KeepRunning()) {
    nlohmann::json::parse(e);
  }
}

static void rapidjsonParse(benchmark::State& state) {
  while (state.KeepRunning()) {
    rapidjson::Document d;
    d.Parse(e.c_str());
  }
}

BENCHMARK(jpParse);
BENCHMARK(nlohmannParse);
BENCHMARK(rapidjsonParse);

BENCHMARK_MAIN();
