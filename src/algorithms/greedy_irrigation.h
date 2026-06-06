#pragma once
#include "../models/irrigation_zone.h"
#include <vector>
#include <algorithm>
#include <chrono>
#include <numeric>

// ─────────────────────────────────────────────────────────────────────────────
// ALGORITMO GREEDY — O(n log n)
// Ordena por urgencia hídrica y asigna agua de forma voraz
// ─────────────────────────────────────────────────────────────────────────────
struct GreedyResult {
    std::vector<double> assignments;   // Vi por zona
    double total_assigned;
    double total_benefit;
    double water_efficiency;           // % agua útil vs total disponible
    long long exec_time_us;            // microsegundos
    std::string complexity_note;
};

class GreedyIrrigation {
public:
    static GreedyResult run(std::vector<IrrigationZone>& zones, double W,
                            double cmax_per_valve = 50.0) {
        auto t_start = std::chrono::high_resolution_clock::now();

        int n = static_cast<int>(zones.size());
        std::vector<double> assignment(n, 0.0);

        // ── PASO 1: Calcular urgencias y crear índices ── O(n)
        std::vector<int> idx(n);
        std::iota(idx.begin(), idx.end(), 0);

        // ── PASO 2: Ordenar por urgencia DESC ── O(n log n)  ← dominante
        std::sort(idx.begin(), idx.end(), [&](int a, int b) {
            return zones[a].urgency() > zones[b].urgency();
        });

        // ── PASO 3: Asignación voraz ── O(n)
        double remaining = W;
        double total_benefit = 0.0;

        for (int rank = 0; rank < n && remaining > 1e-9; ++rank) {
            int i = idx[rank];
            if (!zones[i].active) continue;

            double need = zones[i].waterNeed();
            double vol  = std::min({need, remaining, cmax_per_valve});
            vol = std::max(vol, 0.0);

            assignment[i]    = vol;
            remaining       -= vol;
            total_benefit   += zones[i].benefit() * (vol / std::max(need, 1e-9));
        }

        auto t_end = std::chrono::high_resolution_clock::now();
        long long us = std::chrono::duration_cast<std::chrono::microseconds>(
                           t_end - t_start).count();

        double assigned = W - remaining;
        return {
            assignment,
            assigned,
            total_benefit,
            assigned / std::max(W, 1.0) * 100.0,
            us,
            "O(n log n) — dominado por ordenamiento"
        };
    }
};