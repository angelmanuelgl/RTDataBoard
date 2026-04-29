#ifndef MOD_HPP
#define MOD_HPP

#include <array>
#include <variant>


namespace dsv {
namespace mod {



// ModelRef: cero overhead para el caso local,
// un puntero raw para el caso compartido.
// std::variant<Model, Model*> ocupa sizeof(Model) + 1 byte.

template<typename Model>
struct ModelRef {
    std::variant<Model, Model*> storage { Model{} };

    Model& get() {
        return std::visit([](auto& v) -> Model& {
            if constexpr (std::is_pointer_v<std::decay_t<decltype(v)>>)
                return *v;
            else
                return v;
        }, storage);
    }

    const Model& get() const { /* idem */ }

    void vincular(Model& externo)   { storage = &externo; }
    void usar_propio()              { storage = Model{};  }
    bool es_propio() const          { return std::holds_alternative<Model>(storage); }
};

// Instance genérica: reemplaza SIR_Instance, Lorenz_Instance, ORG_Instance, etc.
template<typename Model>
struct Instance {
    using state_t = std::array<float, Model::dim>;

    ModelRef<Model> model_ref;
    state_t state {};
    float t = 0.0f;

    // Interfaz que esperan los integradores
    Model&       getModel()       { return model_ref.get(); }
    const Model& getModel() const { return model_ref.get(); }
};


}  // end dsv
} // end mod
#endif // end hpp
