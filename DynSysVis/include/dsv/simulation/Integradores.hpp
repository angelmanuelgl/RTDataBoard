#ifndef INTEGRADORES_HPP
#define INTEGRADORES_HPP

#include <random>
#include "dsv/core/Logger.hpp"

namespace dsv {
namespace sim {
    // --- --- ODE --- --- 
    // EULER
    template<typename Instance>
    void E_step(Instance& sys, float dt) {
        using Model = typename std::decay<decltype(sys.getModel())>::type;
        constexpr size_t d = Model::dim;

        std::array<float, d> a;
        sys.getModel().drift(sys.state, sys.t, a); // Solo necesitamos la tendencia

        for(size_t i = 0; i < d; ++i ){
            sys.state[i] += a[i] * dt;
        }
        sys.t += dt;
    }

    // --- --- SDE --- --- 
    //  Euler-Maruyama un ruido
    template<typename Instance>
    void EM_step_simple(Instance& sys, float dt) {
        using Model = typename std::decay<decltype(sys.getModel())>::type;
        constexpr size_t d = Model::dim;

        std::array<float, d> a;
        std::array<float, d> g; // Vector de difusión en lugar de matriz

        sys.getModel().drift(sys.state, sys.t, a);
        sys.getModel().diffusion(sys.state, sys.t, g);

        static std::mt19937 gen(std::random_device{}());
        static std::normal_distribution<float> dist(0.0f, 1.0f);
        float dW = dist(gen) * std::sqrt(dt);

        for(size_t i = 0; i < d; ++i) {
            sys.state[i] += a[i] * dt + g[i] * dW;
        }
        sys.t += dt;
    }

    //  Euler-Maruyama un m ruidos Independiente
    template<typename Instance>
    void EM_step(Instance& sys, float dt)
    {
        using Model = typename std::decay<decltype(sys.getModel())>::type;

        constexpr size_t d = Model::dim;
        constexpr size_t m = Model::noise_dim;

        std::array<float, d> a;
        std::array<std::array<float, m>, d> B;
        std::array<float, m> dW;

        sys.getModel().drift(sys.state, sys.t, a);
        sys.getModel().diffusion(sys.state, sys.t, B);

        static std::mt19937 gen(std::random_device{}());
        static std::normal_distribution<float> dist(0.0f, 1.0f);

        for(size_t k = 0; k < m; ++k)
            dW[k] = dist(gen) * std::sqrt(dt);

        for(size_t i = 0; i < d; ++i)
        {
            float stochastic = 0.0f;
            for(size_t k = 0; k < m; ++k)
                stochastic += B[i][k] * dW[k];

            sys.state[i] += a[i] * dt + stochastic;
        }

        sys.t += dt;
    }

    // GENERAL
    template<typename Instance>
    void step(Instance& sys, float dt) {
        using Model = typename std::decay<decltype(sys.getModel())>::type;
        
        // dim = 0//  determinista
        if constexpr (Model::noise_dim == 0) {
            E_step(sys, dt);
        } 
        // dim = 1// simple
        else if constexpr (Model::noise_dim == 1) {
            EM_step_simple(sys, dt);
        }
        // n x m matriz
        else {
            EM_step(sys, dt); 
        }
    }

    // --- --- ODE --- --- 
    // Runge-Kutta 4to Orden (RK4)
    template<typename Instance>
    void RK4_step(Instance& sys, float dt) {
        using Model = typename std::decay<decltype(sys.getModel())>::type;
        constexpr size_t d = Model::dim;

        // Estructuras para las 4 etapas
        std::array<float, d> k1, k2, k3, k4;
        std::array<float, d> temp_state;

        float h = dt;
        float h2 = h / 2.0f;

        // k1 = f(t, x)
        sys.getModel().drift(sys.state, sys.t, k1);

        // k2 = f(t + h/2, x + h/2 * k1)
        for(size_t i = 0; i < d; ++i) temp_state[i] = sys.state[i] + h2 * k1[i];
        sys.getModel().drift(temp_state, sys.t + h2, k2);

        // k3 = f(t + h/2, x + h/2 * k2)
        for(size_t i = 0; i < d; ++i) temp_state[i] = sys.state[i] + h2 * k2[i];
        sys.getModel().drift(temp_state, sys.t + h2, k3);

        // k4 = f(t + h, x + h * k3)
        for(size_t i = 0; i < d; ++i) temp_state[i] = sys.state[i] + h * k3[i];
        sys.getModel().drift(temp_state, sys.t + h, k4);

        // actualizacion final: x = x + (h/6) * (k1 + 2k2 + 2k3 + k4)
        for(size_t i = 0; i < d; ++i) {
            sys.state[i] += (h / 6.0f) * (k1[i] + 2.0f * k2[i] + 2.0f * k3[i] + k4[i]);
        }
        
        sys.t += h;
    }

} // namespace sim
} // namespace dsv
#endif // integradores