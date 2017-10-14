#include <engine/renderer/Shader.hpp>

Shader::Shader(std::string vertex, std::string fragment){
   compiled = _compile_shader(vertex.c_str(), fragment.c_str());

   if(!compiled)
       glDeleteShader(program_id);

}

Shader::Shader(const char* vertex, const char* fragment){
   compiled = _compile_shader(vertex, fragment);

   if(!compiled)
       glDeleteShader(program_id);
}

Shader::~Shader(){
    glDeleteShader(program_id);
}

void Shader::init_uniform(std::string name, Uniform_Type type){
    Shader::Uniform u;
    u.name = name;
    u.location_id = glGetUniformLocation(program_id, name.c_str());
    u.type = type;
    uniforms.push_back(u);
}

void Shader::use(){
    glUseProgram(program_id);
}

void Shader::set_uniform(std::string name, int value){
    Shader::Uniform u = find_uniform(name);
    if(_check_uniform(u, Uniform_Type::Int) == false) return;
    glUniform1i(u.location_id, value);
}
void Shader::set_uniform(std::string name, float value){
    Shader::Uniform u = find_uniform(name);
    if(_check_uniform(u, Uniform_Type::Float) == false) return;
    glUniform1f(u.location_id, value);
}
void Shader::set_uniform(std::string name, glm::mat3 value){
    Shader::Uniform u = find_uniform(name);
    if(_check_uniform(u, Uniform_Type::Mat3) == false) return;
    glUniformMatrix3fv(u.location_id, 1, GL_FALSE, glm::value_ptr(value));
}
void Shader::set_uniform(std::string name, glm::mat4 value){
    Shader::Uniform u = find_uniform(name);
    if(_check_uniform(u, Uniform_Type::Mat4) == false) return;
    glUniformMatrix4fv(u.location_id, 1, GL_FALSE, glm::value_ptr(value));
}
void Shader::set_uniform(std::string name, glm::vec4 value){
    Shader::Uniform u = find_uniform(name);
    if(_check_uniform(u, Uniform_Type::Vec4) == false) return;
    glUniform4fv(u.location_id, 1, glm::value_ptr(value));
}
void Shader::set_uniform(std::string name, unsigned int texture_id, unsigned int slot){
    Shader::Uniform u = find_uniform(name);
    if(_check_uniform(u, Uniform_Type::Texture) == false) return;

    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glUniform1i(u.location_id, slot);
}


Shader::Uniform Shader::find_uniform(std::string name){
    for (auto i = uniforms.begin(); i != uniforms.end(); i++) {
        if (name == i->name)
            return *i;
    }
    Shader::Uniform u;
    u.location_id = -1;
    std::cout << "Cannot find uniform " << name << " in " << vertex_path << " or " << fragment_path << "\n";
    return u;
}

void Shader::recompile(std::string vertex_shader, std::string fragment_shader){
    glDeleteShader(program_id);

    compiled = _compile_shader(vertex_shader.c_str(), fragment_shader.c_str());

    if(!compiled){
        glDeleteShader(program_id);
        return;
    }

    _reinit_uniforms();
}


///////////////
/// PRIVATE
///////////////


void Shader::_reinit_uniforms(){
    for (auto u = uniforms.begin(); u != uniforms.end(); u++) {
        u->location_id = glGetUniformLocation(program_id, u->name.c_str());
    }
}

bool Shader::_check_uniform(Shader::Uniform uniform, Shader::Uniform_Type correctType){
    if(uniform.location_id == -1){
        //can be optimized away
//            std::cout << "Cannot find uniform has it been initialized and is the shader active\n";
        return false;
    }else if(uniform.type != correctType){
        std::cout << "Incorrect uniform type for " << uniform.name << "\n";
        return false;
    }

    return true;
}

bool Shader::_compile_shader(const char* vertex_shader, const char* fragment_shader){
    program_id = glCreateProgram();

    //vertex
    GLuint shader = 0;
    if(!_compile_shader_part(GL_VERTEX_SHADER, vertex_shader, shader))
        return false;

    glAttachShader(program_id, shader);

    //fragment
    shader = 0;
    if(!_compile_shader_part(GL_FRAGMENT_SHADER, fragment_shader, shader)){
        return false;
    }

    glAttachShader(program_id, shader);

    //link
    glLinkProgram(program_id);

    GLint  linked;
    glGetProgramiv(program_id, GL_LINK_STATUS, &linked );
    if (linked == GL_FALSE) {
        GLint  log_length;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<char> errorLog((size_t) log_length);
        glGetProgramInfoLog(program_id, log_length, NULL, errorLog.data() );

        std::cout << (errorLog.data()) << std::endl;
        return false;
    }

    return true;
}

bool Shader::_compile_shader_part(GLenum type, const char* code, GLuint& shader){
    shader = glCreateShader(type);

    GLint length = (GLint) strlen(code);

    glShaderSource(shader, 1, &code, &length);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if(success == GL_FALSE){
        //print errors
        GLint log_length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

        std::vector<char> errorLog((unsigned long) log_length);
        glGetShaderInfoLog(shader, log_length, &log_length, errorLog.data());

        std::string type_string = "";
        switch(type){
            case GL_VERTEX_SHADER:
                type_string = "Vertex shader";
            break;
            case GL_FRAGMENT_SHADER:
                type_string = "Fragment shader";
            break;
        }

        std::cout<<"\n"<<(std::string{errorLog.data()}+"\n"+ type_string +" error")<<std::endl;
        return false;
    }

    return true;

}
