#pragma once
#include <string>
#include <ctime>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <vector>     // ADDED
#include <mutex>      // ADDED

using namespace std;

enum Status {
    READY,
    WAITING,
    RUNNING,
    TERMINATED
};

class ScreenFactory {
private:
    string name;
    string timeCreated;
    int    lineOfInstruction;
    int    totalLineofInstruction;
    Status status;

    /* ----------------------- NEW ----------------------- */
    vector<string> logs;   //  keeps all PRINT-generated lines
    mutex          log_mx; //  guards <logs>
    /* --------------------------------------------------- */

public:
    ScreenFactory(string name, int min_ins, int max_ins) {
        this->name = name;
        this->lineOfInstruction = 0;
        setTotalLineofInstruction(min_ins, max_ins);
        this->status = READY;
        initializeTimeCreated();
        /*  ⬆  no more file creation / “output” folder */
    }

    /* simple accessors */
    string getTime() { return timeCreated; }
    string getName() { return name; }
    Status getStatus()        const { return status; }
    int    getLineOfInstruction() { return lineOfInstruction; }
    int    getTotalLineofInstruction() { return totalLineofInstruction; }
    void   setStatus(Status s) { status = s; }

    /* NEW – used by process-smi */
    vector<string> getLogsCopy() {
        lock_guard<mutex> g(log_mx);
        return logs;          // copy is fine for read-only printing
    }

    /* called by a core thread */
    void print(int core) {
        if (status == RUNNING) {
            if (lineOfInstruction < totalLineofInstruction) {
                /* time-stamp */
                time_t now = time(0);
                tm localTime;
                localtime_s(&localTime, &now);

                char stamp[50];
                strftime(stamp, 50, "(%m/%d/%y %H:%M:%S %p)", &localTime);

                /* build log line */
                string entry =
                    string(stamp) + " Core:" + to_string(core) +
                    " \"Hello world from " + name + "!\"";

                /* store it thread-safely */
                {
                    lock_guard<mutex> g(log_mx);
                    logs.push_back(entry);
                }
                lineOfInstruction += 1;
            }

            if (lineOfInstruction >= totalLineofInstruction) {
                status = TERMINATED;
            }
        }
    }

    void setTotalLineofInstruction(int min_ins, int max_ins) {
        int range = max_ins - min_ins + 1;
        totalLineofInstruction = rand() % range + min_ins;
    }

private:
    void initializeTimeCreated() {
        time_t now = time(0);
        tm localTime;
        localtime_s(&localTime, &now);

        char buff[50];
        strftime(buff, 50, "(%m/%d/%y %H:%M:%S %p)", &localTime);
        timeCreated = buff;
    }
};
