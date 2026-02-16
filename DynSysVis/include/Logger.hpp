#ifndef DSV_LOGGER_HPP
#define DSV_LOGGER_HPP

#include <iostream>
#include <string>
#include <filesystem>

namespace DSV {

    class Logger {
    public:
        // categorias // segun son las tipicas
        enum class Level { INFO, SUCCESS, WARN, CRITIC };

        static void Log(Level level, const std::string& msg, const char* file, int line) {
            // nombre del archivo
            std::string fileName = std::filesystem::path(file).filename().string();
            
            const char* color;
            const char* label;

            switch (level) {
                case Level::INFO:    color = "\033[36m"; label = "[INFO]";    break; // Cian
                case Level::SUCCESS: color = "\033[32m"; label = "[OK  ]";    break; // Verde
                case Level::WARN:    color = "\033[33m"; label = "[WARN]";    break; // Amarillo
                case Level::CRITIC:  color = "\033[31m"; label = "[ERROR]";   break; // Rojo
            }

            //   [NIVEL] (archivo.cpp:72) -> Mensaje
            std::cout << color << label << "\033[0m " 
                      << "\033[90m(" << fileName << ":" << line << ")\033[0m "
                      << msg << std::endl;
        }
    };
}

// --- MACROS DE ACCESO (Aquí está el truco profesional) ---
#ifdef DSV_DEBUG
    #define DSV_LOG_INFO(m)    DSV::Logger::Log(DSV::Logger::Level::INFO,    m, __FILE__, __LINE__)
    #define DSV_LOG_SUCCESS(m) DSV::Logger::Log(DSV::Logger::Level::SUCCESS, m, __FILE__, __LINE__)
    #define DSV_LOG_WARN(m)    DSV::Logger::Log(DSV::Logger::Level::WARN,    m, __FILE__, __LINE__)
    #define DSV_LOG_ERROR(m)   DSV::Logger::Log(DSV::Logger::Level::CRITIC,  m, __FILE__, __LINE__)
#else
    // Si no estamos en modo debug, las macros no hacen nada (cero impacto en rendimiento)
    #define DSV_LOG_INFO(m)
    #define DSV_LOG_SUCCESS(m)
    #define DSV_LOG_WARN(m)
    #define DSV_LOG_ERROR(m)
#endif

#endif