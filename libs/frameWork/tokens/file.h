//
// Created by dioguabo-rei-delas on 8/26/25.
//

#ifndef CINDRA_FILE_H
#define CINDRA_FILE_H
#include <filesystem>
#include <fstream>
#include <string>

using std::ifstream;
namespace fs = std::filesystem;
namespace cid::help {
    inline bool isValidExtension(const std::filesystem::path& path) {
        static constexpr auto allow = {".cnd", ".cd", ".cindra"};
        auto ext = path.extension().string();
        for (const auto& e : allow) {
            if (ext == e) return true;
        }
        return false;
    }

    inline void extrFile(ifstream& file, string& buffer) {
        file.seekg(0, std::ios::end);
        buffer.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(&buffer[0], static_cast<long>(buffer.size()));
        file.close();
    }

    inline auto openFile(const int argc, const char** argv) {
        if (argc <= 1) { // Corrigido para verificar argc <= 1
            throw std::runtime_error("Wrong usage: you should pass a source file as program argument");
        }

        const fs::path src = argv[1]; // Agora seguro

        if (!fs::exists(src)) {
            throw std::runtime_error("File does not exist");
        }

        if (!isValidExtension(src)) {
            throw std::runtime_error("File is not a valid extension");
        }

        ifstream file(src, std::ios::binary);
        if (!file) {
            throw std::runtime_error("File could not be opened");
        }

        string buffer;
        extrFile(file, buffer);
        return buffer;
    }
}
#endif //CINDRA_FILE_H