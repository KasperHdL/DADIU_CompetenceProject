#pragma once

#include "SDL.h"
#include <iostream>
#include <string>
#include <fstream>

class DataHandler{
public:

    static std::string get_path(const std::string &file = ""){

#ifdef _WIN32
        const char PATH_SEP = '\\';
#else
        const char PATH_SEP = '/';
#endif

        static std::string baseRes;

        if (baseRes.empty()){

            char *basePath = SDL_GetBasePath();

            if (basePath){
                baseRes = basePath;
                SDL_free(basePath);
            }
            else {
                std::cerr << "Error getting resource path: " << SDL_GetError() << std::endl;
                return "";
            }

#if DEBUG
            baseRes = baseRes + ".." + PATH_SEP + "data" + PATH_SEP;
#else
            baseRes = baseRes + "data" + PATH_SEP;
#endif
        }

        return file.empty() ? baseRes: baseRes + file;
    }

    static std::string load_file_as_string(string filename){
        std::ifstream file(get_path(filename));

        std::string vert((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
        file.close();

        return vert;
    }


};
