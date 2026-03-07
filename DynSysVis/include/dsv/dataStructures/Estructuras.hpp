
#ifndef ESTRUCTURAS_HPP
#define ESTRUCTURAS_HPP

#include <iostream>
#include <deque>
#include <stdexcept>
#include <SFML/Graphics.hpp>


namespace dsv{ 


/**
 * @brief MonotonicMaxQueue
 * que hace: matiene el orden FIFO original 
 *          maximo en O(1) push_back O(1) erase_begin(1)
 */
template <typename T>
class MonotonicQueue {
private:
    std::deque<T> data;
    std::deque<T> max_deck;
    std::deque<T> min_deck; // <--- minimos

public:
    void push(const T& val) {
        data.push_back(val);
        // para maximo
        while (!max_deck.empty() && max_deck.back() < val) max_deck.pop_back();
        max_deck.push_back(val);
        
        // alreves
        while (!min_deck.empty() && min_deck.back() > val) min_deck.pop_back();
        min_deck.push_back(val);
    }

 
    void pop(){
        if( data.empty()) return;
        if( data.front() == max_deck.front()) max_deck.pop_front();
        if( data.front() == min_deck.front()) min_deck.pop_front();
        data.pop_front();
    }
    // GETs
    T back(){ return data.back();  }
    T front(){ return data.front();  }
    T get_max() const { return max_deck.empty() ? 0 : max_deck.front(); }
    T get_min() const { return min_deck.empty() ? 0 : min_deck.front(); }
    
    // clasicos
    size_t size() const { return data.size(); }
    bool empty() const { return data.empty(); }
    const T& operator[](size_t index) const{ return data[index]; }
    void clear() {
        data.clear();
        max_deck.clear();
        min_deck.clear();
    }
};



/**
 * --- DESTINADO A USAR EN GRAFICAS ---
 * Usa lo anteior para hacer 2d, se usara mucho
 * con acceso [] devuelve sf::Vector2f
*/

struct Limites {
    float minX, maxX, minY, maxY;
};

void inline ajustarLimites_EvitarDivisionCero(Limites& lim) {
    const float eps = std::numeric_limits<float>::epsilon() * 100.0f;
    // configurar el ajsute
    auto ajustar = [eps](float& minVal, float& maxVal) {
        float delta = maxVal - minVal;
        float maxAbs = std::max(std::abs(minVal), std::abs(maxVal));
      
        float eps_relativo = std::max(maxAbs * eps, 1e-37f); 

        if( delta <= eps_relativo ){
            float offset = (maxAbs > eps_relativo) ? maxAbs * 0.1f : 1.0f;
            minVal -= offset;
            maxVal += offset;
        }
    };
    // ahora si ajustar cada uno
    ajustar(lim.minX, lim.maxX);
    ajustar(lim.minY, lim.maxY);
}

template <typename T>
class MonotonicQueue2D {
private:
    MonotonicQueue<T> qX;
    MonotonicQueue<T> qY;

public:
    void push(T x, T y) {
        qX.push(x);
        qY.push(y);
    }

    void pop() {
        if( empty() ) return;
        qX.pop();
        qY.pop();
    }


    // Gets
    Limites getLimites() const {
        return { qX.get_min(), qX.get_max(), qY.get_min(), qY.get_max() };
    }
    sf::Vector2f back(){
        return sf::Vector2f( static_cast<float>(qX.back()), static_cast<float>(qY.back()) );
    }
    sf::Vector2f front(){
        return sf::Vector2f( static_cast<float>(qX.front()), static_cast<float>(qY.front()) );
    }

    // clasicos
    size_t size() const { return qX.size(); }
    bool empty() const { return qX.empty(); }
    sf::Vector2f operator[](size_t i) const {
        return sf::Vector2f(static_cast<float>(qX[i]), static_cast<float>(qY[i]));
    }
    void clear() {
        qX.clear();
        qY.clear();
    }
};


/**
 * --- DESTINADO A USAR EN 3D ---
 * Usa lo anteior para hacer 2d, se usara mucho
 * con acceso [] devuelve sf::Vector2f
*/
struct Limites3D {
    float minX, maxX, minY, maxY, minZ, maxZ;
};
template <typename T>
class MonotonicQueue3D {
private:
    MonotonicQueue<T> qX;
    MonotonicQueue<T> qY;
    MonotonicQueue<T> qZ;

public:
    void push(T x, T y, T z) {
        qX.push(x);
        qY.push(y);
        qZ.push(z);
    }

    void pop(){
        if( empty() ) return;
        qX.pop();
        qY.pop();
        qZ.pop();
    }
    
    // Gets
    sf::Vector3f operator[](size_t i) const {
        return sf::Vector3f(
            static_cast<float>(qX[i]), 
            static_cast<float>(qY[i]), 
            static_cast<float>(qZ[i])
        );
    }
    sf::Vector3f back() const {
        return sf::Vector3f(
            static_cast<float>(qX.back()), 
            static_cast<float>(qY.back()), 
            static_cast<float>(qZ.back())
        );
    }
    sf::Vector3f front(){
        return sf::Vector3f(
            static_cast<float>(qX.front()), 
            static_cast<float>(qY.front()), 
            static_cast<float>(qZ.front())
        );
    }

    // clasicos
    size_t size() const{ return qX.size(); }
    bool empty() const{ return qX.empty(); }
    void clear() {
        qX.clear();
        qY.clear();
        qZ.clear();
    }

    
};


// termina dsv
}
#endif