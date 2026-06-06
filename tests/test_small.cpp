// tests/test_small.cpp — Casos pequeños verificables manualmente (n=10)
// Entregable: Semana 12 · AyDA 2026-I
#include <cassert>
#include <cmath>
#include <iostream>
#include "../src/models/irrigation_zone.h"
#include "../src/algorithms/greedy_irrigation.h"
#include "../src/algorithms/dp_irrigation.h"
#include "../src/sensors/sensor_simulation.h"

// ─── Utilidad ─────────────────────────────────────────────────────────────
void ASSERT(bool cond, const std::string& msg) {
    if (!cond) {
        std::cerr << "[FAIL] " << msg << "\n";
        std::exit(1);
    }
    std::cout << "[PASS] " << msg << "\n";
}

// ─── TEST 1: Urgencia correcta ─────────────────────────────────────────────
void test_urgency() {
    IrrigationZone z;
    z.crop_index = 0;  // Espárrago: hmin=40
    z.humidity   = 20.0;
    double expected = (40.0 - 20.0) / 40.0;  // = 0.5
    ASSERT(std::abs(z.urgency() - expected) < 1e-9,
           "Urgencia = 0.5 cuando humedad=20 y hmin=40");

    z.humidity = 60.0;
    ASSERT(z.urgency() == 0.0, "Urgencia = 0 cuando humedad > hmin");
}

// ─── TEST 2: No negatividad de asignaciones ────────────────────────────────
void test_non_negativity() {
    auto zones = generateZones(10, 1);
    auto result = GreedyIrrigation::run(zones, 200.0);
    for (double v : result.assignments)
        ASSERT(v >= 0.0, "Asignacion >= 0 para toda zona");
}

// ─── TEST 3: No exceder agua disponible ───────────────────────────────────
void test_water_constraint() {
    auto zones = generateZones(10, 2);
    double W = 150.0;
    auto result = GreedyIrrigation::run(zones, W);
    double total = 0;
    for (double v : result.assignments) total += v;
    ASSERT(total <= W + 1e-6, "Total asignado <= W disponible");
}

// ─── TEST 4: Greedy prioriza zonas con mayor urgencia ─────────────────────
void test_priority_order() {
    std::vector<IrrigationZone> zones(3);
    zones[0] = {0, "Alta",  0, 10.0, 1000.0, 4.5, 0.0, 0.0, true};  // urgencia alta
    zones[1] = {1, "Media", 0, 35.0, 1000.0, 4.5, 0.0, 0.0, true};  // urgencia media
    zones[2] = {2, "Baja",  0, 70.0, 1000.0, 4.5, 0.0, 0.0, true};  // urgencia 0
    double W = 30.0;
    auto result = GreedyIrrigation::run(zones, W);
    ASSERT(result.assignments[0] >= result.assignments[2],
           "Zona de alta urgencia recibe >= agua que zona de baja urgencia");
}

// ─── TEST 5: DP da resultado >= 0 ─────────────────────────────────────────
void test_dp_non_negative() {
    auto zones = generateZones(10, 3);
    auto result = DPIrrigation::run(zones, 200.0, 2.0, 200);
    ASSERT(result.total_benefit >= 0.0, "Beneficio DP >= 0");
}

// ─── TEST 6: Filtro EMA converge ──────────────────────────────────────────
void test_ema_convergence() {
    auto zones = generateZones(5, 4);
    SensorEMAFilter f(5, 0.3);
    // Ejecutar 20 ciclos y verificar que la salida está en [0,100]
    for (int i = 0; i < 20; ++i) {
        auto readings = f.read(zones);
        for (double r : readings)
            ASSERT(r >= 0.0 && r <= 100.0, "Lectura EMA en rango [0,100]");
    }
}

// ─── TEST 7: Complejidad temporal (n=10, debe ser < 1ms) ──────────────────
void test_time_small() {
    auto zones = generateZones(10, 5);
    auto result = GreedyIrrigation::run(zones, 100.0);
    ASSERT(result.exec_time_us < 1000,
           "Greedy n=10 ejecuta en < 1ms (" +
           std::to_string(result.exec_time_us) + " us)");
}

// ─── TEST 8: Zona inactiva no recibe agua ─────────────────────────────────
void test_inactive_zone() {
    auto zones = generateZones(5, 6);
    zones[2].active = false;
    zones[2].humidity = 5.0;  // urgencia máxima pero inactiva
    auto result = GreedyIrrigation::run(zones, 300.0);
    ASSERT(result.assignments[2] == 0.0,
           "Zona inactiva recibe 0 agua");
}

// ─── TEST 9: Cero agua disponible → todo cero ─────────────────────────────
void test_zero_water() {
    auto zones = generateZones(10, 7);
    auto result = GreedyIrrigation::run(zones, 0.0);
    double total = 0;
    for (double v : result.assignments) total += v;
    ASSERT(std::abs(total) < 1e-9, "Con W=0 no se asigna agua");
}

// ─── TEST 10: Modelo de cultivo — parámetros FAO-56 ───────────────────────
void test_crop_params() {
    // Verificar que kc_mid del espárrago sea 0.95 (FAO-56)
    ASSERT(std::abs(CROPS[0].kc_mid - 0.95) < 1e-9,
           "Kc_mid espárrago = 0.95 (FAO-56)");
    ASSERT(std::abs(CROPS[1].ai - 0.90) < 1e-9,
           "Eficiencia riego palto = 0.90");
    ASSERT(CROPS[2].hmin == 45.0, "Hmin maíz = 45%");
    ASSERT(CROPS[3].hmax == 88.0, "Hmax papa = 88%");
}

// ─────────────────────────────────────────────────────────────────────────────
int main() {
    std::cout << "\n=== TEST SMALL (n=10) — Casos verificables manualmente ===\n\n";
    test_urgency();
    test_non_negativity();
    test_water_constraint();
    test_priority_order();
    test_dp_non_negative();
    test_ema_convergence();
    test_time_small();
    test_inactive_zone();
    test_zero_water();
    test_crop_params();
    std::cout << "\n✓ Todos los tests pequeños pasaron.\n\n";
    return 0;
}