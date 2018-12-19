#include "android_file.hpp"
#include <fstream>
#include <strstream>
#undef ifstream
extern "C" int get_asset_data_size(const char *path, long *size,
                                   unsigned char **out);
extern "C" void free_asset_data_size(unsigned char **out);

namespace std {
asset_steam::asset_steam(const std::string &char_file) {
  from_file = false;
  long file_size = 0;
  data = NULL;
  if (get_asset_data_size(char_file.c_str(), &file_size, &data)) {
    /*FILE *f = fopen(char_file.c_str(), "rb");
    if (f) {
      fclose(f);
      file_stream.reset(new std::ifstream(char_file));
    } else {
      file_stream.reset(new std::ifstream("apkoverlay/assets/" + char_file));
    }
    from_file = true;*/
  } else {
    std::streamsize size = file_size;
    file_stream.reset(new std::istrstream((char *)data, size));
  }
}
asset_steam::operator std::istream &() { return *file_stream.get(); }
void asset_steam::close() {
  if (from_file) {
    std::ifstream *st = dynamic_cast<std::ifstream *>(file_stream.get());
    if (st) st->close();
    from_file = false;
  } else if (data)
    free_asset_data_size(&data);
}
asset_steam::~asset_steam() { close(); }
} // namespace std