#ifndef CAMPO_HPP
#define CAMPO_HPP

#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include <cmath>


#include "dsv/core/Objeto.hpp"

#include "dsv/core/Objeto.hpp"
#include "dsv/graphics2D/Geometria.hpp"
#include "dsv/interface/Temas.hpp"

namespace dsv {

// ── constantes compartidas por todos los campos ─────────────────
namespace CampoMetrics {
    constexpr float CHAR_W     = 7.5f;   // ancho aprox por caracter a 13px
    constexpr float PAD        = 8.f;    // padding horizontal interno
    constexpr float ALTO_FILA  = 30.f;   // alto de una fila estandar
    constexpr float TAM_FUENTE = 13u;
    constexpr float RADIO      = 5.f;    // radio de esquinas de widgets
    constexpr float GROSOR_BORDE_NORMAL = 1.2f;
    constexpr float GROSOR_BORDE_HOVER  = 2.2f;
}

// ════════════════════════════════════════════════════════════════
//  BASE
// ════════════════════════════════════════════════════════════════
class Campo {
public:
    bool nuevoRenglon = false;

    virtual ~Campo() = default;

    virtual float getAncho() const = 0;
    virtual float getAlto()  const = 0;

    virtual void draw(sf::RenderWindow& window,
                      sf::Vector2f      pos,
                      float             altoFila,
                      const sf::Font&   fuente) = 0;

    virtual void onMouseMove   (sf::Vector2f mousePos) {}
    virtual void onMousePress  (sf::Vector2f mousePos) {}
    virtual void onMouseRelease(sf::Vector2f mousePos) {}

protected:
    // helper compartido: dibuja un sf::Text centrado verticalmente en la fila
    static void dibujarTexto(sf::RenderWindow& window,
                             const std::string& str,
                             sf::Vector2f       pos,
                             float              altoFila,
                             const sf::Font&    fuente,
                             sf::Color          color    = sf::Color(180,180,180),
                             unsigned int       tamano   = 13u)
    {
        sf::Text t;
        t.setFont(fuente);
        t.setString(str);
        t.setCharacterSize(tamano);
        t.setFillColor(color);
        t.setPosition(pos.x, pos.y + (altoFila - tamano) * 0.5f);
        window.draw(t);
    }
};

// ════════════════════════════════════════════════════════════════
//  TEXTO PLANO
// ════════════════════════════════════════════════════════════════
class CampoTexto : public Campo {
public:
    explicit CampoTexto(const std::string& texto,
                        sf::Color          color    = Color::gris_claro,
                        unsigned int       tamFuente = 13u)
        : texto(texto), color(color), tamFuente(tamFuente) {}

    void setColor(sf::Color c) { color = c; }

    float getAncho() const override {
        return texto.size() * CampoMetrics::CHAR_W + CampoMetrics::PAD * 2;
    }
    float getAlto() const override { return CampoMetrics::ALTO_FILA; }

    void draw(sf::RenderWindow& window, sf::Vector2f pos,
              float altoFila, const sf::Font& fuente) override
    {
        dibujarTexto(window, texto,
                     {pos.x + CampoMetrics::PAD, pos.y},
                     altoFila, fuente, color, tamFuente);
    }

private:
    std::string  texto;
    sf::Color    color;
    unsigned int tamFuente;
};

// ════════════════════════════════════════════════════════════════
//  VARIABLE  —  lector en tiempo real sobre float*
// ════════════════════════════════════════════════════════════════
class CampoVariable : public Campo {
public:
    CampoVariable(const std::string& nombre,
                  float*             valor,
                  bool               esEntero = false,
                  bool               esBool   = false,
                  sf::Color          colorNombre = Color::gris_claro,
                  sf::Color          colorValor  = Color::blanco)
        : nombre(nombre), valor(valor),
          esEntero(esEntero), esBool(esBool),
          colorNombre(colorNombre), colorValor(colorValor) {}

    void setColorNombre(sf::Color c) { colorNombre = c; }
    void setColorValor (sf::Color c) { colorValor  = c; }

    float getAncho() const override {
        return (nombre.size() + VALOR_CHARS) * CampoMetrics::CHAR_W
               + CampoMetrics::PAD * 3;
    }
    float getAlto() const override { return CampoMetrics::ALTO_FILA; }

    void draw(sf::RenderWindow& window, sf::Vector2f pos,
              float altoFila, const sf::Font& fuente) override
    {
        using namespace CampoMetrics;

        std::string strVal;
        if(esBool) {
            strVal = (*valor != 0.f) ? "True" : "False";
        } else if(esEntero) {
            strVal = std::to_string(static_cast<int>(*valor));
        } else {
            char buf[16];
            std::snprintf(buf, sizeof(buf), "%.3f", *valor);
            strVal = buf;
        }

        // nombre con dos puntos
        dibujarTexto(window, nombre ,
                     {pos.x + PAD, pos.y},
                     altoFila, fuente, colorNombre);

        // valor a la derecha del nombre
        float xVal = pos.x + PAD + (nombre.size() + 1) * CHAR_W + PAD;
        dibujarTexto(window, strVal,
                     {xVal, pos.y},
                     altoFila, fuente, colorValor);
    }

private:
    std::string nombre;
    float*      valor;
    bool        esEntero;
    bool        esBool;
    sf::Color   colorNombre;
    sf::Color   colorValor;

    static constexpr int VALOR_CHARS = 8;
};

// ════════════════════════════════════════════════════════════════
//  BARRA DE PROGRESO
//  usa RectanguloRedondeado para fondo y relleno
// ════════════════════════════════════════════════════════════════
class CampoBarra : public Campo {
public:
    CampoBarra(const std::string& nombre,
               float*             valor,
               float              minVal,
               float              maxVal,
               sf::Color          colorRelleno = Color::azul,
               sf::Color          colorFondo   = Color::fondo_panel,
               sf::Color          colorBorde   = Color::gris_oscuro,
               float              anchoFijo    = 120.f)
        : nombre(nombre), valor(valor),
          minVal(minVal), maxVal(maxVal),
          colorRelleno(colorRelleno),
          colorFondo(colorFondo),
          colorBorde(colorBorde),
          anchoFijo(anchoFijo)
    {
        regenerar();
    }

    void setColorRelleno(sf::Color c) { colorRelleno = c; }  // relleno cambia cada frame
    void setColorFondo  (sf::Color c) { colorFondo = c; regenerar(); }
    void setColorBorde  (sf::Color c) { colorBorde = c; regenerar(); }

    float getAncho() const override {
        return CampoMetrics::PAD
             + nombre.size() * CampoMetrics::CHAR_W
             + CampoMetrics::PAD
             + anchoFijo
             + CampoMetrics::PAD;
    }
    float getAlto() const override { return CampoMetrics::ALTO_FILA; }

    void draw(sf::RenderWindow& window, sf::Vector2f pos,
              float altoFila, const sf::Font& fuente) override
    {
        using namespace CampoMetrics;

        // nombre
        dibujarTexto(window, nombre ,
                     {pos.x + PAD, pos.y},
                     altoFila, fuente, Color::gris_claro);

        float xBarra = pos.x + PAD + nombre.size() * CHAR_W + PAD;
        float cy     = pos.y + altoFila * 0.5f;
        float yBarra = cy - ALTO_BARRA * 0.5f;

        sf::Transform tf;
        tf.translate(xBarra, yBarra);

        // fondo (pre-generado)
        marcoFondo.draw(window, tf);

        // relleno proporcional — se regenera solo si el color cambio
        float t_val = std::clamp((*valor - minVal) / (maxVal - minVal), 0.f, 1.f);
        float anchoRelleno = std::max(1.f, anchoFijo * t_val);

        // relleno: rectangulo redondeado escalado al valor actual
        // lo generamos cada frame porque el ancho cambia continuamente
        RectanguloRedondeado relleno;
        relleno.generar({anchoRelleno, ALTO_BARRA}, RADIO_BARRA,
                        colorRelleno, sf::Color::Transparent);
        relleno.drawBack(window, tf);
    }

private:
    void regenerar() {
        marcoFondo.generar({anchoFijo, ALTO_BARRA}, RADIO_BARRA,
                           colorFondo, colorBorde,
                           CampoMetrics::GROSOR_BORDE_NORMAL);
    }

    std::string        nombre;
    float*             valor;
    float              minVal, maxVal;
    sf::Color          colorRelleno;
    sf::Color          colorFondo;
    sf::Color          colorBorde;
    float              anchoFijo;
    RectanguloRedondeado marcoFondo;

    static constexpr float ALTO_BARRA  = 12.f;
    static constexpr float RADIO_BARRA = 4.f;
};


// ════════════════════════════════════════════════════════════════
//  TOGGLE
// ════════════════════════════════════════════════════════════════
class CampoToggleTexto : public Campo {
public:
    CampoToggleTexto(const std::string& etiqueta,
                     bool*              ptrEstado,
                     const std::string& txtOn,
                     const std::string& txtOff,
                     sf::Color          colorOn     = Color::azul,
                     sf::Color          colorOff    = Color::fondo_panel,
                     sf::Color          colorBorde  = Color::gris_oscuro)
        : nombre(etiqueta), estado(ptrEstado),
          labelOn(txtOn), labelOff(txtOff),
          colorOn(colorOn), colorOff(colorOff), colorBorde(colorBorde)
    {
        regenerar();
    }

    // setters — todos llaman regenerar()
    void setColorOn   (sf::Color c) { colorOn    = c; regenerar(); }
    void setColorOff  (sf::Color c) { colorOff   = c; regenerar(); }
    void setColorBorde(sf::Color c) { colorBorde = c; regenerar(); }

    float getAncho() const override {
        return CampoMetrics::PAD
             + nombre.size() * CampoMetrics::CHAR_W
             + CampoMetrics::PAD
             + ANCHO_TOGGLE + CampoMetrics::PAD;
    }
    float getAlto() const override { return CampoMetrics::ALTO_FILA; }

    void draw(sf::RenderWindow& window, sf::Vector2f pos,
              float altoFila, const sf::Font& fuente) override
    {
        posActual = pos;
        float xToggle = pos.x + CampoMetrics::PAD
                      + nombre.size() * CampoMetrics::CHAR_W
                      + CampoMetrics::PAD;
        float cy = pos.y + altoFila * 0.5f;

        dibujarTexto(window, nombre,
                     {pos.x + CampoMetrics::PAD, pos.y},
                     altoFila, fuente, Color::gris_claro);

        sf::Transform tf;
        tf.translate(xToggle, cy - ALTO_TOGGLE * 0.5f);

        // misma logica que el boton: press > hover > estado
        if      (*estado && enHover) marcoOnHover .draw(window, tf);
        else if (*estado)            marcoOn      .draw(window, tf);
        else if (enHover)            marcoOffHover.draw(window, tf);
        else                         marcoOff     .draw(window, tf);

        // texto centrado dentro del toggle
        sf::Text t;
        t.setFont(fuente);
        t.setCharacterSize(11u);
        t.setFillColor(sf::Color::White);
        t.setString(*estado ? labelOn : labelOff);
        auto b = t.getLocalBounds();
        t.setOrigin(b.width * 0.5f, b.height * 0.5f);
        t.setPosition(xToggle + ANCHO_TOGGLE * 0.5f, cy - 1.f);
        window.draw(t);
    }

    void onMouseMove (sf::Vector2f mp) override {
        bool h = hitTest(mp);
        if(h != enHover) { enHover = h; }  // solo regeneramos si cambia
    }
    void onMousePress(sf::Vector2f mp) override {
        if(hitTest(mp)) *estado = !(*estado);
    }

private:
    bool hitTest(sf::Vector2f mp) const {
        float x = posActual.x + CampoMetrics::PAD
                + nombre.size() * CampoMetrics::CHAR_W
                + CampoMetrics::PAD;
        return sf::FloatRect(x, posActual.y,
                             ANCHO_TOGGLE,
                             CampoMetrics::ALTO_FILA).contains(mp);
    }

    void regenerar() {
        sf::Vector2f sz   = { ANCHO_TOGGLE, ALTO_TOGGLE };
        float        r    = 4.f;

        // ON normal
        marcoOn.generar(sz, r,
            colorOn,
            aclarar(colorOn, 40),        // borde mas claro que el fondo
            CampoMetrics::GROSOR_BORDE_NORMAL);

        // ON + hover: borde blanco y fondo un poco mas claro
        marcoOnHover.generar(sz, r,
            aclarar(colorOn, 25),
            Color::blanco,
            CampoMetrics::GROSOR_BORDE_HOVER);

        // OFF normal
        marcoOff.generar(sz, r,
            colorOff,
            colorBorde,
            CampoMetrics::GROSOR_BORDE_NORMAL);

        // OFF + hover: mismo fondo, borde blanco y mas grueso
        marcoOffHover.generar(sz, r,
            aclarar(colorOff, 20),
            Color::blanco,
            CampoMetrics::GROSOR_BORDE_HOVER);
    }

    // helper: aclara un color sumando delta a RGB
    static sf::Color aclarar(sf::Color c, int delta) {
        return sf::Color(
            static_cast<sf::Uint8>(std::min(255, c.r + delta)),
            static_cast<sf::Uint8>(std::min(255, c.g + delta)),
            static_cast<sf::Uint8>(std::min(255, c.b + delta)),
            c.a
        );
    }

    std::string  nombre;
    bool*        estado;
    std::string  labelOn, labelOff;
    sf::Color    colorOn, colorOff, colorBorde;
    sf::Vector2f posActual;
    bool         enHover = false;

    // cuatro marcos: on, on+hover, off, off+hover
    RectanguloRedondeado marcoOn;
    RectanguloRedondeado marcoOnHover;
    RectanguloRedondeado marcoOff;
    RectanguloRedondeado marcoOffHover;

    static constexpr float ANCHO_TOGGLE = 55.f;
    static constexpr float ALTO_TOGGLE  = 18.f;
};



// ════════════════════════════════════════════════════════════════
//  TOGGLE TEXTO
// ════════════════════════════════════════════════════════════════
class CampoToggle : public Campo {
public:
    CampoToggle(const std::string& etiqueta,
                bool*              ptrEstado,
                sf::Color          colorOn    = Color::verde,
                sf::Color          colorOff   = Color::fondo_panel,
                sf::Color          colorBorde = Color::gris_oscuro)
        : nombre(etiqueta), estado(ptrEstado),
          colorOn(colorOn), colorOff(colorOff), colorBorde(colorBorde)
    {
        regenerar();
    }

    void setColorOn   (sf::Color c) { colorOn    = c; regenerar(); }
    void setColorOff  (sf::Color c) { colorOff   = c; regenerar(); }
    void setColorBorde(sf::Color c) { colorBorde = c; regenerar(); }

    float getAncho() const override {
        return CampoMetrics::PAD
             + nombre.size() * CampoMetrics::CHAR_W
             + CampoMetrics::PAD
             + ANCHO_SWITCH + CampoMetrics::PAD;
    }
    float getAlto() const override { return CampoMetrics::ALTO_FILA; }

    void draw(sf::RenderWindow& window, sf::Vector2f pos,
              float altoFila, const sf::Font& fuente) override
    {
        posActual = pos;
        float xSw = pos.x + CampoMetrics::PAD
                  + nombre.size() * CampoMetrics::CHAR_W
                  + CampoMetrics::PAD;
        float cy = pos.y + altoFila * 0.5f;

        dibujarTexto(window, nombre,
                     {pos.x + CampoMetrics::PAD, pos.y},
                     altoFila, fuente, Color::gris_claro);

        sf::Transform tf;
        tf.translate(xSw, cy - ALTO_SWITCH * 0.5f);

        // capsula de fondo — cuatro estados igual que ToggleTexto
        if      (*estado && enHover) marcoOnHover .draw(window, tf);
        else if (*estado)            marcoOn      .draw(window, tf);
        else if (enHover)            marcoOffHover.draw(window, tf);
        else                         marcoOff     .draw(window, tf);

        // circulo deslizante
        // cuando esta ON va a la derecha, OFF a la izquierda
        float radioCirculo = ALTO_SWITCH * 0.38f;
        float xOff = xSw + radioCirculo + 3.f;
        float xOn  = xSw + ANCHO_SWITCH - radioCirculo - 3.f;
        float xCirculo = *estado ? xOn : xOff;

        // el circulo usa el generador que ya tenemos
        float diametro = radioCirculo * 2.f;
        sf::Color colorCirculo = enHover ? Color::gris_claro : Color::blanco;
        auto circulo = generarRectanguloRelleno(
            {diametro, diametro},
            radioCirculo,   // radio = la mitad = circulo perfecto
            24,
            colorCirculo
        );

        sf::Transform tfCirculo;
        tfCirculo.translate(xCirculo - radioCirculo, cy - radioCirculo);
        window.draw(circulo, tfCirculo);
    }

    void onMouseMove (sf::Vector2f mp) override {
        bool h = hitTest(mp);
        if(h != enHover) { enHover = h; }
    }
    void onMousePress(sf::Vector2f mp) override {
        if(hitTest(mp)) *estado = !(*estado);
    }

private:
    bool hitTest(sf::Vector2f mp) const {
        float x = posActual.x + CampoMetrics::PAD
                + nombre.size() * CampoMetrics::CHAR_W
                + CampoMetrics::PAD;
        return sf::FloatRect(x, posActual.y,
                             ANCHO_SWITCH,
                             CampoMetrics::ALTO_FILA).contains(mp);
    }

    void regenerar() {
        float        radioMax = ALTO_SWITCH * 0.5f;  // capsula perfecta
        sf::Vector2f sz       = { ANCHO_SWITCH, ALTO_SWITCH };

        marcoOn.generar(sz, radioMax,
            colorOn,
            aclarar(colorOn, 40),
            CampoMetrics::GROSOR_BORDE_NORMAL);

        marcoOnHover.generar(sz, radioMax,
            aclarar(colorOn, 25),
            Color::blanco,
            CampoMetrics::GROSOR_BORDE_HOVER);

        marcoOff.generar(sz, radioMax,
            colorOff,
            colorBorde,
            CampoMetrics::GROSOR_BORDE_NORMAL);

        marcoOffHover.generar(sz, radioMax,
            aclarar(colorOff, 20),
            Color::blanco,
            CampoMetrics::GROSOR_BORDE_HOVER);
    }

    static sf::Color aclarar(sf::Color c, int delta) {
        return sf::Color(
            static_cast<sf::Uint8>(std::min(255, c.r + delta)),
            static_cast<sf::Uint8>(std::min(255, c.g + delta)),
            static_cast<sf::Uint8>(std::min(255, c.b + delta)),
            c.a
        );
    }

    std::string  nombre;
    bool*        estado;
    sf::Color    colorOn, colorOff, colorBorde;
    sf::Vector2f posActual;
    bool         enHover = false;

    RectanguloRedondeado marcoOn;
    RectanguloRedondeado marcoOnHover;
    RectanguloRedondeado marcoOff;
    RectanguloRedondeado marcoOffHover;

    static constexpr float ANCHO_SWITCH = 36.f;
    static constexpr float ALTO_SWITCH  = 18.f;
};

// ════════════════════════════════════════════════════════════════
//  BOTON
//  tres marcos pre-generados: normal, hover, presionado
// ════════════════════════════════════════════════════════════════
class CampoBoton : public Campo {
public:
    // boton simple
    CampoBoton(const std::string&    texto,
               std::function<void()> accion,
               sf::Color             colorFondo  = Color::fondo_panel,
               sf::Color             colorBorde  = Color::gris_oscuro,
               float                 anchoFijo   = 90.f)
        : textoA(texto), textoB(""),
          accion(accion), getEstado(nullptr),
          colorFondo(colorFondo), colorBorde(colorBorde),
          anchoFijo(anchoFijo)
    {
        regenerar();
    }

    // boton play/pausa
    CampoBoton(const std::string&    textoA,
               const std::string&    textoB,
               std::function<void()> accion,
               std::function<bool()> getEstado,
               sf::Color             colorFondo  = Color::fondo_panel,
               sf::Color             colorBorde  = Color::gris_oscuro,
               float                 anchoFijo   = 90.f)
        : textoA(textoA), textoB(textoB),
          accion(accion), getEstado(getEstado),
          colorFondo(colorFondo), colorBorde(colorBorde),
          anchoFijo(anchoFijo)
    {
        regenerar();
    }

    void setColorFondo (sf::Color c) { colorFondo  = c; regenerar(); }
    void setColorBorde (sf::Color c) { colorBorde  = c; regenerar(); }
    void setColorHover (sf::Color c) { colorHover  = c; regenerar(); }
    void setColorPress (sf::Color c) { colorPress  = c; regenerar(); }

    float getAncho() const override { return anchoFijo + CampoMetrics::PAD * 2; }
    float getAlto()  const override { return CampoMetrics::ALTO_FILA; }

    void draw(sf::RenderWindow& window, sf::Vector2f pos,
              float altoFila, const sf::Font& fuente) override
    {
        posActual = pos;
        float cy  = pos.y + altoFila * 0.5f;
        float yBtn = cy - ALTO_BTN * 0.5f;

        sf::Transform tf;
        tf.translate(pos.x + CampoMetrics::PAD, yBtn);

        // elegir marco
        if(presionado)     marcoPress.draw(window, tf);
        else if(enHover)   marcoHover.draw(window, tf);
        else               marcoNormal.draw(window, tf);

        // label centrado
        std::string label = (getEstado && getEstado()) ? textoB : textoA;
        sf::Text t;
        t.setFont(fuente);
        t.setCharacterSize(12u);
        t.setFillColor(sf::Color::White);
        t.setString(label);
        auto bounds = t.getLocalBounds();
        t.setPosition(
            pos.x + CampoMetrics::PAD + (anchoFijo - bounds.width)  * 0.5f,
            cy - bounds.height * 0.5f - 2.f
        );
        window.draw(t);
    }

    void onMouseMove   (sf::Vector2f mp) override {
        bool h = hitTest(mp);
        if(h != enHover) { enHover = h; }
    }
    void onMousePress  (sf::Vector2f mp) override {
        if(hitTest(mp)) presionado = true;
    }
    void onMouseRelease(sf::Vector2f mp) override {
        if(presionado && hitTest(mp)) accion();
        presionado = false;
    }

private:
    bool hitTest(sf::Vector2f mp) const {
        float cy = posActual.y + CampoMetrics::ALTO_FILA * 0.5f;
        return mp.x >= posActual.x + CampoMetrics::PAD
            && mp.x <= posActual.x + CampoMetrics::PAD + anchoFijo
            && mp.y >= cy - ALTO_BTN * 0.5f
            && mp.y <= cy + ALTO_BTN * 0.5f;
    }

    void regenerar() {
        sf::Vector2f sz = { anchoFijo, ALTO_BTN };
        float r = CampoMetrics::RADIO;

        // hover es el fondo un poco mas claro
        colorHover = sf::Color(
            std::min(255, colorFondo.r + 30),
            std::min(255, colorFondo.g + 30),
            std::min(255, colorFondo.b + 40)
        );
        // press es mas oscuro
        colorPress = sf::Color(
            std::max(0, colorFondo.r - 20),
            std::max(0, colorFondo.g - 20),
            std::max(0, colorFondo.b - 20)
        );
        sf::Color bordeHover = Color::gris_claro;

        marcoNormal.generar(sz, r, colorFondo,
                            colorBorde,  CampoMetrics::GROSOR_BORDE_NORMAL);
        marcoHover .generar(sz, r, colorHover,
                            bordeHover,  CampoMetrics::GROSOR_BORDE_HOVER);
        marcoPress .generar(sz, r, colorPress,
                            colorBorde,  CampoMetrics::GROSOR_BORDE_NORMAL);
    }

    std::string           textoA, textoB;
    std::function<void()> accion;
    std::function<bool()> getEstado;
    sf::Color             colorFondo, colorBorde;
    sf::Color             colorHover, colorPress;  // calculados en regenerar()
    float                 anchoFijo;
    sf::Vector2f          posActual;
    bool                  enHover   = false;
    bool                  presionado = false;

    RectanguloRedondeado  marcoNormal;
    RectanguloRedondeado  marcoHover;
    RectanguloRedondeado  marcoPress;

    static constexpr float ALTO_BTN = 20.f;
};

// ════════════════════════════════════════════════════════════════
//  DESLIZADOR
//  track con RectanguloRedondeado, thumb con circulo via genrador
// ════════════════════════════════════════════════════════════════
class CampoDeslizador : public Campo {
public:
    CampoDeslizador(const std::string& nombre,
                    float*             valor,
                    float              minVal,
                    float              maxVal,
                    sf::Color          colorTrack  = Color::fondo_panel,
                    sf::Color          colorRelleno= Color::azul,
                    sf::Color          colorThumb  = Color::azul_l,
                    float              anchoFijo   = 120.f)
        : nombre(nombre), valor(valor),
          minVal(minVal), maxVal(maxVal),
          colorTrack(colorTrack),
          colorRelleno(colorRelleno),
          colorThumb(colorThumb),
          anchoFijo(anchoFijo)
    {
        regenerar();
    }

    void setColorTrack  (sf::Color c) { colorTrack   = c; regenerar(); }
    void setColorRelleno(sf::Color c) { colorRelleno = c; }   // cambia cada frame
    void setColorThumb  (sf::Color c) { colorThumb   = c; regenerar(); }

    float getAncho() const override {
        return CampoMetrics::PAD
             + nombre.size() * CampoMetrics::CHAR_W
             + CampoMetrics::PAD
             + anchoFijo
             + ANCHO_VALOR_TEXT
             + CampoMetrics::PAD;
    }
    float getAlto() const override { return CampoMetrics::ALTO_FILA; }

    void draw(sf::RenderWindow& window, sf::Vector2f pos,
              float altoFila, const sf::Font& fuente) override
    {
        using namespace CampoMetrics;
        posActual  = pos;
        altoActual = altoFila;

        float cy     = pos.y + altoFila * 0.5f;
        float xTrack = xTrackAbs();
        float yTrack = cy - ALTO_TRACK * 0.5f;

        // nombre
        dibujarTexto(window, nombre + " ",
                     {pos.x + PAD, pos.y},
                     altoFila, fuente, Color::gris_claro);

        sf::Transform tfTrack;
        tfTrack.translate(xTrack, yTrack);

        // fondo del track (pre-generado)
        marcoTrack.draw(window, tfTrack);

        // relleno proporcional
        float t_val = std::clamp((*valor - minVal) / (maxVal - minVal), 0.f, 1.f);
        float anchoRelleno = std::max(1.f, anchoFijo * t_val);

        RectanguloRedondeado relleno;
        relleno.generar({anchoRelleno, ALTO_TRACK}, RADIO_TRACK,
                        colorRelleno, sf::Color::Transparent);
        relleno.drawBack(window, tfTrack);

        // thumb — circulo usando VertexArray de TriangleFan
        float xThumb = xTrack + anchoFijo * t_val;
        sf::Color cThumb = (arrastrando || enHover) ? Color::blanco : colorThumb;
        auto thumbVA = generarRectanguloRelleno(
            {RADIO_THUMB * 2.f, RADIO_THUMB * 2.f},
            RADIO_THUMB, 24, cThumb);

        sf::Transform tfThumb;
        tfThumb.translate(xThumb - RADIO_THUMB, cy - RADIO_THUMB);
        window.draw(thumbVA, tfThumb);

        // valor numerico
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%.2f", *valor);
        dibujarTexto(window, buf,
                     {xTrack + anchoFijo + 4.f, pos.y},
                     altoFila, fuente, Color::gris, 11u);
    }

    void onMouseMove(sf::Vector2f mp) override {
        enHover = hitTestThumb(mp);
        if(arrastrando) actualizarValor(mp.x);
    }
    void onMousePress(sf::Vector2f mp) override {
        if(hitTestTrack(mp)) {
            arrastrando = true;
            actualizarValor(mp.x);
        }
    }
    void onMouseRelease(sf::Vector2f mp) override {
        arrastrando = false;
    }

private:
    float xTrackAbs() const {
        return posActual.x + CampoMetrics::PAD
             + nombre.size() * CampoMetrics::CHAR_W
             + CampoMetrics::PAD;
    }

    bool hitTestTrack(sf::Vector2f mp) const {
        float cy = posActual.y + altoActual * 0.5f;
        float xt = xTrackAbs();
        return mp.x >= xt && mp.x <= xt + anchoFijo
            && mp.y >= cy - RADIO_THUMB && mp.y <= cy + RADIO_THUMB;
    }
    bool hitTestThumb(sf::Vector2f mp) const {
        float t_val = std::clamp((*valor - minVal) / (maxVal - minVal), 0.f, 1.f);
        float cx = xTrackAbs() + anchoFijo * t_val;
        float cy = posActual.y + altoActual * 0.5f;
        float dx = mp.x - cx, dy = mp.y - cy;
        return (dx*dx + dy*dy) <= (RADIO_THUMB * RADIO_THUMB * 3.f);
    }
    void actualizarValor(float mouseX) {
        float t = std::clamp((mouseX - xTrackAbs()) / anchoFijo, 0.f, 1.f);
        *valor = minVal + t * (maxVal - minVal);
    }

    void regenerar() {
        marcoTrack.generar({anchoFijo, ALTO_TRACK}, RADIO_TRACK,
                           colorTrack, Color::gris_oscuro,
                           CampoMetrics::GROSOR_BORDE_NORMAL);
    }

    std::string          nombre;
    float*               valor;
    float                minVal, maxVal;
    sf::Color            colorTrack, colorRelleno, colorThumb;
    float                anchoFijo;
    sf::Vector2f         posActual;
    float                altoActual = 30.f;
    bool                 enHover    = false;
    bool                 arrastrando = false;

    RectanguloRedondeado marcoTrack;

    static constexpr float ALTO_TRACK      = 6.f;
    static constexpr float RADIO_TRACK     = 3.f;
    static constexpr float RADIO_THUMB     = 6.f;
    static constexpr float ANCHO_VALOR_TEXT = 40.f;
};

} // namespace dsv
#endif // CAMPO_HPP