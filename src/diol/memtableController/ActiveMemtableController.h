#ifndef ACTIVEMEMTABLECONTROLLER_H
#define ACTIVEMEMTABLECONTROLLER_H

#include "../memtable/IMemtable.h"
#include "../memtable/NormalMemtable.h"
#include "../memtable/DelayMemtable.h"

using namespace std;

class ActiveMemtableController {
public:
    static ActiveMemtableController& getInstance(){
        static ActiveMemtableController instance;
        return instance;
    }

    NormalMemtable* activeNormalMemtable;
    DelayMemtable* activeDelayMemtable;

    void insert(unsigned int key, int value);
    bool isDelayData(unsigned int key);
    bool insertData(IMemtable& memtable, uint64_t key, int value);
    IMemtable* updateNormalMem(int id);
    IMemtable* updateDelayMem(int id);
private:
    ActiveMemtableController(){
        activeNormalMemtable = new NormalMemtable(0);
        activeDelayMemtable = new DelayMemtable(1);
    }
    ActiveMemtableController(const ActiveMemtableController&) = delete;
    void operator=(const ActiveMemtableController&) = delete;
};

#endif // ACTIVEMEMTABLECONTROLLER_H