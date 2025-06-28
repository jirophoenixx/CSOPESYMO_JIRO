#pragma once
#include <string>
#include <map>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>

class ConfigException : public std::runtime_error {
public:
    explicit ConfigException(const std::string& message) : std::runtime_error(message) {}
};

class Config {
private:
    int numCPU;                // Range: [1, 128]
    std::string scheduler_type; // Options: "fcfs" or "rr"
    uint32_t quantum_cycles;    // Range: [1, 2^32]
    uint32_t batch_process_freq; // Range: [1, 2^32]
    uint32_t min_ins;  // Range: [1, 2^32]
    uint32_t max_ins;  // Range: [1, 2^32]
    uint32_t delay_per_exec;    // Range: [0, 2^32]
    bool initialized = false;

    static Config* instancePtr;
    Config() {}

public:
    static Config* getInstance() {
        if (!instancePtr) {
            instancePtr = new Config();
        }
        return instancePtr;
    }

    void loadConfig(const std::string& filename);
    void validateParameters();
    bool isInitialized() const { return initialized; }
    int getNumCPU() const { return numCPU; }
    const std::string& getSchedulerType() const { return scheduler_type; }
    uint32_t getQuantumCycles() const { return quantum_cycles; }
    uint32_t getBatchProcessFreq() const { return batch_process_freq; }
    uint32_t getMinIns() const { return min_ins; }
    uint32_t getMaxIns() const { return max_ins; }
    uint32_t getDelayPerExec() const { return delay_per_exec; }
};

Config* Config::instancePtr = nullptr;

void Config::loadConfig(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw ConfigException("Could not open config file: " + filename);
    }

    std::map<std::string, bool> requiredParams = { // For missing parameters
        {"num-cpu", false},
        {"scheduler", false},
        {"quantum-cycles", false},
        {"batch-process-freq", false},
        {"min-ins", false},
        {"max-ins", false},
        {"delay-per-exec", false}
    };

    std::string param;
    while (file >> param) {
        if (param == "num-cpu") {
            file >> numCPU;
            requiredParams[param] = true;
        }
        else if (param == "scheduler") {
            file >> scheduler_type;
            requiredParams[param] = true;
        }
        else if (param == "quantum-cycles") {
            file >> quantum_cycles;
            requiredParams[param] = true;
        }
        else if (param == "batch-process-freq") {
            file >> batch_process_freq;
            requiredParams[param] = true;
        }
        else if (param == "min-ins") {
            file >> min_ins;
            requiredParams[param] = true;
        }
        else if (param == "max-ins") {
            file >> max_ins;
            requiredParams[param] = true;
        }
        else if (param == "delay-per-exec") {
            file >> delay_per_exec;
            requiredParams[param] = true;
        }
        else {
            throw ConfigException("Unknown parameter: " + param);
        }
    }

    // Check if all required parameters were found
    for (const auto& param : requiredParams) {
        if (!param.second) {
            throw ConfigException("Missing required parameter: " + param.first);
        }
    }

    validateParameters();
    initialized = true;
}

void Config::validateParameters() {
    if (numCPU < 1 || numCPU > 128) {
        throw ConfigException("Invalid number of CPUs (must be between 1 and 128): " + std::to_string(numCPU));
    }

    if (scheduler_type != "fcfs" && scheduler_type != "rr") {
        throw ConfigException("Invalid scheduler type (must be either 'fcfs' or 'rr'): " + scheduler_type);
    }

    if (quantum_cycles < 1) {
        throw ConfigException("Invalid quantum cycles (must be at least 1): " + std::to_string(quantum_cycles));
    }

    if (batch_process_freq < 1) {
        throw ConfigException("Invalid batch process frequency (must be at least 1): " + std::to_string(batch_process_freq));
    }

    if (min_ins < 1) {
        throw ConfigException("Invalid minimum instructions (must be at least 1): " + std::to_string(min_ins));
    }

    if (max_ins < min_ins) {
        throw ConfigException("Invalid maximum instructions (must be greater than or equal to min-ins): " + std::to_string(max_ins));
    }

    if (delay_per_exec < 0) {
        throw ConfigException("Invalid delays per execution (must be non-negative): " + std::to_string(delay_per_exec));
    }
}
