#include "DisplayInfo.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <dirent.h>
#include <cstdio>
#include <cstring>
#include <memory>
#include <array>
#include <cmath>
#include <algorithm>
#include <unistd.h>
using namespace std;

// small helpers (no access to DisplayInfo internals needed here)
static string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Run a shell command and return whatever it printed to stdout.
static string runCommand(const string& cmd) {
    string result;
    array<char, 256> buffer;
    string fullCmd = cmd + " 2>/dev/null";
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(fullCmd.c_str(), "r"), pclose);
    if (!pipe) return "";
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

static vector<unsigned char> readBinaryFile(const string& path) {
    ifstream file(path, ios::binary);
    if (!file.is_open()) return {};
    return vector<unsigned char>(istreambuf_iterator<char>(file), istreambuf_iterator<char>());
}

static int gcdInt(int a, int b) { return b == 0 ? a : gcdInt(b, a % b); }

// Same formatting as DisplayInfo::scaleMultiplier(), duplicated here because
// that method is private.
static string formatScaleMultiplier(int scalePercent) {
    double mul = scalePercent / 100.0;
    ostringstream ss;
    ss.precision(mul == (int)mul ? 0 : 2);
    ss << fixed << mul << "x";
    return ss.str();
}

// Constructor / accessors
DisplayInfo::DisplayInfo() {
    refresh();
}

const vector<DisplayInfo::ScreenInfo>& DisplayInfo::getScreens() const {
    return screens;
}

// GPU vendor detection
static string activeDrmDriver() {
    DIR* dir = opendir("/sys/class/drm");
    if (!dir) return "";
    string driver;
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        string name = entry->d_name;
        if (name.rfind("card", 0) != 0) continue;
        if (name.find('-') != string::npos) continue;
        string driverLink = "/sys/class/drm/" + name + "/device/driver";
        char buf[256];
        ssize_t len = readlink(driverLink.c_str(), buf, sizeof(buf) - 1);
        if (len > 0) {
            buf[len] = '\0';
            string path = buf;
            size_t slash = path.find_last_of('/');
            driver = (slash != string::npos) ? path.substr(slash + 1) : path;
            break;
        }
    }
    closedir(dir);
    return driver;
}

bool DisplayInfo::isNvidiaPresent() {
    return activeDrmDriver() == "nvidia";
}

bool DisplayInfo::isAMDPresent() {
    string d = activeDrmDriver();
    return d == "amdgpu" || d == "radeon";
}

// EDID parsing (binary, straight from the monitor's own firmware).
//
// Detailed Timing Descriptor layout (18 bytes, offsets relative to the
// start of the descriptor, i.e. `dtd = edid + 54`):
//   dtd[0..1] = pixel clock
//   dtd[2]    = H active, low 8 bits
//   dtd[3]    = H blanking, low 8 bits
//   dtd[4]    = high nibble -> H active bits 8-11, low nibble -> H blanking bits 8-11
//   dtd[5]    = V active, low 8 bits
//   dtd[6]    = V blanking, low 8 bits
//   dtd[7]    = high nibble -> V active bits 8-11, low nibble -> V blanking bits 8-11
DisplayInfo::EDIDInfo DisplayInfo::parseEDID(const unsigned char* edid, size_t size) {
    EDIDInfo info;
    info.valid = false;
    info.nativeWidth = 0;
    info.nativeHeight = 0;
    if (!edid || size < 128) return info;
    static const unsigned char header[8] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};
    if (memcmp(edid, header, 8) != 0) return info;

    // First detailed timing descriptor: offset 54, 18 bytes.
    const unsigned char* dtd = edid + 54;
    unsigned short pixelClock = dtd[0] | (dtd[1] << 8);
    if (pixelClock != 0) {
        int hActive = dtd[2] | ((dtd[4] & 0xF0) << 4);
        int vActive = dtd[5] | ((dtd[7] & 0xF0) << 4);
        if (hActive > 0 && vActive > 0) {
            info.nativeWidth = hActive;
            info.nativeHeight = vActive;
            info.valid = true;
        }
    }

    // Monitor name descriptor (tag 0xFC), one of 4 possible 18-byte blocks.
    for (int i = 0; i < 4; ++i) {
        const unsigned char* block = edid + 54 + (i * 18);
        if (block[0] == 0x00 && block[1] == 0x00 && block[2] == 0x00 && block[3] == 0xFC) {
            string name;
            for (int j = 5; j < 18; ++j) {
                char c = (char)block[j];
                if (c == 0x0A || c == 0x00) break;
                name += c;
            }
            info.friendlyName = trim(name);
            break;
        }
    }
    return info;
}

string DisplayInfo::getFriendlyNameFromEDID(const wstring& deviceName) {
    string connector(deviceName.begin(), deviceName.end());
    DIR* dir = opendir("/sys/class/drm");
    if (!dir) return "";
    string edidPath;
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        string name = entry->d_name;
        if (name.find(connector) != string::npos) {
            edidPath = "/sys/class/drm/" + name + "/edid";
            break;
        }
    }
    closedir(dir);
    if (edidPath.empty()) return "";
    vector<unsigned char> raw = readBinaryFile(edidPath);
    if (raw.empty()) return "";
    EDIDInfo info = parseEDID(raw.data(), raw.size());
    return info.valid ? info.friendlyName : "";
}

// Small pure-formatting helpers
string DisplayInfo::WideToUtf8(const wchar_t* w) {
    if (!w) return "";
    wstring ws(w);
    return string(ws.begin(), ws.end());
}

string DisplayInfo::scaleMultiplier(int scalePercent) {
    double mul = scalePercent / 100.0;
    ostringstream ss;
    ss.precision(mul == (int)mul ? 0 : 2);
    ss << fixed << mul << "x";
    return ss.str();
}

int DisplayInfo::computeUpscaleFactor(int currentWidth, int nativeWidth) {
    if (nativeWidth <= 0 || currentWidth <= nativeWidth) return 1;
    return (int)round((double)currentWidth / (double)nativeWidth);
}

string DisplayInfo::computeAspectRatio(int w, int h) {
    if (w <= 0 || h <= 0) return "Unknown";
    int g = gcdInt(w, h);
    if (g == 0) return "Unknown";
    return to_string(w / g) + ":" + to_string(h / g);
}

// Core population — tries three tiers, in order:
//   1) xrandr      -> X11 (and XWayland where it happens to work)
//   2) wlr-randr   -> wlroots-based Wayland compositors
//   3) sysfs only  -> /sys/class/drm directly; works with NO display server
bool DisplayInfo::populateFromDXGI() {
    screens.clear();

    auto lookupEdid = [this](const string& connectorName, int& w, int& h, string& friendlyName) -> bool {
        DIR* dir = opendir("/sys/class/drm");
        if (!dir) return false;
        string edidPath;
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            string dname = entry->d_name;
            if (dname.find(connectorName) != string::npos) {
                edidPath = "/sys/class/drm/" + dname + "/edid";
                break;
            }
        }
        closedir(dir);
        if (edidPath.empty()) return false;
        vector<unsigned char> raw = readBinaryFile(edidPath);
        if (raw.empty()) return false;
        EDIDInfo info = parseEDID(raw.data(), raw.size());
        if (!info.valid) return false;
        w = info.nativeWidth;
        h = info.nativeHeight;
        friendlyName = info.friendlyName;
        return true;
    };

    // ---------------- Tier 1: xrandr (X11) ----------------
    string xrandrOutput = runCommand("xrandr --query");
    if (!xrandrOutput.empty()) {
        istringstream stream(xrandrOutput);
        string line;
        string currentConnector;
        while (getline(stream, line)) {
            if (line.find(" connected") != string::npos) {
                istringstream ls(line);
                string name, state;
                ls >> name >> state;
                ScreenInfo screen;
                screen.name = name;
                currentConnector = name;

                // Find "<W>x<H>+<X>+<Y>" geometry token, e.g. "1920x1080+0+0".
                // Scan token-by-token instead of raw char offsets so we don't
                // misfire on digits inside the connector name (e.g. "HDMI-0").
                istringstream ts(line);
                string token;
                while (ts >> token) {
                    size_t xPos = token.find('x');
                    if (xPos == string::npos || xPos == 0 || xPos + 1 >= token.size()) continue;
                    if (!isdigit((unsigned char)token[0]) || !isdigit((unsigned char)token[xPos + 1])) continue;
                    size_t plusSign = token.find('+', xPos);
                    if (plusSign == string::npos) continue;
                    bool digitsOnlyBeforeX = true;
                    for (size_t k = 0; k < xPos; ++k) {
                        if (!isdigit((unsigned char)token[k])) { digitsOnlyBeforeX = false; break; }
                    }
                    if (!digitsOnlyBeforeX) continue;
                    try {
                        screen.current_width = stoi(token.substr(0, xPos));
                        screen.current_height = stoi(token.substr(xPos + 1, plusSign - xPos - 1));
                    } catch (...) {
                        continue;
                    }
                    break;
                }

                int nativeW = 0, nativeH = 0;
                string friendly;
                bool haveEdid = lookupEdid(name, nativeW, nativeH, friendly);
                if (haveEdid) {
                    screen.native_width = nativeW;
                    screen.native_height = nativeH;
                    screen.native_resolution = to_string(nativeW) + "x" + to_string(nativeH);
                    if (!friendly.empty()) screen.name = friendly;
                } else {
                    screen.native_width = screen.current_width;
                    screen.native_height = screen.current_height;
                    screen.native_resolution = to_string(screen.current_width) + "x" + to_string(screen.current_height);
                }
                screen.aspect_ratio = computeAspectRatio(screen.current_width, screen.current_height);
                screens.push_back(screen);
            }
            else if (!currentConnector.empty() && line.find('*') != string::npos && !screens.empty()) {
                size_t star = line.find('*');
                size_t numStart = line.find_last_not_of("0123456789. \t", star - 1);
                if (numStart != string::npos) {
                    string refreshStr = trim(line.substr(numStart + 1, star - numStart - 1));
                    if (!refreshStr.empty()) {
                        try {
                            screens.back().refresh_rate = (int)round(stod(refreshStr));
                        } catch (...) {}
                    }
                }
            }
        }
        if (!screens.empty()) return true;
    }

    // ---------------- Tier 2: wlr-randr (wlroots Wayland compositors) ----------------
    string wlrOutput = runCommand("wlr-randr");
    if (!wlrOutput.empty()) {
        istringstream stream(wlrOutput);
        string line;
        bool connectorEnabled = false;
        while (getline(stream, line)) {
            string trimmed = trim(line);
            if (!line.empty() && line[0] != ' ' && line[0] != '\t') {
                istringstream ls(line);
                string name;
                ls >> name;
                ScreenInfo screen;
                screen.name = name;
                screens.push_back(screen);
                connectorEnabled = true;
                continue;
            }
            if (screens.empty()) continue;
            ScreenInfo& screen = screens.back();
            if (trimmed.rfind("Enabled:", 0) == 0) {
                connectorEnabled = (trimmed.find("yes") != string::npos);
                if (!connectorEnabled) {
                    screens.pop_back();
                }
                continue;
            }
            if (!connectorEnabled) continue;
            if (trimmed.find("px,") != string::npos && trimmed.find("current") != string::npos) {
                int w = 0, h = 0;
                double hz = 0.0;
                if (sscanf(trimmed.c_str(), "%dx%d px, %lf Hz", &w, &h, &hz) == 3) {
                    screen.current_width = w;
                    screen.current_height = h;
                    screen.refresh_rate = (int)round(hz);
                }
            }
            if (trimmed.rfind("Scale:", 0) == 0) {
                double scale = 1.0;
                if (sscanf(trimmed.c_str(), "Scale: %lf", &scale) == 1) {
                    screen.scale_percent = (int)round(scale * 100.0);
                    screen.scale_mul = scaleMultiplier(screen.scale_percent);
                }
            }
        }
        for (auto& screen : screens) {
            int nativeW = 0, nativeH = 0;
            string friendly;
            bool haveEdid = lookupEdid(screen.name, nativeW, nativeH, friendly);
            if (haveEdid) {
                screen.native_width = nativeW;
                screen.native_height = nativeH;
                screen.native_resolution = to_string(nativeW) + "x" + to_string(nativeH);
                if (!friendly.empty()) screen.name = friendly;
            } else {
                screen.native_width = screen.current_width;
                screen.native_height = screen.current_height;
                screen.native_resolution = to_string(screen.current_width) + "x" + to_string(screen.current_height);
            }
            screen.aspect_ratio = computeAspectRatio(screen.current_width, screen.current_height);
        }
        if (!screens.empty()) return true;
    }

    // ---------------- Tier 3: raw /sys/class/drm scan (works anywhere) ----------------
    DIR* dir = opendir("/sys/class/drm");
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            string dname = entry->d_name;
            if (dname.rfind("card", 0) != 0) continue;
            if (dname.find('-') == string::npos) continue;
            string statusPath = "/sys/class/drm/" + dname + "/status";
            ifstream statusFile(statusPath);
            if (!statusFile.is_open()) continue;
            string status;
            getline(statusFile, status);
            if (trim(status) != "connected") continue;
            size_t dash = dname.find('-');
            string connectorName = dname.substr(dash + 1);
            ScreenInfo screen;
            screen.name = connectorName;
            int nativeW = 0, nativeH = 0;
            string friendly;
            if (lookupEdid(dname, nativeW, nativeH, friendly)) {
                screen.native_width = nativeW;
                screen.native_height = nativeH;
                screen.native_resolution = to_string(nativeW) + "x" + to_string(nativeH);
                if (!friendly.empty()) screen.name = friendly;
                screen.current_width = nativeW;
                screen.current_height = nativeH;
                screen.refresh_rate = 0;
            }
            screen.aspect_ratio = computeAspectRatio(screen.current_width, screen.current_height);
            screens.push_back(screen);
        }
        closedir(dir);
    }
    return !screens.empty();
}

// DSR/VSR labeling — heuristic only.
bool DisplayInfo::enrichWithNVAPI() {
    if (!isNvidiaPresent()) return false;
    bool any = false;
    for (auto& s : screens) {
        int factor = computeUpscaleFactor(s.current_width, s.native_width);
        if (factor > 1) {
            s.dsr_enabled = true;
            s.dsr_type = "DSR";
            s.upscale = to_string(factor) + "x";
            any = true;
        } else if (s.dsr_type.empty()) {
            s.dsr_type = "None";
            s.upscale = "1x";
        }
    }
    return any;
}

bool DisplayInfo::enrichWithADL() {
    if (!isAMDPresent()) return false;
    bool any = false;
    for (auto& s : screens) {
        if (s.dsr_enabled) continue;
        int factor = computeUpscaleFactor(s.current_width, s.native_width);
        if (factor > 1) {
            s.dsr_enabled = true;
            s.dsr_type = "VSR";
            s.upscale = to_string(factor) + "x";
            any = true;
        } else if (s.dsr_type.empty()) {
            s.dsr_type = "None";
            s.upscale = "1x";
        }
    }
    return any;
}

// Scaling — best-effort, X11 only.
static void applyScaleFromXrandrVerbose(vector<DisplayInfo::ScreenInfo>& screens) {
    string verbose = runCommand("xrandr --verbose");
    if (verbose.empty()) return;
    istringstream stream(verbose);
    string line;
    DisplayInfo::ScreenInfo* current = nullptr;
    while (getline(stream, line)) {
        for (auto& s : screens) {
            if (line.rfind(s.name, 0) == 0 && line.find(" connected") != string::npos) {
                current = &s;
                break;
            }
        }
        if (current && line.find("Transform:") != string::npos) {
            istringstream ts(line.substr(line.find(':') + 1));
            double m00 = 1.0;
            ts >> m00;
            if (m00 > 0.0) {
                current->scale_percent = (int)round(m00 * 100.0);
                current->scale_mul = formatScaleMultiplier(current->scale_percent);
            }
        }
    }
    for (auto& s : screens) {
        if (s.scale_mul.empty()) {
            s.scale_mul = formatScaleMultiplier(s.scale_percent);
        }
    }
}

// Public entry point
bool DisplayInfo::refresh() {
    bool ok = populateFromDXGI();
    if (!ok) return false;
    applyScaleFromXrandrVerbose(screens);
    enrichWithNVAPI();
    enrichWithADL();
    return true;
}