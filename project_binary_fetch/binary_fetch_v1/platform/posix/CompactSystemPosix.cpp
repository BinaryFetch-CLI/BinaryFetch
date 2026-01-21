#include "CompactSystem.h"
#include "platform/Platform.h"

#if PLATFORM_LINUX

std::string CompactSystem::getBIOSInfo() {
    std::string vendor = Platform::readFileLine("/sys/class/dmi/id/bios_vendor");
    std::string version = Platform::readFileLine("/sys/class/dmi/id/bios_version");
    
    vendor = Platform::trim(vendor);
    version = Platform::trim(version);
    
    if (!vendor.empty() && !version.empty()) {
        return vendor + " " + version;
    }
    if (!vendor.empty()) return vendor;
    if (!version.empty()) return version;
    
    if (Platform::commandExists("dmidecode")) {
        std::string result = Platform::exec("sudo dmidecode -s bios-vendor 2>/dev/null");
        result = Platform::trim(result);
        if (!result.empty()) {
            std::string ver = Platform::exec("sudo dmidecode -s bios-version 2>/dev/null");
            ver = Platform::trim(ver);
            if (!ver.empty()) result += " " + ver;
            return result;
        }
    }
    
    return "Unknown";
}

std::string CompactSystem::getMotherboardInfo() {
    std::string product = Platform::readFileLine("/sys/class/dmi/id/board_name");
    std::string vendor = Platform::readFileLine("/sys/class/dmi/id/board_vendor");
    
    product = Platform::trim(product);
    vendor = Platform::trim(vendor);
    
    if (!product.empty() && !vendor.empty()) {
        return vendor + " " + product;
    }
    if (!product.empty()) return product;
    if (!vendor.empty()) return vendor;
    
    if (Platform::commandExists("dmidecode")) {
        std::string result = Platform::exec("sudo dmidecode -s baseboard-product-name 2>/dev/null");
        result = Platform::trim(result);
        if (!result.empty()) return result;
    }
    
    return "Unknown";
}

#elif PLATFORM_FREEBSD

std::string CompactSystem::getBIOSInfo() {
    if (Platform::commandExists("kenv")) {
        std::string vendor = Platform::exec("kenv smbios.bios.vendor 2>/dev/null");
        std::string version = Platform::exec("kenv smbios.bios.version 2>/dev/null");
        
        vendor = Platform::trim(vendor);
        version = Platform::trim(version);
        
        if (!vendor.empty() && !version.empty()) {
            return vendor + " " + version;
        }
        if (!vendor.empty()) return vendor;
        if (!version.empty()) return version;
    }
    return "Unknown";
}

std::string CompactSystem::getMotherboardInfo() {
    if (Platform::commandExists("kenv")) {
        std::string product = Platform::exec("kenv smbios.planar.product 2>/dev/null");
        std::string maker = Platform::exec("kenv smbios.planar.maker 2>/dev/null");
        
        product = Platform::trim(product);
        maker = Platform::trim(maker);
        
        if (!product.empty() && !maker.empty()) {
            return maker + " " + product;
        }
        if (!product.empty()) return product;
        if (!maker.empty()) return maker;
    }
    return "Unknown";
}

#endif
