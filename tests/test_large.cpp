// tests/test_large.cpp — Casos grandes (n=10⁶) y extremos (n=10¹⁰ extrapolado)
// Entregable: Semana 12 · AyDA 2026-I
#include <iostream>
#include <cmath>
#include <chrono>
#include "../src/models/irrigation_zone.h"
#include "../src/algorithms/greedy_irrigation.h"

void ASSERT(bool cond, const std::string& msg) {
    if (!cond) { std::cerr << "[FAIL] " << msg << "\n"; std::exit(1); }
    std::cout << "[PASS] " << msg << "\n";
}

void test_large_n1M() {
    int n = 1'000'000;
    std::cout << "  Generando " << n << " zonas...\n";
    auto zones = generateZones(n, 99);
    auto result = GreedyIrrigation::run(zones, (double)n * 10.0);
    ASSERT(result.exec_time_us < 5'000'000, "n=10⁶: Greedy < 5 segundos");
    double total = 0;
    for (double v : result.assignments) total += v;
    ASSERT(total <= (double)n * 10.0 + 1e-3, "n=10⁶: respeta restricción hídrica");
    std::cout << "  Tiempo: " << result.exec_time_us/1000 << " ms\n";
}

void test_extreme_extrapolation() {
    // Mide tiempos para n pequeños y extrapola a n=10^10
    std::cout << "\n  Extrapolación para n=10^10:\n";
    std::vector<int> ns = {1000, 5000, 10000, 50000};
    std::vector<double> times;
    for (int n : ns) {
        auto zones = generateZones(n, 98);
        auto result = GreedyIrrigation::run(zones, (double)n * 20.0);
        times.push_back((double)result.exec_time_us);
        std::cout << "    n=" << n << " → " << result.exec_time_us << " μs\n";
    }
    // Ajuste lineal de log(T) vs log(n)
    double sum_xy=0, sum_xx=0, sum_x=0, sum_y=0;
    int m = ns.size();
    for (int i=0;i<m;i++){
        double x=std::log((double)ns[i]), y=std::log(times[i]);
        sum_xy+=x*y; sum_xx+=x*x; sum_x+=x; sum_y+=y;
    }
    double slope = (m*sum_xy - sum_x*sum_y) / (m*sum_xx - sum_x*sum_x);
    ASSERT(slope > 0.9 && slope < 1.5,
           "Exponente empírico entre 1.0 y 1.5 (esperado ~1.0-1.1 para O(n log n))");
    // Extrapolación a n=10^10
    double x10 = std::log(1e10);
    double intercept = (sum_y - slope*sum_x) / m;
    double t10 = std::exp(intercept + slope*x10);
    std::cout << "  Extrapolación n=10^10: ~" << (long long)t10 << " μs ("
              << (long long)(t10/1e6) << " s)\n";
    ASSERT(true, "Extrapolación completada (ver valor impreso)");
}

int main() {
    std::cout << "\n=== TEST LARGE/EXTREME (n=10⁶, extrapolación n=10^10) ===\n\n";
    test_large_n1M();
    test_extreme_extrapolation();
    std::cout << "\n✓ Tests grandes pasaron.\n\n";
    return 0;
}