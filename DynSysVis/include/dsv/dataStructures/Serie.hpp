/*
    CONTIENE Serie
    * puede ser de cualquier dimension
        -> una vez elegida la dimension al principio, esta se mantiene fija
    * optimiza los datos para tenerlos en memoria continua
    * permite
        -> agregar al final
        -> mantener simepre maxPoints boroando al principio
*/

#ifndef SERIE_HPP
#define SERIE_HPP

//  sfml y std
#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>

#include "dsv/core/Logger.hpp"

namespace dsv{

struct Serie {
    std::vector<float> buffer; 
    int dimension = 3;
    size_t maxPoints = 10000;
    
    // punteros para mantener ciruclar
    size_t head = 0;           // aqui escribirmeos el sigueinte dato
    size_t count = 0;          // cuantos puntos reales tenemos hasta llegar a maxPoints

    // dibujado
    std::vector<sf::Color> paleta = {};
    float grosor = 2.0f;
    bool difuminarCola = false; 
    bool adelgazarCola = false; 

    // --- --- --- --- cconfiguraciones --- --- --- ---

    // Constructor unico que sirve como "por defecto" (Cyan) y como "por color"
    Serie() = default;
    // Constructor por paleta (para degradados)
    Serie(const std::vector<sf::Color>& paleta) : paleta(paleta) {}

    void configurar(int dim, size_t maxP){
        dimension = dim;
        maxPoints = maxP;
        buffer.assign(dimension * maxPoints, 0.0f);
        head = 0;
        count = 0;
    }


    // --- --- --- --- push back --- --- --- ---

    // O(1) y manitene solo los maxPoints recientes
    void push_back( const std::vector<float>& valores ){
        if( buffer.empty() ){
            DSV_LOG_ERROR("Olvidaste llamar a configurar(int dim, size_t maxP)");
            return; 
        }
        
        size_t offset = head * dimension;
        for( int i = 0; i < dimension; i++ ){
            int val = static_cast<int>(valores.size());
            buffer[offset + i] = (i < val) ? valores[i] : 0.0f;
        }

        head = (head + 1) % maxPoints;
        if( count < maxPoints) count++;
    }
    
    // --- --- --- --- colores --- --- --- --- 
    void setColor(sf::Color col) {
        paleta = {col};
    }

    void setColor(std::vector<sf::Color> col) {
        paleta = col;
    }


    // Acceso rapido para el draw si solo hay un color
    sf::Color getColorPrincipal() const {
        return paleta.empty() ? sf::Color::White : paleta[0];
    }


    // --- --- --- --- acceder a memoria --- --- --- ---

    // i va de 0 (mas viejo) a count-1 (mas nuevo)
    inline size_t getIndiceReal(size_t i) const {
        if( count < maxPoints) return i;
        return (head + i) % maxPoints;
    }

    /*
        para obtener los floats directamente
        devuelve puntero a donde apunta la memoria de los n datos del indice i
        osea un puntero a  x_i y_i z_i ... w_i
    */
    // Retorna puntero normal + Funcion normal
    // despues de calcular un punto, quieres aplicar un filtro o corregir un valor manualmente. Necesitas un puntero que te permita escribir en la memoria.
    inline float* getPointer(size_t i) { 
        return &buffer[getIndiceReal(i) * dimension]; 
    }

    // Retorna puntero constante + Funcion constante
    // permite que el draw lea los puntos para proyectarlos sin que nada cambie
    inline const float* getPointer(size_t i) const { 
        return &buffer[getIndiceReal(i) * dimension]; 
    }
    

    // --- --- --- --- GETs de accesro general y sf::Vector3f --- --- --- ---

    // Operador [] para leer como si fuera un vector normal
    // Uso: std::vector<float> p = serie[5];
    std::vector<float> operator[](size_t i) const {
        if( i >= count) return {}; // O lanzar un error

        size_t realIdx = getIndiceReal(i) * dimension;
        std::vector<float> punto(dimension);
        
        for (int d = 0; d < dimension; ++d ){ 
            punto[d] = buffer[realIdx + d];
        }
        
        return punto;
    }
    sf::Vector3f getPunto3D(size_t i) const {
        size_t realIdx = getIndiceReal(i) * dimension;
        return { 
            buffer[realIdx], 
            (dimension > 1 ? buffer[realIdx + 1] : 0.f), 
            (dimension > 2 ? buffer[realIdx + 2] : 0.f) 
        };
    }
    
    sf::Vector2f getPunto2D(size_t i) const {
        size_t realIdx = getIndiceReal(i) * dimension;
        return { 
            buffer[realIdx], 
            (dimension > 1 ? buffer[realIdx + 1] : 0.f)
        };
    }

    float getPunto1D(size_t i) const {
        size_t realIdx = getIndiceReal(i) * dimension;
        return buffer[realIdx];
    }


    // --- --- reajustar --- ---
    void setMaxPoints(size_t nuevoMax ){ 
        if( nuevoMax == maxPoints) return;

        std::vector<float> nuevoBuffer(nuevoMax * dimension, 0.0f);
        size_t puntosARescatar = std::min(count, nuevoMax);
        
        for (size_t i = 0; i < puntosARescatar; i++ ){ 
            size_t idxViejo = getIndiceReal(count - puntosARescatar + i) * dimension;
            size_t idxNuevo = i * dimension;
            for (int d = 0; d < dimension; ++d ){ 
                nuevoBuffer[idxNuevo + d] = buffer[idxViejo + d];
            }
        }

        buffer = std::move(nuevoBuffer);
        maxPoints = nuevoMax;
        count = puntosARescatar;
        head = (count == maxPoints) ? 0 : count;
    }
};



} // dsv
#endif // #ifndef