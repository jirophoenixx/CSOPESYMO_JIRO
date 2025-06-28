#include "Process.h"
#include "Scheduler.h"
#include "Config.h"
#include <iostream>
#include <string>
#include <vector>
#include <conio.h>
#include <thread>
#include <atomic>

int cpu_cycles = 0;
bool running = true;
std::atomic<bool> initialized(false);  // Track initialization status
Config* config = Config::getInstance();
ScreenManager* screens = nullptr; // Pointer to ScreenManager

void mainThread();
void cpuCycle();

std::thread main_worker(mainThread);
std::thread scheduler_start_thread;
std::thread cpu_cycle(cpuCycle);
std::atomic<bool> making_process(false);

// Clear screen function
void Clear() {
    system("CLS");
    std::cout << R"( 
 ___________ ____  ____  _____________  __
/ ____/ ___// __ \/ __ \/ ____/ ___/\ \/ /
/ /    \__ \/ / / / /_/ / __ / \__ \  \  /
/ /___ ___/ / /_/ / ____/ /___ ___/ /  / /
\____//____/\____/_/   /_____//____/  /_/              
                                                                                                                
    )" << std::endl;

    std::cout << "Welcome to CSOPESY OS Emulator!\n";
 
}

void initializeScreens() {
  
    int RR = 0;
    if (config->getSchedulerType() == "rr") RR = 1;

    screens = new ScreenManager(config->getNumCPU(), config->getDelayPerExec(), config->getQuantumCycles(), RR);
    if (screens) {
        
    }
    else {
        std::cout << "Failed to initialize ScreenManager.\n"; 
    }
}

void Screen(std::vector<std::string> inputBuffer) {
    if (inputBuffer.size() == 2 && inputBuffer[1] == "-ls") {
        screens->listScreens();
    }
    else if (inputBuffer.size() == 3) {
        std::string action = inputBuffer[1];
        std::string name = inputBuffer[2];

        if (action == "-r") {
            std::cout << "Restoring screen with name: " << name << "\n";
            if (!screens->sFind(name)) {
                std::cout << "Screen with the name: [" << name << "] does not exist.\n";
            }
            else {
                screens->displayScreen(name);
                screens->loopScreen(name);
                Clear();
            }
        }
        else if (action == "-s") {
            std::cout << "Creating a new screen with name: " << name << "\n";
            if (screens->sFind(name)) {
                std::cout << "Screen with the name: [" << name << "] already exists.\n";
            }
            else {
                screens->addScreen(name, config->getMinIns(), config->getMaxIns());
                screens->isInsideScreen(true);
            }
        }
        else {
            std::cout << "Unknown screen command action: " << action << "\n";
        }
    }
    else {
        std::cout << "Invalid screen command. Please provide -r or -s and a name.\n";
    }
}

void SchedulerStart(int batch_process_freq, int min_ins, int max_ins) {
    std::cout << "scheduler-start command recognized. Starting process generation.\n";
    if (!making_process.load()) {
        making_process.store(true);
        scheduler_start_thread = std::thread([=]() {
            int process_count = 0;
            while (making_process.load()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(batch_process_freq*1000));
                std::string process_name = "Process_" + std::to_string(process_count++);
                screens->addScreen(process_name, min_ins, max_ins);
            }
            });
    }
    else {
        std::cout << "Scheduler is already running.\n";
    }
}

void SchedulerStop() {
    std::cout << "scheduler-stop command recognized. Stopping process generation.\n";
    making_process.store(false);
    if (scheduler_start_thread.joinable()) {
        scheduler_start_thread.join();
    }
}

void Exit() {
    screens->shutdown();
    delete screens; // Clean up
    std::cout << "Exiting program.\n";
    running = false;
}

std::vector<std::string> split_sentence(std::string sen) {
    std::stringstream ss(sen);
    std::string word;
    std::vector<std::string> words;
    while (ss >> word) {
        words.push_back(word);
    }
    return words;
}

void mainThread() {
    std::string inputBufferB;
    std::vector<std::string> inputBuffer;
    bool input_done = false;
    Clear();

    while (running) {
        inputBuffer.clear();
        inputBufferB.clear();
        input_done = false;

        std::cout << "\nEnter a command: ";
        while (!input_done) {
            if (_kbhit()) {
                char ch = _getch();
                switch (ch) {
                case 8: // backspace
                    if (inputBufferB.size() > 0) {
                        inputBufferB.pop_back();
                        std::cout << "\b \b";
                    }
                    break;
                case 13: // enter
                    std::cout << "\n";
                    input_done = true;
                    break;
                default:
                    if (ch >= 32 && ch <= 126) {
                        inputBufferB.push_back(ch);
                        std::cout << ch;
                    }
                    break;
                }
            }
        }

        inputBuffer = split_sentence(inputBufferB);
        if (inputBuffer.size() <= 0) continue;

        std::string firstInput = inputBuffer[0];

        if (firstInput == "exit") {
            Exit();
        }
        else if (firstInput == "initialize") {
    
            try {
            
                config->loadConfig("config.txt"); // Load configuration file
             
                // Print out config values
                std::cout << "Configuration values:\n";
                std::cout << "  num-cpu: " << config->getNumCPU() << "\n";
                std::cout << "  scheduler: " << config->getSchedulerType() << "\n";
                std::cout << "  quantum-cycles: " << config->getQuantumCycles() << "\n";
                std::cout << "  batch-process-freq: " << config->getBatchProcessFreq() << "\n";
                std::cout << "  min-ins: " << config->getMinIns() << "\n";
                std::cout << "  max-ins: " << config->getMaxIns() << "\n";
                std::cout << "  delay-per-exec: " << config->getDelayPerExec() << "\n";

                if (config->isInitialized()) {
                    initialized.store(true);
                  
                    initializeScreens(); // Call to initialize screens after successful config load
                    std::cout << "Set Config successfully \n";
                }
                else {
                    std::cout << "Failed to initialize configuration.\n";
                }
            }
            catch (const ConfigException& e) {
                std::cerr << "Configuration error: " << e.what() << "\n";
            }
            catch (const std::exception& e) {
                std::cerr << "Unexpected error: " << e.what() << "\n"; // Catch any unexpected errors
            }
        }

        else if (initialized.load()) {
            if (firstInput == "screen") {
                Screen(inputBuffer);
            }
            else if (firstInput == "scheduler-start") {
                SchedulerStart(config->getBatchProcessFreq(), config->getMinIns(), config->getMaxIns());
            }
            else if (firstInput == "scheduler-stop") {
                SchedulerStop();
            }
            else if (firstInput == "clear") {
                Clear();
            }
            else if (firstInput == "report-util") {
                screens->report_util();
            }
            else {
                std::cout << firstInput << " is not a recognized command.\n";
            }
        }
        else {
            std::cout << "You must initialize first.\n";
        }
    }
}

void cpuCycle() {
    while (running) {
        cpu_cycles++;
        std::this_thread::sleep_for(std::chrono::milliseconds(config->getDelayPerExec()));
    }
}

int main() {
    std::thread cpu_cycle(cpuCycle); // Create CPU cycle thread
    main_worker.join();
    cpu_cycle.join(); // Wait for CPU cycle thread to finish
}
