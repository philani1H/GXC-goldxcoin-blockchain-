// HardwareDetector.cpp
// Stub implementation for mining hardware detection.

#include "GXCMiner.h"
#include "blockchain.h"
#include <iostream>
#include <vector>

std::vector<MiningDevice> HardwareDetector::detectCPUs(Blockchain* blockchain) {
    std::cout << "[HardwareDetector] Detecting CPUs... Blockchain height: " << (blockchain ? blockchain->getHeight() : 0) << std::endl;
    return { {0, DeviceType::CPU, "Intel CPU", "Generic", 8192, 8, 3200, 40.0, 65.0, true, false, 0.0, 0, 0, 0, std::chrono::steady_clock::now()} };
}
std::vector<MiningDevice> HardwareDetector::detectNVIDIAGPUs() {
    std::cout << "[HardwareDetector] Detecting NVIDIA GPUs..." << std::endl;
    return { {1, DeviceType::GPU, "NVIDIA RTX 3080", "NVIDIA", 10240, 68, 1800, 55.0, 220.0, true, false, 0.0, 0, 0, 0, std::chrono::steady_clock::now()} };
}
std::vector<MiningDevice> HardwareDetector::detectAMDGPUs() {
    std::cout << "[HardwareDetector] Detecting AMD GPUs..." << std::endl;
    return { {2, DeviceType::GPU, "AMD RX 6800", "AMD", 16384, 60, 1700, 50.0, 180.0, true, false, 0.0, 0, 0, 0, std::chrono::steady_clock::now()} };
}
std::vector<MiningDevice> HardwareDetector::detectASICDevices() {
    std::cout << "[HardwareDetector] Detecting ASIC devices..." << std::endl;
    return { {3, DeviceType::ASIC, "Bitmain Antminer S19", "Bitmain", 0, 0, 0, 60.0, 3250.0, true, false, 0.0, 0, 0, 0, std::chrono::steady_clock::now()} };
}
std::vector<MiningDevice> HardwareDetector::detectAllDevices() {
    auto cpus = detectCPUs();
    auto nvidias = detectNVIDIAGPUs();
    auto amds = detectAMDGPUs();
    auto asics = detectASICDevices();
    std::vector<MiningDevice> all;
    all.insert(all.end(), cpus.begin(), cpus.end());
    all.insert(all.end(), nvidias.begin(), nvidias.end());
    all.insert(all.end(), amds.begin(), amds.end());
    all.insert(all.end(), asics.begin(), asics.end());
    return all;
}
std::string HardwareDetector::getCPUInfo() {
    return "Intel CPU, 8 cores, 3.2GHz";
}
std::string HardwareDetector::getGPUInfo(uint32_t deviceId) {
    return "GPU device info for device " + std::to_string(deviceId);
}
std::string HardwareDetector::getASICInfo(uint32_t deviceId) {
    return "ASIC device info for device " + std::to_string(deviceId);
}
bool HardwareDetector::isDeviceSupported(const MiningDevice& device, MiningAlgorithm algorithm) {
    return true;
}