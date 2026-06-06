#pragma once
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
// Parámetros de cultivos representativos de la costa peruana (FAO-56)
// ─────────────────────────────────────────────────────────────────────────────
struct CropParams {
    std::string name;
    double hmin;      // Humedad mínima requerida [%]
    double hmax;      // Humedad máxima tolerable [%]
    double kc_ini;    // Coeficiente cultivo inicial
    double kc_mid;    // Coeficiente cultivo medio
    double kc_end;    // Coeficiente cultivo final
    double ai;        // Eficiencia de riego (goteo=0.92, aspersión=0.80, gravedad=0.55)
    double alpha;     // Coeficiente de productividad
    double beta;      // Costo hídrico
};

inline const CropParams CROPS[] = {
    {"Espárrago",  40.0, 80.0, 0.50, 0.95, 0.30, 0.92, 1.20, 0.35},
    {"Palto",      50.0, 85.0, 0.60, 1.00, 0.85, 0.90, 1.15, 0.40},
    {"Maíz",       45.0, 82.0, 0.30, 1.20, 0.60, 0.80, 1.00, 0.30},
    {"Papa",       55.0, 88.0, 0.50, 1.15, 0.75, 0.85, 1.10, 0.38},
};

// ─────────────────────────────────────────────────────────────────────────────
// Zona de Irrigación
// ─────────────────────────────────────────────────────────────────────────────
struct IrrigationZone {
    int    id;
    std::string name;
    int    crop_index;       // índice en CROPS[]
    double humidity;         // Hi(t) actual [%]
    double area_m2;          // área [m²]
    double et0;              // ET₀ calculada [mm/día]
    double assigned_volume;  // volumen asignado en este ciclo [m³]
    double water_saved;      // agua ahorrada vs riego tradicional [m³]
    bool   active;

    const CropParams& crop() const { return CROPS[crop_index]; }

    // Índice de urgencia hídrica [0,1]
    double urgency() const {
        if (humidity >= crop().hmin) return 0.0;
        return (crop().hmin - humidity) / crop().hmin;
    }

    // Volumen de agua necesario [m³]
    double waterNeed() const {
        double deficit = std::max(0.0, crop().hmin - humidity);
        double et_need = et0 * crop().kc_mid * area_m2 / 1000.0;
        return (deficit / 100.0 * area_m2 * 0.3) / crop().ai + et_need;
    }

    // Beneficio agronómico de regar esta zona
    double benefit() const {
        return crop().alpha * urgency() * area_m2 / 1000.0;
    }

    // Simular un paso de tiempo: actualizar humedad
    void stepTime(double volume_applied, double precipitation, double dt_hours = 1.0) {
        double et_loss = et0 * crop().kc_mid * dt_hours / 24.0;
        double water_gain = (volume_applied * crop().ai * 1000.0 / area_m2)
                          + precipitation;
        humidity += water_gain * 0.1 - et_loss * 0.3;
        humidity  = std::clamp(humidity, 0.0, 100.0);
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// Estado climático
// ─────────────────────────────────────────────────────────────────────────────
struct ClimateData {
    double temperature;    // °C
    double humidity_air;   // % HR
    double wind_speed;     // m/s
    double solar_rad;      // MJ/m²/día
    double precipitation;  // mm

    // Penman-Monteith simplificado (FAO-56) → ET₀ [mm/día]
    double computeET0() const {
        double T = temperature;
        double u2 = wind_speed;
        double Rs = solar_rad;
        double RH = humidity_air / 100.0;

        double es = 0.6108 * std::exp(17.27 * T / (T + 237.3));
        double ea = RH * es;
        double delta = 4098.0 * es / std::pow(T + 237.3, 2.0);
        double gamma = 0.0665;
        double Rn = 0.77 * Rs - 2.1;  // aproximación neta

        double et0 = (0.408 * delta * Rn + gamma * (900.0 / (T + 273.0)) * u2 * (es - ea))
                   / (delta + gamma * (1.0 + 0.34 * u2));
        return std::max(0.0, et0);
    }
};