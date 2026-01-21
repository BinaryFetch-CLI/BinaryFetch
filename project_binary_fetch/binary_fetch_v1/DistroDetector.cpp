#include "DistroDetector.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

#if defined(_WIN32)
#define PLATFORM_WINDOWS 1
#else
#define PLATFORM_WINDOWS 0
#endif

#if defined(__linux__)
#define PLATFORM_LINUX 1
#else
#define PLATFORM_LINUX 0
#endif

#if defined(__FreeBSD__)
#define PLATFORM_FREEBSD 1
#else
#define PLATFORM_FREEBSD 0
#endif

#if defined(__APPLE__)
#define PLATFORM_MACOS 1
#else
#define PLATFORM_MACOS 0
#endif

static std::string toLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string DistroDetector::readOsRelease() {
    std::ifstream file("/etc/os-release");
    if (!file.is_open()) {
        file.open("/usr/lib/os-release");
    }
    if (!file.is_open()) return "";
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

Distro DistroDetector::detectLinux() {
    std::string osRelease = toLower(readOsRelease());
    
    if (osRelease.find("nixos") != std::string::npos) return Distro::NixOS;
    if (osRelease.find("endeavouros") != std::string::npos) return Distro::EndeavourOS;
    if (osRelease.find("garuda") != std::string::npos) return Distro::Garuda;
    if (osRelease.find("manjaro") != std::string::npos) return Distro::Manjaro;
    if (osRelease.find("artix") != std::string::npos) return Distro::Artix;
    if (osRelease.find("arch") != std::string::npos) return Distro::Arch;
    if (osRelease.find("pop") != std::string::npos) return Distro::PopOS;
    if (osRelease.find("elementary") != std::string::npos) return Distro::Elementary;
    if (osRelease.find("zorin") != std::string::npos) return Distro::Zorin;
    if (osRelease.find("mint") != std::string::npos) return Distro::Mint;
    if (osRelease.find("kali") != std::string::npos) return Distro::Kali;
    if (osRelease.find("parrot") != std::string::npos) return Distro::ParrotOS;
    if (osRelease.find("ubuntu") != std::string::npos) return Distro::Ubuntu;
    if (osRelease.find("debian") != std::string::npos) return Distro::Debian;
    if (osRelease.find("fedora") != std::string::npos) return Distro::Fedora;
    if (osRelease.find("centos") != std::string::npos) return Distro::CentOS;
    if (osRelease.find("red hat") != std::string::npos || 
        osRelease.find("rhel") != std::string::npos) return Distro::RHEL;
    if (osRelease.find("opensuse") != std::string::npos || 
        osRelease.find("suse") != std::string::npos) return Distro::OpenSUSE;
    if (osRelease.find("gentoo") != std::string::npos) return Distro::Gentoo;
    if (osRelease.find("slackware") != std::string::npos) return Distro::Slackware;
    if (osRelease.find("alpine") != std::string::npos) return Distro::Alpine;
    if (osRelease.find("void") != std::string::npos) return Distro::Void;
    if (osRelease.find("mx") != std::string::npos) return Distro::MXLinux;
    
    return Distro::Unknown;
}

Distro DistroDetector::detectBSD() {
#if PLATFORM_FREEBSD
    return Distro::FreeBSD;
#endif
    
    std::ifstream file("/etc/rc.conf");
    if (file.is_open()) {
        return Distro::FreeBSD;
    }
    
    return Distro::Unknown;
}

Distro DistroDetector::detect() {
#if PLATFORM_WINDOWS
    return Distro::Windows;
#elif PLATFORM_MACOS
    return Distro::macOS;
#elif PLATFORM_FREEBSD
    return Distro::FreeBSD;
#elif PLATFORM_LINUX
    return detectLinux();
#else
    return Distro::Unknown;
#endif
}

std::string DistroDetector::getName(Distro d) {
    switch (d) {
        case Distro::Arch: return "Arch Linux";
        case Distro::Debian: return "Debian";
        case Distro::Ubuntu: return "Ubuntu";
        case Distro::Fedora: return "Fedora";
        case Distro::CentOS: return "CentOS";
        case Distro::RHEL: return "Red Hat";
        case Distro::OpenSUSE: return "openSUSE";
        case Distro::Manjaro: return "Manjaro";
        case Distro::Mint: return "Linux Mint";
        case Distro::PopOS: return "Pop!_OS";
        case Distro::Gentoo: return "Gentoo";
        case Distro::Slackware: return "Slackware";
        case Distro::Alpine: return "Alpine";
        case Distro::Void: return "Void Linux";
        case Distro::NixOS: return "NixOS";
        case Distro::EndeavourOS: return "EndeavourOS";
        case Distro::Garuda: return "Garuda Linux";
        case Distro::Kali: return "Kali Linux";
        case Distro::ParrotOS: return "Parrot OS";
        case Distro::Zorin: return "Zorin OS";
        case Distro::Elementary: return "elementary OS";
        case Distro::MXLinux: return "MX Linux";
        case Distro::Artix: return "Artix Linux";
        case Distro::FreeBSD: return "FreeBSD";
        case Distro::OpenBSD: return "OpenBSD";
        case Distro::NetBSD: return "NetBSD";
        case Distro::DragonFlyBSD: return "DragonFly BSD";
        case Distro::macOS: return "macOS";
        case Distro::Windows: return "Windows";
        default: return "Linux";
    }
}

std::string DistroDetector::getAsciiArt(Distro d) {
    switch (d) {
        case Distro::NixOS:
            return R"($6  \\  \\ //
$6 ==\\__\\/ //
$6   //   \\//
$6==//     //==
$6 //\\___//
$6// /\\  \\==
$6  // \\  \\)";

        case Distro::Arch:
            return R"($6      /\
$6     /  \
$6    /\   \
$6   /      \
$6  /   ,,   \
$6 /   |  |  -\
$6/_-''    ''-_\)";

        case Distro::Debian:
            return R"($1  _____
$1 /  __ \
$1|  /    |
$1|  \___-
$1-_
$1  --_)";

        case Distro::Ubuntu:
            return R"($3         _
$3     ---(_)
$3 _/  ---  \
$3(_) |   |
$3 \  --- _/
$3    ---(_))";

        case Distro::Fedora:
            return R"($4        _____
$4       /   __)$7\
$4       |  /  $7\ \
$7    __$4_|  |_$7_/ /
$7   / $4(_    _)$7_/
$7  / /  $4|  |
$7  \ \$4__/  |
$7   \$4(_____/)";

        case Distro::Manjaro:
            return R"($2||||||||| ||||
$2||||||||| ||||
$2||||      ||||
$2|||| |||| ||||
$2|||| |||| ||||
$2|||| |||| ||||
$2|||| |||| ||||)";

        case Distro::Mint:
            return R"($2 _____________
$2|_            \\
$2  | $7| _____ $2|
$2  | $7| | | | $2|
$2  | $7| | | | $2|
$2  | $7\\__$7___/ $2|
$2  \\_________/)";

        case Distro::PopOS:
            return R"($6______
$6\   _ \        __
$6 \ \ \ \      / /
$6  \ \_\ \    / /
$6   \  ___\  /_/
$6    \ \    _
$6   __\_\__(_)_
$6  (___________)";

        case Distro::Gentoo:
            return R"($5 _-----_
$5(       \\
$5\    0   \\
$7 \        )
$7 /      _/
$7(     _-
$7\____-)";

        case Distro::Alpine:
            return R"($4   /\ /\
$4  /  \  \
$4 /    \  \
$4/      \  \
$4\       \ /
$4 \       /
$4  \     /)";

        case Distro::Void:
            return R"($2    _______
$2 _ \______ -
$2| \  ___  \ |
$2| | /   \ | |
$2| | \___/ | |
$2| \______ \_|
$2 -_______\)";

        case Distro::EndeavourOS:
            return R"($5      /$1\
$5    /$1/  \$5\
$5   /$1/ $5/$1\  \$5\
$5  /$1/ $5/$6   \$1\  \$5\
$5 /$1/ $5/$6      \$1\  \$5\
$5/$1/  $6         \$1\  \$5\
$5\$1\$6            /$1/  $5/
$5 \$1\$6_________/$1/  $5/)";

        case Distro::Garuda:
            return R"($6          ..
$6        .;;,.
$6       ';;;;;;;.
$6     ':;;;;;;;;;;,
$6   .:;;;;;;;$1''$6;;;'
$6  ';;;;;;$1'$6  .$1';;$6
$6.;;;$1'.  .$6;;;$1'.
$6;;$1'  .;;;;;$1'.)";

        case Distro::Kali:
            return R"($4..............
$4            ..,;:ccc,.
$4          ......''';lxO.
$4.....''''..........,:ld;
$4           .';;;:::;,,.x,
$4      ..'''.            0Kx
$4  ....                   KKK)";

        case Distro::Artix:
            return R"($6      /\
$6     /  \
$6    /`'.,\
$6   /     ',
$6  /      ,`\
$6 /   ,.'`.  \
$6/.,'`     `'.\)";

        case Distro::FreeBSD:
            return R"($1 /\,-'''''-,/\
$1 \_)       (_/
$1 |   \ /   |
$1 |   O O   |
$1  ;  ._,  ;
$1   '-___-')";

        case Distro::OpenBSD:
            return R"($3      _____
$3    \-     -/
$3 \_/         \
$3 |        O O |
$3 |_  <   )  3 )
$3 / \         /
$3    /-_____-\)";

        case Distro::macOS:
            return R"($2        .:'
$2    __ :'__
$3 .'`  `-'  ``.
$1:          .-'
$1:         :
$5 :         `-;
$4  `.__.-.__.')";

        case Distro::Windows:
            return R"($6lllllllll  lllllllll
$6lllllllll  lllllllll
$6lllllllll  lllllllll
$6lllllllll  lllllllll

$6lllllllll  lllllllll
$6lllllllll  lllllllll
$6lllllllll  lllllllll
$6lllllllll  lllllllll)";

        case Distro::OpenSUSE:
            return R"($2  _______
$2__|   __ \
$2     / .\\ \\
$2     \\__/ |
$2   _______|
$2   \_______
$2__________/)";

        case Distro::CentOS:
        case Distro::RHEL:
            return R"($2       .---.
$3      /     \\
$3      \\     /
$2    /`--.--'\\
$4   /  $2.'o'.$4  \\
$4  /  `-----'  \\
$4  \\           /
$4   `--.___,--')";

        default:
            return R"($7    ___
$7   (.· |
$7   (<> |
$7  / __  \
$7 ( /  \ /|
$7_/\ __)/_)
$7\/-____\/)";
    }
}
