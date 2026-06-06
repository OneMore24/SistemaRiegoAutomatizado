// tests/test_benchmark.cpp — Benchmarks de escalabilidad para el informe
// Entregable: Semana 14/15 · AyDA 2026-I
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include "../src/models/irrigation_zone.h"
#include "../src/algorithms/greedy_irrigation.h"
#include "../src/algorithms/dp_irrigation.h"

int main() {
    std::cout << "\n=== BENCHMARKS DE ESCALABILIDAD ===\n\n";
    std::cout << std::left
              << std::setw(12) << "n"
              << std::setw(16) << "Greedy (μs)"
              << std::setw(20) << "T/(n·log₂n)"
              << std::setw(16) << "DP (μs)"
              << "\n";
    std::cout << std::string(64, '-') << "\n";

    std::vector<int> sizes = {10,50,100,500,1000,5000,10000,100000,1000000};

    for (int n : sizes) {
        auto zones_g = generateZones(n, 77);
        auto g = GreedyIrrigation::run(zones_g, (double)n * 20.0);
        double ratio = (double)g.exec_time_us / (n * std::log2((double)n));

        std::string dp_str;
        if (n <= 1000) {
            auto zones_d = generateZones(n, 77);
            auto d = DPIrrigation::run(zones_d,
                std::min((double)n*20.0, 1000.0), 2.0, 500);
            dp_str = std::to_string(d.exec_time_us);
        } else {
            dp_str = "N/A (mem)";
        }

        std::cout << std::left
                  << std::setw(12) << n
                  << std::setw(16) << g.exec_time_us
                  << std::setw(20) << std::fixed << std::setprecision(5) << ratio
                  << std::setw(16) << dp_str
                  << "\n";
    }

    std::cout << "\nNota: ratio T/(n·log₂n) ≈ constante confirma O(n log n)\n\n";
    return 0;
}