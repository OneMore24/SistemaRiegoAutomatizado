#pragma once
#include "../models/irrigation_zone.h"
#include <vector>
#include <algorithm>
#include <chrono>
#include <numeric>
#include <stdexcept>

// ─────────────────────────────────────────────────────────────────────────────
// ALGORITMO DP — O(n * C)  con rolling array → O(C) espacio
// Variante del Knapsack 0/1 para asignación óptima de agua
// ─────────────────────────────────────────────────────────────────────────────
struct DPResult {
    std::vector<double> assignments;
    double total_assigned;
    double total_benefit;
    double water_efficiency;
    long long exec_time_us;
    int capacity_units;       // C = W/delta
    std::string complexity_note;
};

class DPIrrigation {
public:
    // delta: resolución de discretización [m³] (ej. 0.5 m³ por unidad)
    static DPResult run(std::vector<IrrigationZone>& zones, double W,
                        double delta = 1.0, int max_capacity = 2000) {
        auto t_start = std::chrono::high_resolution_clock::now();

        int n = static_cast<int>(zones.size());
        int C = std::min(static_cast<int>(W / delta), max_capacity);

        // ── Rolling array: solo 2 filas de tamaño C+1 ── O(C) espacio
        std::vector<double> prev(C + 1, 0.0);
        std::vector<double> curr(C + 1, 0.0);

        // Pesos y valores discretizados
        std::vector<int>    weights(n);
        std::vector<double> values(n);

        for (int i = 0; i < n; ++i) {
            if (!zones[i].active) { weights[i] = C + 1; values[i] = 0.0; continue; }
            double need  = zones[i].waterNeed();
            weights[i]   = std::max(1, static_cast<int>(std::ceil(need / delta)));
            values[i]    = zones[i].benefit();
        }

        // ── Llenado tabla DP ── O(n * C)
        for (int i = 0; i < n; ++i) {
            for (int w = 0; w <= C; ++w) {
                curr[w] = prev[w];
                if (w >= weights[i]) {
                    curr[w] = std::max(curr[w], prev[w - weights[i]] + values[i]);
                }
            }
            std::swap(prev, curr);
            std::fill(curr.begin(), curr.end(), 0.0);
        }

        // ── Reconstrucción O(n + C) ──
        // Necesitamos reconstruir, así que corremos de nuevo guardando la tabla completa
        // (solo para reconstrucción, limitado a n <= 200)
        std::vector<double> assignments(n, 0.0);
        double total_assigned = 0.0;
        double total_benefit  = 0.0;

        if (n <= 200) {
            std::vector<std::vector<double>> dp(n + 1, std::vector<double>(C + 1, 0.0));
            for (int i = 0; i < n; ++i) {
                for (int w = 0; w <= C; ++w) {
                    dp[i+1][w] = dp[i][w];
                    if (w >= weights[i]) {
                        dp[i+1][w] = std::max(dp[i+1][w], dp[i][w - weights[i]] + values[i]);
                    }
                }
            }
            // Reconstruir
            int w = C;
            for (int i = n - 1; i >= 0; --i) {
                if (dp[i+1][w] != dp[i][w]) {
                    assignments[i]   = weights[i] * delta;
                    total_assigned  += assignments[i];
                    total_benefit   += values[i];
                    w -= weights[i];
                }
            }
        }

        auto t_end = std::chrono::high_resolution_clock::now();
        long long us = std::chrono::duration_cast<std::chrono::microseconds>(
                           t_end - t_start).count();

        return {
            assignments,
            total_assigned,
            total_benefit,
            total_assigned / std::max(W, 1.0) * 100.0,
            us,
            C,
            "O(n·C) tiempo, O(C) espacio (rolling array)"
        };
    }
};