#ifndef GROWTH_MODELS_HPP
#define GROWTH_MODELS_HPP

#include <array>
#include <cstddef>

// Modelos de crecimiento de una sola especie (d=1).
// Campo de pruebas para integradores escalares.
// La solución exacta es conocida en ambos casos, lo que
// los hace ideales para medir el error acumulado de cada integrador.

namespace dsv {
namespace mod {


// ──────────────────────────────────────────────────────────────────────────────
// Malthus — Crecimiento Exponencial
//
//   dx/dt = r * x
//
// Solución exacta: x(t) = x0 * exp(r * t)
// Cualquier error de paso se amplifica exponencialmente: prueba
// de fuego para detectar deriva numérica en el integrador.
// ──────────────────────────────────────────────────────────────────────────────
struct Malthus_Model {

    static constexpr size_t dim       = 1;
    static constexpr size_t noise_dim = 0;

    float r = 0.3f; // tasa intrínseca de crecimiento (r > 0: crece, r < 0: decrece)

    void drift(const std::array<float, dim>& x,
               float /*t*/,
               std::array<float, dim>& out) const
    {
        out[0] = r * x[0];
    }
};




// ──────────────────────────────────────────────────────────────────────────────
// Logístico (Verhulst) — Crecimiento con Capacidad de Carga
//
//   dx/dt = r * x * (1 - x/K)
//
// Solución exacta: x(t) = K / (1 + ((K - x0)/x0) * exp(-r*t))
// Punto de equilibrio estable en x* = K, inestable en x* = 0.
// ──────────────────────────────────────────────────────────────────────────────
struct Logistic_Model {

    static constexpr size_t dim       = 1;
    static constexpr size_t noise_dim = 0;

    float r = 0.4f;   // tasa intrínseca de crecimiento  [0.1, 1.0]
    float K = 100.0f; // capacidad de carga del entorno   (K > 0)

    void drift(const std::array<float, dim>& x,
               float /*t*/,
               std::array<float, dim>& out) const
    {
        out[0] = r * x[0] * (1.0f - x[0] / K);
    }
};


} // namespace mod
} // namespace dsv
#endif // GROWTH_MODELS_HPP
