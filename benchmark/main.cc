#include <fstream>
#include <string>

#include "benchmark/benchmark.h"

#include "../src/json_parser.h"
#include "nlohmann/json.hpp"
#include "cpprest/json.h"
#include "json/json.h"

/* Results:
 *
 * Run on (8 X 1000 MHz CPU s)
 * 2016-09-07 23:38:11
 * Benchmark       Time(ns)    CPU(ns) Iterations
 * ----------------------------------------------
 * jpParse          8256589    8199881         84
 * nlohmannParse   15699699   15655044         45
 * cppRestParse     7834885    7831289         83
 * jsonCppParse    13401031   13382741         54
 *
 */

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

static void microsoftCppRestParse(benchmark::State& state) {
  while (state.KeepRunning()) {
    web::json::value::parse(e);
  }
}

static void jsonCppParse(benchmark::State& state) {
  Json::Reader reader;
  Json::Value value;
  while (state.KeepRunning()) {
    reader.parse(e, value, false);
  }
}

BENCHMARK(jpParse);
BENCHMARK(nlohmannParse);
BENCHMARK(microsoftCppRestParse);
BENCHMARK(jsonCppParse);

BENCHMARK_MAIN();
