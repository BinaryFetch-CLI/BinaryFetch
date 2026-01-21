#ifndef DISTRO_DETECTOR_H
#define DISTRO_DETECTOR_H

#include <string>

enum class Distro {
    Unknown,
    Arch, Debian, Ubuntu, Fedora, CentOS, RHEL, OpenSUSE, Manjaro,
    Mint, PopOS, Gentoo, Slackware, Alpine, Void, NixOS, EndeavourOS,
    Garuda, Kali, ParrotOS, Zorin, Elementary, MXLinux, Artix,
    FreeBSD, OpenBSD, NetBSD, DragonFlyBSD,
    macOS,
    Windows
};

class DistroDetector {
public:
    static Distro detect();
    static std::string getName(Distro d);
    static std::string getAsciiArt(Distro d);
    
private:
    static Distro detectLinux();
    static Distro detectBSD();
    static std::string readOsRelease();
};

#endif
