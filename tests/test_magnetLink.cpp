#include <../include/magnetLink.h>
#include <iostream>

int main() {
  MagnetLink magnetLink = parseMagnetLink(R"(magnet:?xt=urn:btih:7278e625de2b1da598b23954c13933047126238a&dn=pixtral-12b-240910&tr=udp%3A%2F%http://2Ftracker.opentrackr.org%3A1337%2Fannounce&tr=udp%3A%2F%https://t.co/2UepcMHjvL%3A1337%2Fannounce&tr=http%3A%2F%https://t.co/NsTRgy7h8S%3A80%2Fannounce)");
  std::cout << magnetLink.info_hash << std::endl;
  std::cout << magnetLink.name << std::endl;
  for (const auto &tracker : magnetLink.trackers) {
    std::cout << tracker << std::endl;
  }
  return 0;
}

