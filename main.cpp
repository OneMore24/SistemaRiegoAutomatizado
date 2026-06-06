#define _USE_MATH_DEFINES
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <string>
#include <sstream>
#include <numeric>

#include "src/models/irrigation_zone.h"
#include "src/algorithms/greedy_irrigation.h"
#include "src/algorithms/dp_irrigation.h"
#include "src/sensors/sensor_simulation.h"

// ─────────────────────────────────────────────────────────────────────────────
// Utilidades de presentacion (sin caracteres Unicode)
// ─────────────────────────────────────────────────────────────────────────────
void printSeparator(char c = '-', int w = 68) {
    std::cout << std::string(w, c) << "\n";
}

void printSection(const std::string& title) {
    std::cout << "\n";
    printSeparator('=');
    std::cout << "  " << title << "\n";
    printSeparator('=');
}

void printBanner() {
    std::cout << "\n";
    printSeparator('*');
    std::cout << "   SISTEMA DE RIEGO AUTOMATIZADO -- UNMSM AyDA 2026-I\n";
    std::cout << "   Proyecto 3: Control de Riego con Algoritmos de Optimizacion\n";
    printSeparator('*');
    std::cout << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// Demo 1: Sensores y estado de zonas
// ─────────────────────────────────────────────────────────────────────────────
void demoSensors(int n = 8) {
    printSection("DEMO 1: LECTURAS DE SENSORES Y ESTADO DE ZONAS");

    ClimateSimulator climate;
    auto zones = generateZones(n);
    SensorEMAFilter filter(n);
    ClimateData cd = climate.next();

    double et0_val = cd.computeET0();
    for (auto& z : zones) z.et0 = et0_val;

    auto readings = filter.read(zones);

    std::cout << "\nCondiciones Climaticas (Ica, Peru):\n";
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "   Temperatura:    " << cd.temperature   << " C\n";
    std::cout << "   HR Ambiental:   " << cd.humidity_air  << " %\n";
    std::cout << "   Viento:         " << cd.wind_speed    << " m/s\n";
    std::cout << "   Radiacion Sol.: " << cd.solar_rad     << " MJ/m2/dia\n";
    std::cout << "   Precipitacion:  " << cd.precipitation << " mm\n";
    std::cout << "   ET0 calculada:  " << et0_val          << " mm/dia\n";

    std::cout << "\nEstado de Zonas de Cultivo:\n\n";
    std::cout << std::left
              << std::setw(10) << "Zona"
              << std::setw(14) << "Cultivo"
              << std::setw(12) << "Hum. Real"
              << std::setw(12) << "Hum. EMA"
              << std::setw(12) << "Area (m2)"
              << std::setw(12) << "Urgencia"
              << std::setw(12) << "Necesidad"
              << "\n";
    printSeparator();

    for (int i = 0; i < n; ++i) {
        const auto& z = zones[i];
        int bars = static_cast<int>(z.urgency() * 10);
        std::string urgency_bar(bars, '#');
        urgency_bar.resize(10, '.');

        std::cout << std::left
                  << std::setw(10) << z.name
                  << std::setw(14) << z.crop().name
                  << std::setw(12) << (std::to_string(static_cast<int>(z.humidity)) + "%")
                  << std::setw(12) << (std::to_string(static_cast<int>(readings[i])) + "%")
                  << std::setw(12) << static_cast<int>(z.area_m2)
                  << std::setw(12) << urgency_bar
                  << std::setprecision(2) << z.waterNeed() << " m3"
                  << "\n";
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Demo 2: Comparacion de algoritmos
// ─────────────────────────────────────────────────────────────────────────────
void demoAlgorithms(int n = 10) {
    printSection("DEMO 2: EJECUCION Y COMPARACION DE ALGORITMOS");

    auto zones_g = generateZones(n, 100);
    auto zones_d = zones_g;
    double W = 200.0;

    std::cout << "\nParametros:\n";
    std::cout << "   n = " << n << " zonas de cultivo\n";
    std::cout << "   W = " << W << " m3 de agua disponible\n\n";

    auto gr = GreedyIrrigation::run(zones_g, W);
    auto dp = DPIrrigation::run(zones_d, W, 2.0, 1000);

    std::cout << std::left
              << std::setw(10) << "Zona"
              << std::setw(14) << "Cultivo"
              << std::setw(14) << "Necesidad"
              << std::setw(14) << "Greedy (m3)"
              << std::setw(14) << "DP (m3)"
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
    std::cout << "\nResultados Comparativos:\n\n";
    std::cout << std::left << std::setw(30) << "Metrica"
              << std::setw(22) << "GREEDY"
              << std::setw(22) << "PROG. DINAMICA" << "\n";
    printSeparator('-');
    std::cout << std::left << std::setw(30) << "Complejidad temporal"
              << std::setw(22) << "O(n log n)"
              << std::setw(22) << "O(n*C)" << "\n";
    std::cout << std::left << std::setw(30) << "Complejidad espacial"
              << std::setw(22) << "O(n)"
              << std::setw(22) << "O(C) rolling array" << "\n";
    std::cout << std::left << std::setw(30) << "Tiempo ejecucion"
              << std::setw(22) << (std::to_string(gr.exec_time_us) + " us")
              << std::setw(22) << (std::to_string(dp.exec_time_us) + " us") << "\n";
    std::cout << std::left << std::setw(30) << "Agua asignada (m3)"
              << std::setw(22) << std::fixed << std::setprecision(2) << gr.total_assigned
              << std::setw(22) << dp.total_assigned << "\n";
    std::cout << std::left << std::setw(30) << "Beneficio total"
              << std::setw(22) << std::setprecision(3) << gr.total_benefit
              << std::setw(22) << dp.total_benefit << "\n";
    std::cout << std::left << std::setw(30) << "Eficiencia hidrica (%)"
              << std::setw(22) << std::setprecision(1) << gr.water_efficiency
              << std::setw(22) << dp.water_efficiency << "\n";
    std::cout << std::left << std::setw(30) << "Apto para tiempo real"
              << std::setw(22) << "SI"
              << std::setw(22) << "NO (offline)" << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// Demo 3: Benchmarks de escalabilidad
// ─────────────────────────────────────────────────────────────────────────────
void demoBenchmark() {
    printSection("DEMO 3: ANALISIS DE ESCALABILIDAD -- BENCHMARKS");

    std::vector<int> sizes = {10, 50, 100, 500, 1000, 5000, 10000, 100000};

    std::cout << "\n" << std::left
              << std::setw(12) << "n (zonas)"
              << std::setw(18) << "Greedy (us)"
              << std::setw(22) << "Greedy / (n*log2n)"
              << std::setw(18) << "DP (us)"
              << "\n";
    printSeparator();

    for (int n : sizes) {
        auto zones = generateZones(n);
        double W = n * 20.0;

        auto t1 = std::chrono::high_resolution_clock::now();
        auto zones_copy = zones;
        GreedyIrrigation::run(zones_copy, W);
        auto t2 = std::chrono::high_resolution_clock::now();
        long long g_us = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

        double n_logn = (n > 1) ? static_cast<double>(n) * std::log2(static_cast<double>(n)) : n;
        double ratio  = static_cast<double>(g_us) / n_logn;

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

    std::cout << "\nNota: ratio Greedy/(n*log2(n)) aprox. constante confirma O(n log n)\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// Demo 4: Simulacion en tiempo real
// ─────────────────────────────────────────────────────────────────────────────
void demoRealTimeSimulation(int n = 6, int cycles = 5) {
    printSection("DEMO 4: SIMULACION EN TIEMPO REAL -- " + std::to_string(cycles) + " CICLOS");

    auto zones = generateZones(n, 200);
    ClimateSimulator climate;
    SensorEMAFilter filter(n);
    double W_per_cycle = 80.0;
    double total_saved = 0.0;
    double traditional_usage = W_per_cycle * cycles;

    for (int cycle = 1; cycle <= cycles; ++cycle) {
        auto cd = climate.next();
        double et0 = cd.computeET0();
        for (auto& z : zones) z.et0 = et0;

        auto readings = filter.read(zones);
        for (int i = 0; i < n; ++i) zones[i].humidity = readings[i];

        auto result = GreedyIrrigation::run(zones, W_per_cycle);

        double saved = W_per_cycle - result.total_assigned;
        total_saved += saved;

        std::cout << "\nCiclo " << cycle
                  << " | Hora " << climate.hour() << ":00"
                  << " | Temp: " << std::fixed << std::setprecision(1) << cd.temperature << "C"
                  << " | ET0: " << et0 << " mm/dia\n";

        std::cout << std::left
                  << std::setw(10) << "Zona"
                  << std::setw(12) << "Cultivo"
                  << std::setw(12) << "Hum. (%)"
                  << std::setw(14) << "Asig. (m3)"
                  << std::setw(12) << "Estado"
                  << "\n";
        printSeparator('-');

        for (int i = 0; i < n; ++i) {
            double u = zones[i].urgency();
            std::string estado = u > 0.5 ? "CRITICO" : u > 0.1 ? "Bajo" : "OK";
            std::cout << std::left
                      << std::setw(10) << zones[i].name
                      << std::setw(12) << zones[i].crop().name
                      << std::setw(12) << static_cast<int>(zones[i].humidity)
                      << std::setw(14) << std::setprecision(2) << result.assignments[i]
                      << estado << "\n";
            zones[i].stepTime(result.assignments[i], cd.precipitation);
        }

        std::cout << "   Agua usada: " << std::setprecision(1) << result.total_assigned
                  << " m3 / " << W_per_cycle << " m3 | Ahorro: " << saved << " m3\n";
    }

    std::cout << "\nRESUMEN:\n";
    std::cout << "   Agua total disponible:   " << traditional_usage << " m3\n";
    std::cout << "   Agua total utilizada:    " << (traditional_usage - total_saved) << " m3\n";
    std::cout << "   Ahorro vs. tradicional:  " << total_saved << " m3 ("
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
            demoSensors(8);
            demoAlgorithms(10);
            demoBenchmark();
            demoRealTimeSimulation(6, 5);
            break;
    }

    std::cout << "\n";
    printSeparator('=');
    std::cout << "  Fin del programa -- UNMSM AyDA 2026-I -- Proyecto 3\n";
    printSeparator('=');
    std::cout << "\n";
    return 0;
}