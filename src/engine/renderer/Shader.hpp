#pragma once

#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <SDL_video.h>

#include "impl/GL.hpp"
#include <iostream>
#include <vector>

class Shader{
public:

#if DEBUG
    //Debug Meta data (set by the AssetManager)
    int timestamp;
    std::string vertex_path;
    std::string fragment_path;
#endif

    enum class Uniform_Type{
        Int,
        Float,
        Mat3,
        Mat4,
        Vec4,
        Texture,
    };

    struct Uniform{
        std::string name;
        int location_id;
        Uniform_Type type;

    };


    bool compiled = false;
    unsigned int program_id;
    std::vector<Uniform> uniforms;

    Shader(std::string vertex, std::string fragment);
    Shader(const char* vertex, const char* fragment);

    ~Shader();

    void init_uniform(std::string name, Uniform_Type type);

    void use();

    void set_uniform(std::string name, int value);
    void set_uniform(std::string name, float value);
    void set_uniform(std::string name, glm::mat3 value);
    void set_uniform(std::string name, glm::mat4 value);
    void set_uniform(std::string name, glm::vec4 value);
    void set_uniform(std::string name, unsigned int texture_id, unsigned int slot);

    Uniform find_uniform(std::string name);
    
    void recompile(std::string vertex_shader, std::string fragment_shader);


private:

    void _reinit_uniforms();

    bool _check_uniform(Uniform uniform, Uniform_Type correctType);

    bool _compile_shader(const char* vertex_shader, const char* fragment_shader);
    bool _compile_shader_part(GLenum type, const char* code, GLuint& shader);


};
