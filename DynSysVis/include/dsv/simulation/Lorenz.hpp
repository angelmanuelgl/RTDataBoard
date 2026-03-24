#ifndef LORENZ_HPP
#define LORENZ_HPP

#include <array>
#include <cstddef>

namespace dsv {
namespace mod {

struct Lorenz_Model {

    static constexpr size_t dim = 3;
    static constexpr size_t noise_dim = 0;

    float sigma = 10.0f;
    float rho   = 28.0f;
    float beta  = 8.0f / 3.0f;
    float ruido = 2.0f;

    enum { X, Y, Z };

    void drift(const std::array<float, dim>& v,
               float t,
               std::array<float, dim>& out) const
    {
        out[X] = sigma * (v[Y] - v[X]);
        out[Y] = v[X] * (rho - v[Z]) - v[Y];
        out[Z] = v[X] * v[Y] - beta * v[Z];
    }

    void diffusion(const std::array<float, dim>& v, float t,
               std::array<std::array<float, noise_dim>, dim>& out) const
    {
        // Limpiamos la matriz (ponemos todo a 0)
        for(auto& row : out) row.fill(0.0f);

        // Ruido constante e independiente para cada eje
        out[X][0] = ruido; 
        out[Y][1] = ruido;
        out[Z][2] = ruido;
    }
};

struct Lorenz_Instance {
    Lorenz_Model model;

    dsv::mod::Lorenz_Model& getModel() {
        return model;
    }

    std::array<float, Lorenz_Model::dim> state { 1.0f, 1.0f, 1.0f };
    float t = 0.0f;

};



}  // end dsv
} // end mod
#endif // end hpp
