#include <fstream>
#include <string>

#include "benchmark/benchmark.h"

#include "../src/json_parser.h"
#include "nlohmann/json.hpp"
#include "cpprest/json.h"
#include "json/json.h"
#include "rapidjson/document.h"

/* Results:
 *
 * Run on (8 X 1000 MHz CPU s)
 * 2016-09-07 23:49:36
 * Benchmark               Time(ns)    CPU(ns) Iterations
 * ------------------------------------------------------
 * jpParse                 15508647   15504319         47
 * nlohmannParse           29532919   29508542         24
 * rapidJsonParse           3366416    3362638        207
 * microsoftCppRestParse   22255828   22233613         31
 * jsonCppParse            26419685   26394731         26
 *
 */

// file contains lot of numbers
static std::ifstream file("test_data/benchmark/citm_catalog.json");
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

static void rapidJsonParse(benchmark::State& state) {
  while (state.KeepRunning()) {
    rapidjson::Document doc;
    doc.Parse(e.c_str());
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
BENCHMARK(rapidJsonParse);
BENCHMARK(microsoftCppRestParse);
BENCHMARK(jsonCppParse);

BENCHMARK_MAIN();
