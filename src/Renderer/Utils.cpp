#include "Utils.h"

std::vector<char> Utils::LoadBinaryFromFile(const std::string& file_path) {
  std::ifstream ifs(file_path, std::ios::ate | std::ios::binary);
  if (!ifs.is_open()) {
    SPDLOG_ERROR("FAILED TO OPEN FILE {}", file_path);
    return {};
  }
  uint64 file_size = static_cast<uint64>(ifs.tellg());
  std::vector<char> Buffer(file_size);
  ifs.seekg(0);
  ifs.read(Buffer.data(), file_size);
  return Buffer;
}

std::string Utils::LoadCodeFromFile(const std::string& file_path) {
  std::ifstream ifs(file_path);
  if (ifs) {
    std::ostringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
  }
  return "";
}
