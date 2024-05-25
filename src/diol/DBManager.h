#ifndef DBMANAGER_H
#define DBMANAGER_H

#include "memtable/IMemtable.h"
#include "memtableController/ActiveMemtableController.h"
#include "memtableController/ImmutableMemtableController.h"

class DBManager {
public:
    static DBManager& getInstance(){
        static DBManager instance;
        return instance;
    }
    int currentId = 0;
    int getIdAndIncrement();

    bool insert(uint64_t key, uint64_t value);
    int readData(uint64_t key);
    map<unsigned int, int> range(uint64_t start, uint64_t end);
//    bool isFull(IMemtable& memtable);
    IMemtable* transformM0ToM1(IMemtable* memtable); // normal? delay?
private:
    DBManager(): activeMemtableController(ActiveMemtableController::getInstance()),
                 immMemtableController(ImmutableMemtableController::getInstance()){
        currentId = 0;
    }
    DBManager(const DBManager&) = delete;
    void operator=(const DBManager&) = delete;

    ActiveMemtableController& activeMemtableController;
    ImmutableMemtableController& immMemtableController;
};


#endif // DBMANAGER_H