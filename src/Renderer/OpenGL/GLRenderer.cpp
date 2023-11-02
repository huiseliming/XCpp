#include "GLRenderer.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

static float vertices[] = {
    -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f,
};
const char* VertexShaderSource =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";


const char* FragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
} 
)";

void CGLRenderer::Init(SDL_Window* main_window) {
  MainWindow = main_window;
  OpenGLContext = SDL_GL_CreateContext(MainWindow);
  SDL_GL_MakeCurrent(MainWindow, OpenGLContext);
  X_RUNTIME_ASSERT(OpenGLContext != nullptr);

  gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

  X_RUNTIME_ASSERT(SDL_GL_SetSwapInterval(1) >= 0);

  // BG
  GLuint BufferHandle;
  glGenBuffers(1, &BufferHandle);
  glBindBuffer(GL_ARRAY_BUFFER, BufferHandle);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &VertexShaderSource, NULL);
  glCompileShader(vertex_shader);

  GLint is_succeeded = GL_FALSE;
  GLint info_log_length;

  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &is_succeeded);
  if (is_succeeded) {

  } else {
    glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length > 0) {
      std::vector<char> info_log(info_log_length + 1);
      glGetShaderInfoLog(vertex_shader, info_log_length, NULL, &info_log[0]);
      SPDLOG_WARN("vertex_shader : %s", info_log.data());
    }
  }

  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &FragmentShaderSource, NULL);
  glCompileShader(fragment_shader);

  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &is_succeeded);
  if (!is_succeeded) {
    glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length > 0) {
      std::vector<char> info_log(info_log_length + 1);
      glGetShaderInfoLog(fragment_shader, info_log_length, NULL, &info_log[0]);
      SPDLOG_WARN("fragment_shader : %s", info_log.data());
    }
  }
  unsigned int shader_program;
  shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);

  glGetProgramiv(shader_program, GL_LINK_STATUS, &is_succeeded);
  if (!is_succeeded) {
    glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length > 0) {
      std::vector<char> info_log(info_log_length + 1);
      glGetProgramInfoLog(shader_program, info_log_length, NULL, &info_log[0]);
      SPDLOG_WARN("shader_program : %s", info_log.data());
    }
  }
  glUseProgram(shader_program);

  glDeleteShader(fragment_shader);
  glDeleteShader(vertex_shader);

  //
  //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  //glEnableVertexAttribArray(0);  

  // ED
}

void CGLRenderer::Render() {
  glClearColor(ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w);
  glClear(GL_COLOR_BUFFER_BIT);
  // BG

  // ED
  SDL_GL_SwapWindow(MainWindow);
}

void CGLRenderer::Shutdown() {
  // BG

  // ED
  SDL_GL_DeleteContext(OpenGLContext);
}

OTexture* CGLRenderer::ImportTextureFromFile(const std::string& file_path) {
  GLuint texture_handle;
  glGenTextures(1, &texture_handle);
  glBindTexture(GL_TEXTURE_2D, texture_handle);  // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
  
  //glGetTexParameterIiv();
  // set the texture wrapping parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);  // set texture wrapping to GL_REPEAT (default wrapping method)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // set texture filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // load image, create texture and generate mipmaps
  int width, height, nrChannels;
  // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with
  // your own image path.
  unsigned char* image_data = stbi_load(file_path.c_str(), &width, &height, &nrChannels, 0);
  
  if (!image_data) return nullptr;

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
  glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(image_data);
  return nullptr;
}

OMesh* CGLRenderer::ImportMeshFromFile(const std::string& file_path) {
    Assimp::Importer importer;

    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // probably to request more postprocessing than we do in this example.
    const aiScene* scene = importer.ReadFile(file_path, aiProcess_CalcTangentSpace | aiProcess_Triangulate |
                                                            aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);

    // If the import failed, report it
    if (nullptr == scene) {
      SPDLOG_ERROR(importer.GetErrorString());
      return nullptr;
    }

  return nullptr;
}
