#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <cmath>
#include <string>
#include <sstream>

#include "src/models/irrigation_zone.h"
#include "src/algorithms/greedy_irrigation.h"
#include "src/algorithms/dp_irrigation.h"
#include "src/sensors/sensor_simulation.h"

// ─────────────────────────────────────────────────────────────────────────────
// Utilidades de presentación
// ─────────────────────────────────────────────────────────────────────────────
void printBanner() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║   SISTEMA DE RIEGO AUTOMATIZADO — UNMSM AyDA 2026-I             ║\n";
    std::cout << "║   Proyecto 3: Control de Riego con Algoritmos de Optimización   ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════╝\n\n";
}

void printSeparator(char c = '─', int w = 68) {
    std::cout << std::string(w, c) << "\n";
}

void printSection(const std::string& title) {
    std::cout << "\n";
    printSeparator('═');
    std::cout << "  " << title << "\n";
    printSeparator('═');
}

// ─────────────────────────────────────────────────────────────────────────────
// Demo 1: Mostrar zonas generadas y lecturas de sensores
// ─────────────────────────────────────────────────────────────────────────────
void demoSensors(int n = 8) {
    printSection("DEMO 1: LECTURAS DE SENSORES Y ESTADO DE ZONAS");

    ClimateSimulator climate;
    auto zones = generateZones(n);
    SensorEMAFilter filter(n);
    ClimateData cd = climate.next();

    // Actualizar ET0 con datos climáticos
    double et0_val = cd.computeET0();
    for (auto& z : zones) z.et0 = et0_val;

    auto readings = filter.read(zones);

    std::cout << "\n📍 Condiciones Climáticas (Ica, Perú):\n";
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "   Temperatura:    " << cd.temperature   << " °C\n";
    std::cout << "   HR Ambiental:   " << cd.humidity_air  << " %\n";
    std::cout << "   Viento:         " << cd.wind_speed     << " m/s\n";
    std::cout << "   Radiación Sol.: " << cd.solar_rad      << " MJ/m²/día\n";
    std::cout << "   Precipitación:  " << cd.precipitation  << " mm\n";
    std::cout << "   ET₀ calculada:  " << et0_val           << " mm/día\n";

    std::cout << "\n📊 Estado de Zonas de Cultivo:\n\n";
    std::cout << std::left
              << std::setw(10) << "Zona"
              << std::setw(14) << "Cultivo"
              << std::setw(12) << "Hum. Real"
              << std::setw(12) << "Hum. EMA"
              << std::setw(12) << "Área (m²)"
              << std::setw(10) << "Urgencia"
              << std::setw(12) << "Necesidad"
              << "\n";
    printSeparator();

    for (int i = 0; i < n; ++i) {
        const auto& z = zones[i];
        // Aplicar lectura filtrada
        IrrigationZone zf = z;
        zf.humidity = readings[i];

        std::string urgency_bar(static_cast<int>(z.urgency() * 10), '█');
        urgency_bar.resize(10, '░');

        std::cout << std::left
                  << std::setw(10) << z.name
                  << std::setw(14) << z.crop().name
                  << std::setw(12) << (std::to_string(static_cast<int>(z.humidity)) + "%")
                  << std::setw(12) << (std::to_string(static_cast<int>(readings[i])) + "%")
                  << std::setw(12) << static_cast<int>(z.area_m2)
                  << std::setw(10) << urgency_bar
                  << std::setw(12) << (std::to_string(static_cast<int>(z.waterNeed() * 10) / 10.0) + " m³")
                  << "\n";
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Demo 2: Ejecutar y comparar ambos algoritmos
// ─────────────────────────────────────────────────────────────────────────────
void demoAlgorithms(int n = 10) {
    printSection("DEMO 2: EJECUCIÓN Y COMPARACIÓN DE ALGORITMOS");

    auto zones_g = generateZones(n, 100);
    auto zones_d = zones_g;  // copia para DP
    double W = 200.0;        // 200 m³ disponibles

    std::cout << "\n⚙️  Parámetros:\n";
    std::cout << "   n = " << n << " zonas de cultivo\n";
    std::cout << "   W = " << W << " m³ de agua disponible\n\n";

    // ── Ejecutar Greedy ──
    auto gr = GreedyIrrigation::run(zones_g, W);

    // ── Ejecutar DP ──
    auto dp = DPIrrigation::run(zones_d, W, 2.0, 1000);

    // ── Tabla de asignaciones ──
    std::cout << std::left
              << std::setw(10) << "Zona"
              << std::setw(14) << "Cultivo"
              << std::setw(14) << "Necesidad"
              << std::setw(14) << "Greedy (m³)"
              << std::setw(14) << "DP (m³)"
              << "\n";
    printSeparator();

    for (int i = 0; i < n; ++i) {
        std::cout << std::left
                  << std::setw(10) << zones_g[i].name
                  << std::setw(14) << zones_g[i].crop().name
                  << std::setw(14) << std::fixed << std::setprecision(2) << zones_g[i].waterNeed()
                  << std::setw(14) << gr.assignments[i]
                  << std::setw(14) << (dp.assignments.empty() ? 0.0 : dp.assignments[i])
                  << "\n";
    }

    printSeparator();
    std::cout << "\n📈 Resultados Comparativos:\n\n";
    std::cout << std::left << std::setw(30) << "Métrica"
              << std::setw(20) << "GREEDY"
              << std::setw(20) << "PROG. DINÁMICA" << "\n";
    printSeparator('-');
    std::cout << std::left << std::setw(30) << "Complejidad temporal"
              << std::setw(20) << "O(n log n)"
              << std::setw(20) << "O(n·C)" << "\n";
    std::cout << std::left << std::setw(30) << "Complejidad espacial"
              << std::setw(20) << "O(n)"
              << std::setw(20) << "O(C) [rolling]" << "\n";
    std::cout << std::left << std::setw(30) << "Tiempo ejecución"
              << std::setw(20) << (std::to_string(gr.exec_time_us) + " μs")
              << std::setw(20) << (std::to_string(dp.exec_time_us) + " μs") << "\n";
    std::cout << std::left << std::setw(30) << "Agua asignada (m³)"
              << std::setw(20) << std::fixed << std::setprecision(2) << gr.total_assigned
              << std::setw(20) << dp.total_assigned << "\n";
    std::cout << std::left << std::setw(30) << "Beneficio total"
              << std::setw(20) << std::setprecision(3) << gr.total_benefit
              << std::setw(20) << dp.total_benefit << "\n";
    std::cout << std::left << std::setw(30) << "Eficiencia hídrica (%)"
              << std::setw(20) << std::setprecision(1) << gr.water_efficiency
              << std::setw(20) << dp.water_efficiency << "\n";
    std::cout << std::left << std::setw(30) << "Adecuado para tiempo real"
              << std::setw(20) << "✓ Sí"
              << std::setw(20) << "✗ No (offline)" << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// Demo 3: Benchmarks de escalabilidad
// ─────────────────────────────────────────────────────────────────────────────
void demoBenchmark() {
    printSection("DEMO 3: ANÁLISIS DE ESCALABILIDAD — BENCHMARKS");

    std::vector<int> sizes = {10, 50, 100, 500, 1000, 5000, 10000, 100000};

    std::cout << "\n" << std::left
              << std::setw(12) << "n (zonas)"
              << std::setw(18) << "Greedy (μs)"
              << std::setw(22) << "Greedy / (n·log₂n)"
              << std::setw(18) << "DP (μs)"
              << "\n";
    printSeparator();

    for (int n : sizes) {
        auto zones = generateZones(n);
        double W = n * 20.0;

        // Greedy
        auto t1 = std::chrono::high_resolution_clock::now();
        auto zones_copy = zones;
        GreedyIrrigation::run(zones_copy, W);
        auto t2 = std::chrono::high_resolution_clock::now();
        long long g_us = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

        double n_logn = (n > 1) ? static_cast<double>(n) * std::log2(static_cast<double>(n)) : n;
        double ratio  = static_cast<double>(g_us) / n_logn;

        // DP (solo para n pequeño)
        std::string dp_str;
        if (n <= 1000) {
            auto zones_dp = zones;
            auto t3 = std::chrono::high_resolution_clock::now();
            DPIrrigation::run(zones_dp, std::min(W, 500.0), 2.0, 500);
            auto t4 = std::chrono::high_resolution_clock::now();
            long long dp_us = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count();
            dp_str = std::to_string(dp_us);
        } else {
            dp_str = "N/A (memoria)";
        }

        std::cout << std::left
                  << std::setw(12) << n
                  << std::setw(18) << g_us
                  << std::setw(22) << std::fixed << std::setprecision(4) << ratio
                  << std::setw(18) << dp_str
                  << "\n";
    }

    std::cout << "\n💡 Nota: El ratio Greedy/(n·log₂n) ≈ constante confirma O(n log n)\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// Demo 4: Simulación de ciclos de riego en tiempo real
// ─────────────────────────────────────────────────────────────────────────────
void demoRealTimeSimulation(int n = 6, int cycles = 5) {
    printSection("DEMO 4: SIMULACIÓN EN TIEMPO REAL — " + std::to_string(cycles) + " CICLOS DE RIEGO");

    auto zones = generateZones(n, 200);
    ClimateSimulator climate;
    SensorEMAFilter filter(n);
    double W_per_cycle = 80.0;
    double total_saved = 0.0;
    double traditional_usage = W_per_cycle * cycles;  // riego tradicional usa siempre W completo

    for (int cycle = 1; cycle <= cycles; ++cycle) {
        auto cd = climate.next();
        double et0 = cd.computeET0();
        for (auto& z : zones) z.et0 = et0;

        auto readings = filter.read(zones);
        for (int i = 0; i < n; ++i) zones[i].humidity = readings[i];

        auto result = GreedyIrrigation::run(zones, W_per_cycle);

        double saved = W_per_cycle - result.total_assigned;
        total_saved += saved;

        std::cout << "\n🔄 Ciclo " << cycle << " | Hora " << climate.hour()
                  << ":00 | Temp: " << std::fixed << std::setprecision(1)
                  << cd.temperature << "°C | ET₀: " << et0 << " mm/día\n";

        std::cout << std::left
                  << std::setw(10) << "Zona"
                  << std::setw(12) << "Cultivo"
                  << std::setw(12) << "Hum. (%)"
                  << std::setw(14) << "Asignado (m³)"
                  << std::setw(12) << "Estado"
                  << "\n";
        printSeparator('-');

        for (int i = 0; i < n; ++i) {
            std::string estado;
            double vol = result.assignments[i];
            if (zones[i].urgency() > 0.5) estado = "⚠️  CRÍTICO";
            else if (zones[i].urgency() > 0.1) estado = "💧 Bajo";
            else estado = "✅ OK";

            std::cout << std::left
                      << std::setw(10) << zones[i].name
                      << std::setw(12) << zones[i].crop().name
                      << std::setw(12) << static_cast<int>(zones[i].humidity)
                      << std::setw(14) << std::setprecision(2) << vol
                      << estado << "\n";

            // Actualizar humedad tras el riego
            zones[i].stepTime(vol, cd.precipitation);
        }

        std::cout << "   ▶ Agua usada: " << std::setprecision(1) << result.total_assigned
                  << " m³ / " << W_per_cycle << " m³ disponibles"
                  << " | Ahorro: " << saved << " m³\n";
    }

    std::cout << "\n📊 RESUMEN DE SIMULACIÓN:\n";
    std::cout << "   Agua total disponible:      " << traditional_usage << " m³\n";
    std::cout << "   Agua total utilizada:        " << (traditional_usage - total_saved) << " m³\n";
    std::cout << "   Agua ahorrada vs. tradicional: " << total_saved << " m³ ("
              << std::setprecision(1) << (total_saved / traditional_usage * 100.0) << "%)\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// MAIN
// ─────────────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    printBanner();

    int demo = 0;
    if (argc > 1) demo = std::atoi(argv[1]);

    switch (demo) {
        case 1: demoSensors(8);           break;
        case 2: demoAlgorithms(10);       break;
        case 3: demoBenchmark();          break;
        case 4: demoRealTimeSimulation(); break;
        default:
            // Ejecutar todo
            demoSensors(8);
            demoAlgorithms(10);
            demoBenchmark();
            demoRealTimeSimulation(6, 5);
            break;
    }

    std::cout << "\n";
    printSeparator('═');
    std::cout << "  Fin del programa — UNMSM AyDA 2026-I — Proyecto 3\n";
    printSeparator('═');
    std::cout << "\n";

    return 0;
}