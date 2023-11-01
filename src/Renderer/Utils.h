#pragma once

#define STBIDEF XRENDERER_API
#include <stb_image.h>

namespace Utils {

XRENDERER_API std::vector<char> LoadBinaryFromFile(const std::string& file_path);
XRENDERER_API std::string LoadCodeFromFile(const std::string& file_path);
}  // namespace Utils