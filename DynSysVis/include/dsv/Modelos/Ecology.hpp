#ifndef ECOLOGY_HPP
#define ECOLOGY_HPP

#include <array>

// Sistema de dos ODEs acopladas no lineales.
//
// ADVERTENCIA SOBRE INTEGRADORES:
//   El sistema conserva la cantidad de Lyapunov V(x,y) = delta*x - gamma*ln(x)
//                                                       + beta*y  - alpha*ln(y)
//   Euler (E_step) destruye esta conservación: la espiral se abre o se cierra
//   artificialmente. Para órbitas cerradas estables usar RK4_step.

namespace dsv {
namespace mod {



// ──────────────────────────────────────────────────────────────────────────────
// LotkaVolterra — Depredador-Presa
//
//   dx1/dt = x1 * (alpha - beta  * x2)   <- presas
//   dx2/dt = x2 * (delta * x1   - gamma) <- depredadores
//
// Punto de equilibrio: (x1*, x2*) = (gamma/delta, alpha/beta)
// Con condición inicial distinta del equilibrio → órbitas cerradas periódicas.
// ──────────────────────────────────────────────────────────────────────────────
struct LotkaVolterra_Model {

    static constexpr size_t dim       = 2;
    static constexpr size_t noise_dim = 0;

    // Parámetros ecológicos (todos positivos)
    float alpha = 1.1f; // tasa de nacimiento de presas
    float beta  = 0.4f; // éxito de caza (encuentros presa-depredador)
    float delta = 0.1f; // eficiencia alimenticia del depredador
    float gamma = 0.4f; // tasa de muerte natural del depredador

    enum { PRESA, DEPREDADOR };

    void drift(const std::array<float, dim>& x,
               float /*t*/,
               std::array<float, dim>& out) const
    {
        out[PRESA]      = x[PRESA]      * (alpha - beta  * x[DEPREDADOR]);
        out[DEPREDADOR] = x[DEPREDADOR] * (delta * x[PRESA] - gamma);
    }
};



// ──────────────────────────────────────────────────────────────────────────────
// StochasticLotkaVolterra — Depredador-Presa con Ruido Ambiental
//
//   dx1 = x1·(α - β·x2) dt + σ1·x1 dW1     <- presas
//   dx2 = x2·(δ·x1 - γ) dt + σ2·x2 dW2     <- depredadores
//
// El ruido modela variaciones ambientales (clima, disponibilidad de alimento)
// que afectan las tasas de natalidad/mortalidad de cada especie de forma
// independiente. Cada especie tiene su propio proceso de Wiener.
//
// La difusión es diagonal en la matriz B (d×m con d=m=2):
//
//   B = | σ1·x1    0    |
//       |  0    σ2·x2  |
//
// El ruido multiplicativo xi·dWi asegura que si xi→0, el ruido también → 0:
// una especie extinta no puede recuperarse por fluctuación.
//
// Parámetro   Fenómeno
//   alpha     tasa de natalidad de presas
//   beta      tasa de depredación (encuentros letales)
//   delta     eficiencia de conversión presa→depredador
//   gamma     tasa de mortalidad natural del depredador
//   sigma1    intensidad de ruido ambiental en presas
//   sigma2    intensidad de ruido ambiental en depredadores
// ──────────────────────────────────────────────────────────────────────────────
struct StochasticLotkaVolterra_Model {

    static constexpr size_t dim       = 2;
    static constexpr size_t noise_dim = 2; // → usa EM_step (matriz d×m)

    float alpha  = 1.1f;  // natalidad de presas
    float beta   = 0.4f;  // éxito de caza
    float delta  = 0.1f;  // eficiencia alimenticia
    float gamma  = 0.4f;  // mortalidad del depredador
    float sigma1 = 0.10f; // ruido ambiental en presas
    float sigma2 = 0.10f; // ruido ambiental en depredadores

    enum { PRESA, DEPREDADOR };

    void drift(const std::array<float, dim>& x,
               float /*t*/,
               std::array<float, dim>& out) const
    {
        out[PRESA]      = x[PRESA]      * (alpha - beta  * x[DEPREDADOR]);
        out[DEPREDADOR] = x[DEPREDADOR] * (delta * x[PRESA] - gamma);
    }

    // Matriz de difusión B (d × m) = (2 × 2), diagonal
    void diffusion(const std::array<float, dim>& x,
                   float /*t*/,
                   std::array<std::array<float, noise_dim>, dim>& out) const
    {
        out[PRESA]     [0] = sigma1 * x[PRESA];      out[PRESA]     [1] = 0.0f;
        out[DEPREDADOR][0] = 0.0f;                    out[DEPREDADOR][1] = sigma2 * x[DEPREDADOR];
    }
};


// ──────────────────────────────────────────────────────────────────────────────
// Nacimientos -> Obreras -> Recolectoras ->  Guerreras -> Muertes
//                                        -> Muertes
//
// ver mas sobre ete modelo en:
// https://www.jovenesenlaciencia.ugto.mx/index.php/jovenesenlaciencia/article/view/4873
// ──────────────────────────────────────────────────────────────────────────────
struct AntColony_Model {
    static constexpr size_t dim       = 3;
    static constexpr size_t noise_dim = 0;

    // Parámetros del modelo (con tus valores por defecto)
    float bO      = 0.05f;
    float betaOG  = 4.0f;
    float betaGR  = 2.5f;
    float betaRG  = 1.2f;
    float kG      = 50.0f;
    float kR      = 50.0f;
    float dG      = 0.005f;
    float dR      = 0.005f;

    enum { O, G, R }; // Mapeo de índices

    void drift(const std::array<float, dim>& x,
               float /*t*/,
               std::array<float, dim>& out) const 
    {
        // x[O] = Obreras, x[G] = Guerreras, x[R] = Recolectoras
        
        // dO/dt = bO*O - (betaOG / (kG + G))*O
        out[O] = (bO * x[O]) - (betaOG / (kG + x[G])) * x[O];

        // dR/dt = (betaGR / (kR + R)) * G - (betaRG / (kG + G)) * R - (dR * R)
        out[R] = (betaGR / (kR + x[R])) * x[G] 
                 - (betaRG / (kG + x[G])) * x[R] 
                 - (dR * x[R]);
                 
        // dG/dt = (betaOG / (kG + G)) * O - (betaGR / (kR + R)) * G - (dG * G) + (betaRG / (kG + G)) * R
        out[G] = (betaOG / (kG + x[G])) * x[O] 
                 - (betaGR / (kR + x[R])) * x[G] 
                 - (dG * x[G]) 
                 + (betaRG / (kG + x[G])) * x[R];

        
    }
};


} // namespace mod
} // namespace dsv
#endif // ECOLOGY_HPP
