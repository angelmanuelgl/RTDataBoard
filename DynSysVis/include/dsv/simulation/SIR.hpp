#ifndef MOD_HPP
#define MOD_HPP

#include <array>
#include <cstddef>

namespace dsv {
namespace mod {


struct SIR_Model {

    static constexpr size_t dim = 3;
    static constexpr size_t noise_dim = 3;

    float beta  = 0.3f; // infeccion
    float gamma = 0.01f; // recuperacion
    float ruido = 0.05f;

    enum { S, I, R };

    void drift(const std::array<float, dim>& x,
               float t,
               std::array<float, dim>& out) const
    {
        float N = x[S] + x[I] + x[R];
        out[S] = -beta * x[S] * x[I] / N;
        out[I] = (beta * x[S] * x[I] / N) - (gamma * x[I]);
        out[R] = gamma * x[I];
    }

    void diffusion(const std::array<float, dim>& x, float t, std::array<std::array<float, noise_dim>, dim>& out) {
        for(auto& row : out) row.fill(0.0f);
        out[S][S] = -ruido * x[S];
        out[I][I] =  ruido * x[I];
        out[R][R] =  ruido * 0.05f; // Ruido bajo en recuperados
    }
};

struct SIR_Instance {
private:
    //  modelo propio (por si no quiere compartir)
    dsv::mod::SIR_Model localModel; 
    
    // puntero a modelo compartido
    dsv::mod::SIR_Model* sharedModelPtr = nullptr;

public:
    dsv::mod::SIR_Model& getModel(){
        return (sharedModelPtr != nullptr) ? *sharedModelPtr : localModel;
    }
    
    // para compartir (Usa la dirección de memoria &)
    void vincularModelo(dsv::mod::SIR_Model& externo) {
        sharedModelPtr = &externo;
    }
    // para volver a ser independiente
    void usarModeloPropio() {
        sharedModelPtr = nullptr;
    }

    std::array<float, SIR_Model::dim> state { 0.99f, 0.01f, 0.0f };
    float t = 0.0f;
    
};


}  // end dsv
} // end mod
#endif // end hpp
