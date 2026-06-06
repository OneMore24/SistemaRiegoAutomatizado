// tests/test_medium.cpp — Casos medianos (n=10³)
// Entregable: Semana 12 · AyDA 2026-I
#include <cassert>
#include <cmath>
#include <iostream>
#include <numeric>
#include "../src/models/irrigation_zone.h"
#include "../src/algorithms/greedy_irrigation.h"
#include "../src/algorithms/dp_irrigation.h"
#include "../src/sensors/sensor_simulation.h"

void ASSERT(bool cond, const std::string& msg) {
    if (!cond) { std::cerr << "[FAIL] " << msg << "\n"; std::exit(1); }
    std::cout << "[PASS] " << msg << "\n";
}

void test_medium_greedy_constraint() {
    int n = 1000;
    auto zones = generateZones(n, 10);
    double W = 5000.0;
    auto result = GreedyIrrigation::run(zones, W);
    double total = std::accumulate(result.assignments.begin(), result.assignments.end(), 0.0);
    ASSERT(total <= W + 1e-6, "n=1000: Greedy no excede W");
    ASSERT(result.exec_time_us < 50000, "n=1000: Greedy < 50ms");
}

void test_medium_dp_constraint() {
    int n = 100;   // DP factible para n<=200
    auto zones = generateZones(n, 11);
    double W = 500.0;
    auto result = DPIrrigation::run(zones, W, 2.0, 500);
    ASSERT(result.total_benefit >= 0.0, "n=100: DP beneficio >= 0");
    ASSERT(result.exec_time_us < 500000, "n=100: DP < 500ms");
}

void test_medium_non_negativity_all() {
    auto zones = generateZones(1000, 12);
    auto result = GreedyIrrigation::run(zones, 3000.0);
    bool all_non_neg = true;
    for (double v : result.assignments) if (v < 0) { all_non_neg = false; break; }
    ASSERT(all_non_neg, "n=1000: todas las asignaciones >= 0");
}

void test_medium_ema_stability() {
    int n = 500;
    auto zones = generateZones(n, 13);
    SensorEMAFilter f(n, 0.3);
    // Ejecutar 10 ciclos — verificar estabilidad
    std::vector<double> prev = f.read(zones);
    for (int i = 1; i < 10; ++i) {
        auto curr = f.read(zones);
        double max_jump = 0;
        for (int j = 0; j < n; ++j)
            max_jump = std::max(max_jump, std::abs(curr[j] - prev[j]));
        ASSERT(max_jump < 20.0, "EMA estable: salto < 20% entre ciclos");
        prev = curr;
    }
}

void test_medium_complexity_ratio() {
    // Verifica que ratio T(n)/n*log2(n) es aproximadamente constante
    std::vector<int> sizes = {100, 200, 500, 1000};
    std::vector<double> ratios;
    for (int n : sizes) {
        auto zones = generateZones(n, 14);
        auto result = GreedyIrrigation::run(zones, n * 20.0);
        double ratio = (double)result.exec_time_us /
                       (n * std::log2((double)n));
        ratios.push_back(ratio);
    }
    double mean = std::accumulate(ratios.begin(), ratios.end(), 0.0) / ratios.size();
    for (double r : ratios)
        ASSERT(std::abs(r - mean) / mean < 0.8,
               "Ratio T/(n·log₂n) aprox. constante (±80% de la media)");
}

int main() {
    std::cout << "\n=== TEST MEDIUM (n=10³) ===\n\n";
    test_medium_greedy_constraint();
    test_medium_dp_constraint();
    test_medium_non_negativity_all();
    test_medium_ema_stability();
    test_medium_complexity_ratio();
    std::cout << "\n✓ Todos los tests medianos pasaron.\n\n";
    return 0;
}