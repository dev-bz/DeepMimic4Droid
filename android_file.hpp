#pragma once

#ifdef __ANDROID__
#include <istream>
#include <string>

namespace std {
struct asset_steam {
  bool from_file;
  unsigned char *data;

  std::unique_ptr<std::istream> file_stream;
  asset_steam(const std::string &char_file);
  operator std::istream &();
  void close();
  ~asset_steam();
};
#define ifstream asset_steam
} // namespace std
#endif