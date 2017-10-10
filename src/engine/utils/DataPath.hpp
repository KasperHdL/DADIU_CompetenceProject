#pragma once

#include "SDL.h"
#include <iostream>
#include <string>

class DataPath{
public:

    static std::string get(const std::string &file = ""){

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

};
