#ifndef MAGNET_LINK_HPP
#define MAGNET_LINK_HPP

#include <string>
#include <vector>
#include <stdexcept>

struct MagnetLink {
  std::string info_hash, name;
  std::vector<std::string> trackers;
};

MagnetLink parseMagnetLink(const std::string &magnetLink) {
  MagnetLink result;
  const std::string prefix = "xt=urn:btih:";
  size_t startPos = magnetLink.find(prefix);
  if (startPos == std::string::npos) {
    throw std::invalid_argument("Invalid magnet link");
  }
  startPos += prefix.size();
  size_t endPos = magnetLink.find('&', startPos);
  if (endPos == std::string::npos) {
    throw std::invalid_argument("Invalid magnet link");
  }
  result.info_hash = magnetLink.substr(startPos, endPos - startPos);
  size_t nameStartPos = magnetLink.find("dn=");
  if (nameStartPos != std::string::npos) {
    size_t nameEndPos = magnetLink.find('&', nameStartPos);
    result.name = magnetLink.substr(nameStartPos + 3, nameEndPos - nameStartPos - 3);
  }
  size_t trackerStartPos = magnetLink.find("tr=");
  while(trackerStartPos != std::string::npos) {
    size_t trackerEndPos = magnetLink.find('&', trackerStartPos);
    result.trackers.push_back(magnetLink.substr(trackerStartPos + 3, trackerEndPos - trackerStartPos - 3));
    trackerStartPos = magnetLink.find("tr=", trackerEndPos);
  }
  return result;
}

#endif
