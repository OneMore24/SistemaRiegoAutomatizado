#pragma once
#include "../models/irrigation_zone.h"
#include <vector>
#include <random>
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
// Filtro EMA para señales de sensores — O(n) por ciclo
// Hᵢ_filtrado(t) = λ·Hᵢ_sensor(t) + (1−λ)·Hᵢ_filtrado(t−1)
// ─────────────────────────────────────────────────────────────────────────────
class SensorEMAFilter {
    double lambda_;
    std::vector<double> filtered_;
    std::mt19937 rng_{42};
    std::normal_distribution<double> noise_{0.0, 2.5}; // ruido ±2.5%

public:
    explicit SensorEMAFilter(int n, double lambda = 0.3)
        : lambda_(lambda), filtered_(n, 50.0) {}

    // Leer y filtrar — O(n)
    std::vector<double> read(const std::vector<IrrigationZone>& zones) {
        std::vector<double> out(zones.size());
        for (int i = 0; i < static_cast<int>(zones.size()); ++i) {
            double raw = zones[i].humidity + noise_(rng_);
            raw = std::clamp(raw, 0.0, 100.0);
            filtered_[i] = lambda_ * raw + (1.0 - lambda_) * filtered_[i];
            out[i] = filtered_[i];
        }
        return out;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// Simulador de Clima (datos generativos para demo)
// ─────────────────────────────────────────────────────────────────────────────
class ClimateSimulator {
    int hour_ = 0;
    std::mt19937 rng_{7};
    std::uniform_real_distribution<double> rain_chance_{0.0, 1.0};

public:
    ClimateData next() {
        double t = hour_ % 24;
        // Temperatura: perfil diario sinusoidal (Ica, Perú)
        double temp = 22.0 + 8.0 * std::sin((t - 6.0) * M_PI / 12.0);
        double wind = 2.0 + 1.5 * std::sin(t * M_PI / 8.0);
        double solar = std::max(0.0, 20.0 * std::sin((t - 6.0) * M_PI / 12.0));
        double rh   = 65.0 - 15.0 * std::sin((t - 6.0) * M_PI / 12.0);
        double rain = (rain_chance_(rng_) < 0.05) ? (5.0 + rain_chance_(rng_) * 10.0) : 0.0;

        ++hour_;
        return { temp, rh, wind, solar, rain };
    }

    int hour() const { return hour_; }
};

// ─────────────────────────────────────────────────────────────────────────────
// Generador de zonas de cultivo para pruebas
// ─────────────────────────────────────────────────────────────────────────────
inline std::vector<IrrigationZone> generateZones(int n, unsigned seed = 42) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> crop_dist(0, 3);
    std::uniform_real_distribution<double> hum_dist(25.0, 75.0);
    std::uniform_real_distribution<double> area_dist(500.0, 5000.0);

    std::vector<IrrigationZone> zones;
    zones.reserve(n);

    for (int i = 0; i < n; ++i) {
        IrrigationZone z;
        z.id          = i;
        z.name        = "Zona-" + std::to_string(i + 1);
        z.crop_index  = crop_dist(rng);
        z.humidity    = hum_dist(rng);
        z.area_m2     = area_dist(rng);
        z.et0         = 4.5 + (rng() % 30) * 0.1;
        z.assigned_volume = 0.0;
        z.water_saved = 0.0;
        z.active      = true;
        zones.push_back(z);
    }
    return zones;
}