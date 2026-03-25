#ifndef PANELFLOTANTE_HPP
#define PANELFLOTANTE_HPP

// std y sfml
#include <SFML/Graphics.hpp>
#include <string>

// dsv
#include "dsv/core/Logger.hpp"
#include "dsv/core/Panel.hpp"

namespace dsv{


enum class HandleOrientacion { Horizontal, Vertical };
enum class DespliegueDir     { Abajo, Arriba, Der, Izq };

class PanelFlotante {
private:


    // ── config ──────────────────────────────────────────────────
    sf::RenderWindow& window;
    HandleOrientacion orientacion;
    DespliegueDir     despliegue;

    
    // ── los dos paneles ─────────────────────────────────────────
    Panel handle;   // siempre visible
    Panel cuerpo;   // visible solo si abierto

    // ── tamanos fijos del handle según orientacion ──────────────
    // Horizontal: ancho generoso, alto compacto
    // Vertical:   ancho compacto, alto generoso
    static constexpr float HANDLE_ANCHO_H = 260.f;
    static constexpr float HANDLE_ALTO_H  =  32.f;
    static constexpr float HANDLE_ANCHO_V =  32.f;
    static constexpr float HANDLE_ALTO_V  = 260.f;

    static constexpr float ESPACIADO      =   6.f;

    // ── estado ──────────────────────────────────────────────────
    bool abierto = false;
    bool enHover = false;

    // ── helpers ─────────────────────────────────────────────────
    bool mouseEnHandle(sf::Vector2f mousePos) const;
    // void recalcularTamanoCuerpo();
    void recalcularPosCuerpo();
    void ajustarOrientacionHandle();

    // ── estilo ─────────────────────────────────────────────────
    std::string etiquetaOriginal="";
    float borde = 2.0f;
    float radioHandle = 10.0f;
public:

    PanelFlotante(
        sf::RenderWindow&  window,
        const std::string& etiqueta,
        HandleOrientacion  orientacion = HandleOrientacion::Horizontal,
        DespliegueDir      despliegue  = DespliegueDir::Abajo,
        sf::Color          colorBorde  = sf::Color::White
    );

    // ── contenido ── va directo al cuerpo ───────────────────────
    template <typename T, typename... Args>
    T* crearContenido(Args&&... args) {
        T* ptr = cuerpo.crearContenido<T>(std::forward<Args>(args)...);
        // recalcularTamanoCuerpo();
        recalcularPosCuerpo();
        return ptr;
    }

    // ── eventos ─────────────────────────────────────────────────
    void gestionarEvento(const sf::Event& evento);
    void update(sf::Vector2f mousePos, bool click);
    void draw();

    // ── estado ──────────────────────────────────────────────────
    bool estaAbierto() const { return abierto; }
    void abrir()  { abierto = true;  }
    void cerrar() { abierto = false; }

    // ── estilo ──────────────────────────────────────────────────
    void setColorFondo(sf::Color color);
    void setDegradado(sf::Color colorTop, sf::Color colorBot);
    void setBorderWidth(float grosor);    
    void setRadio(float r){ radioHandle = r; handle.setRadio(radioHandle);  }



    void positionAbsoluta(Ubicacion ubi){ handle.positionAbsoluta(ubi); }

    void configurarMedidas(float esp, float margen){ 
        handle.configurarMedidas(radioHandle, esp, margen); 
    }
         
};

}


#endif