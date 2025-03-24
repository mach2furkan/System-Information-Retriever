#include <iostream>
#include <vector>
#include <string>
#include <sys/sysctl.h>
#include <mach/mach_host.h>
#include <mach/host_info.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <ifaddrs.h>
#include <IOKit/graphics/IOGraphicsLib.h>
#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPSKeys.h>
#include <libproc.h>
#include <unistd.h>

using namespace std;

// Helper function to retrieve system information using sysctl
string getSysctlInfo(const string& key) {
    size_t size = 0;
    sysctlbyname(key.c_str(), nullptr, &size, nullptr, 0);
    string result(size, '\0');
    sysctlbyname(key.c_str(), &result[0], &size, nullptr, 0);
    return result;
}

// Get Total System Memory (macOS)
uint64_t getTotalSystemMemory() {
    uint64_t memSize = 0;
    size_t len = sizeof(memSize);
    sysctlbyname("hw.memsize", &memSize, &len, nullptr, 0);
    return memSize / (1024 * 1024); // Convert to MB
}

// Get CPU Information (macOS)
string getCPUInfo() {
    return getSysctlInfo("machdep.cpu.brand_string");
}

// Get OS Version (macOS)
string getOSVersion() {
    return getSysctlInfo("kern.osrelease");
}

// Get Disk Drive Information (macOS)
vector<string> getDiskDriveInfo() {
    vector<string> diskDrives;
    string diskInfo = getSysctlInfo("kern.disks");
    size_t pos = 0;
    while ((pos = diskInfo.find(" ")) != string::npos) {
        diskDrives.push_back(diskInfo.substr(0, pos));
        diskInfo.erase(0, pos + 1);
    }
    if (!diskInfo.empty()) {
        diskDrives.push_back(diskInfo);
    }
    return diskDrives;
}

// Get MAC Address (macOS)
string getMACAddress() {
    struct ifaddrs* ifAddrStruct = nullptr;
    struct ifaddrs* ifa = nullptr;
    void* tmpAddrPtr = nullptr;

    getifaddrs(&ifAddrStruct);
    string macAddress;

    for (ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;
        if (ifa->ifa_addr->sa_family == AF_LINK) {
            tmpAddrPtr = &((struct sockaddr_dl*)ifa->ifa_addr)->sdl_data[((struct sockaddr_dl*)ifa->ifa_addr)->sdl_nlen];
            uint8_t* mac = (uint8_t*)tmpAddrPtr;
            char macStr[18];
            snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            macAddress = macStr;
            break;
        }
    }

    if (ifAddrStruct != nullptr) freeifaddrs(ifAddrStruct);
    return macAddress;
}

// Get Monitor Resolutions (macOS)
vector<string> getMonitorResolutions() {
    vector<string> resolutions;
    io_iterator_t iter;
    kern_return_t kr = IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching("IODisplayConnect"), &iter);
    if (kr != KERN_SUCCESS) return resolutions;

    io_object_t service;
    while ((service = IOIteratorNext(iter))) {
        CFDictionaryRef displayDict = (CFDictionaryRef)IORegistryEntryCreateCFProperty(service, CFSTR("IODisplayEDID"), kCFAllocatorDefault, 0);
        if (displayDict) {
            const uint8_t* edidData = nullptr;
            CFDataRef edidCFData = (CFDataRef)displayDict;
            if (CFGetTypeID(edidCFData) == CFDataGetTypeID()) {
                edidData = CFDataGetBytePtr(edidCFData);
                if (edidData) {
                    uint16_t width = (edidData[68] | (edidData[69] << 8));
                    uint16_t height = (edidData[70] | (edidData[71] << 8));
                    resolutions.push_back(to_string(width) + "x" + to_string(height));
                }
            }
            CFRelease(displayDict);
        }
        IOObjectRelease(service);
    }
    IOObjectRelease(iter);
    return resolutions;
}

// New Functionality 1: Get Installed RAM Slots and Capacity
vector<string> getInstalledRAMInfo() {
    vector<string> ramInfo;
    int mib[2] = {CTL_HW, HW_MEMSIZE};
    uint64_t totalMemory = 0;
    size_t length = sizeof(totalMemory);

    if (sysctl(mib, 2, &totalMemory, &length, NULL, 0) == 0) {
        ramInfo.push_back("Total Installed RAM: " + to_string(totalMemory / (1024 * 1024)) + " MB");
    } else {
        ramInfo.push_back("Failed to retrieve RAM information");
    }

    return ramInfo;
}

// New Functionality 2: Get Battery Status
string getBatteryStatus() {
    string batteryInfo;
    CFTypeRef powerSourcesInfo = IOPSCopyPowerSourcesInfo();
    CFArrayRef powerSourcesList = IOPSCopyPowerSourcesList(powerSourcesInfo);

    if (powerSourcesList == nullptr) {
        return "No battery information available";
    }

    for (int i = 0; i < CFArrayGetCount(powerSourcesList); ++i) {
        CFTypeRef powerSource = CFArrayGetValueAtIndex(powerSourcesList, i);
        CFDictionaryRef powerSourceDict = IOPSGetPowerSourceDescription(powerSourcesInfo, powerSource);

        if (powerSourceDict) {
            CFNumberRef currentCapacityRef = (CFNumberRef)CFDictionaryGetValue(powerSourceDict, CFSTR(kIOPSCurrentCapacityKey));
            CFNumberRef maxCapacityRef = (CFNumberRef)CFDictionaryGetValue(powerSourceDict, CFSTR(kIOPSMaxCapacityKey));
            CFStringRef isChargingRef = (CFStringRef)CFDictionaryGetValue(powerSourceDict, CFSTR(kIOPSIsChargingKey));

            if (currentCapacityRef && maxCapacityRef) {
                int currentCapacity = 0, maxCapacity = 0;
                CFNumberGetValue(currentCapacityRef, kCFNumberIntType, &currentCapacity);
                CFNumberGetValue(maxCapacityRef, kCFNumberIntType, &maxCapacity);

                int batteryPercentage = (currentCapacity * 100) / maxCapacity;
                batteryInfo += "Battery Percentage: " + to_string(batteryPercentage) + "%\n";

                if (isChargingRef) {
                    batteryInfo += "Charging: " + string(CFStringGetCStringPtr(isChargingRef, kCFStringEncodingUTF8)) + "\n";
                }
            }
        }
    }

    CFRelease(powerSourcesList);
    CFRelease(powerSourcesInfo);
    return batteryInfo;
}

// New Functionality 3: List Running Processes
vector<string> getRunningProcesses() {
    vector<string> processes;
    int bufferSize = proc_listpids(PROC_ALL_PIDS, 0, nullptr, 0);
    if (bufferSize <= 0) return processes;

    pid_t pids[bufferSize / sizeof(pid_t)];
    proc_listpids(PROC_ALL_PIDS, 0, pids, sizeof(pids));

    for (int i = 0; i < sizeof(pids) / sizeof(pid_t); ++i) {
        if (pids[i] == 0) continue;

        char pathBuffer[PROC_PIDPATHINFO_MAXSIZE];
        if (proc_pidpath(pids[i], pathBuffer, sizeof(pathBuffer)) > 0) {
            processes.push_back(pathBuffer);
        }
    }

    return processes;
}

// New Functionality 4: Get System Uptime
string getSystemUptime() {
    struct timeval bootTime;
    size_t len = sizeof(bootTime);
    int mib[2] = {CTL_KERN, KERN_BOOTTIME};

    if (sysctl(mib, 2, &bootTime, &len, nullptr, 0) < 0) {
        return "Failed to retrieve system uptime";
    }

    time_t now = time(nullptr);
    time_t uptimeSeconds = now - bootTime.tv_sec;

    int days = uptimeSeconds / (24 * 3600);
    int hours = (uptimeSeconds % (24 * 3600)) / 3600;
    int minutes = (uptimeSeconds % 3600) / 60;

    return "System Uptime: " + to_string(days) + " days, " +
           to_string(hours) + " hours, " +
           to_string(minutes) + " minutes";
}

// New Functionality 5: Retrieve Motherboard Information
string getMotherboardInfo() {
    string motherboardInfo;
    motherboardInfo += "Hardware Model: " + getSysctlInfo("hw.model") + "\n";
    motherboardInfo += "Hardware Vendor: Apple\n"; // macOS systems are exclusively Apple hardware
    motherboardInfo += "Serial Number: " + getSysctlInfo("hw.serialno") + "\n";
    return motherboardInfo;
}

// New Functionality 6: Get CPU Core Count
string getCPUCoreCount() {
    int physicalCores = 0, logicalCores = 0;
    size_t len = sizeof(physicalCores);

    // Get physical core count
    sysctlbyname("hw.physicalcpu", &physicalCores, &len, nullptr, 0);

    // Get logical core count
    len = sizeof(logicalCores);
    sysctlbyname("hw.logicalcpu", &logicalCores, &len, nullptr, 0);

    return "Physical Cores: " + to_string(physicalCores) + ", Logical Cores: " + to_string(logicalCores);
}

// Main Function
int main() {
    // Example usage of all functions
    cout << "Graphics Cards:\n";
    cout << "- Not implemented on macOS\n"; // Graphics card info is not straightforward on macOS

    cout << "\nTotal System Memory: " << getTotalSystemMemory() << " MB\n";

    cout << "\nMAC Address: " << getMACAddress() << "\n";

    cout << "\nCPU Info: " << getCPUInfo() << "\n";

    cout << "\nOS Version: " << getOSVersion() << "\n";

    cout << "\nDisk Drives:\n";
    for (const auto& drive : getDiskDriveInfo()) {
        cout << "- " << drive << "\n";
    }

    cout << "\nMonitor Resolutions:\n";
    for (const auto& resolution : getMonitorResolutions()) {
        cout << "- " << resolution << "\n";
    }

    cout << "\nInstalled RAM Slots and Capacities:\n";
    for (const auto& ram : getInstalledRAMInfo()) {
        cout << "- " << ram << "\n";
    }

    cout << "\nBattery Status:\n" << getBatteryStatus() << "\n";

    cout << "\nRunning Processes:\n";
    for (const auto& process : getRunningProcesses()) {
        cout << "- " << process << "\n";
    }

    cout << "\nSystem Uptime:\n" << getSystemUptime() << "\n";

    cout << "\nMotherboard Information:\n" << getMotherboardInfo() << "\n";

    cout << "\nCPU Core Count:\n" << getCPUCoreCount() << "\n";

    return 0;
}