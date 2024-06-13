#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <unordered_map>
#include <vector>
#include <algorithm>
#include "memtable/IMemtable.h"
#include "DelayDetector.h"
#include "memtableController/ActiveMemtableController.h"
#include "memtableController/ImmutableMemtableController.h"
#include "CompactionController.h"
#include "FlushController.h"

class FlushController;

class DBManager {
public:
    static DBManager& getInstance(){
        static DBManager instance;
        return instance;
    }

    int currentId = 0;
    int getIdAndIncrement();

    bool insert(uint64_t key, int value);
    int readData(uint64_t key);
    map<uint64_t, int> range(uint64_t start, uint64_t end);
    IMemtable* transformM0ToM1(IMemtable* memtable); // normal? delay?
private:
    DBManager(): activeMemtableController(ActiveMemtableController::getInstance()),
                 immMemtableController(ImmutableMemtableController::getInstance()){
        currentId = 1;
        getIdAndIncrement();

        immMemtableController.setFlushController(new FlushController(immMemtableController));
    }
    DBManager(const DBManager&) = delete;
    void operator=(const DBManager&) = delete;


    ActiveMemtableController& activeMemtableController;
    ImmutableMemtableController& immMemtableController;
};


#endif // DBMANAGER_H