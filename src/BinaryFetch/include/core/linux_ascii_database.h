#ifndef LINUX_ASCII_DATABASE_H
#define LINUX_ASCII_DATABASE_H

#include <string>
#include <vector>
#include <map>

struct DistroArt {
    std::string art;
    std::vector<std::string> aliases;
};

const std::map<std::string, DistroArt>& getDistroArtDatabase();
const std::string& getDistroArt(const std::string& distroId);

#endif // LINUX_ASCII_DATABASE_H