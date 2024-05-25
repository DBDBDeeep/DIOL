#include "ActiveMemtableController.h"
#include "DBManager.h"

void ActiveMemtableController::insert(unsigned int key, int value) {
    if(!isDelayData(key)) {
        insertData(*activeNormalMemtable, key, value);
    }
    else {
        insertData(*activeDelayMemtable, key, value);
    }
}

bool ActiveMemtableController::isDelayData(unsigned int key) {
    if(activeNormalMemtable->mem.empty()){
        return false;
    }else {
        return activeNormalMemtable->mem.begin()->first > key;
    }
}

bool ActiveMemtableController::insertData(IMemtable& memtable, uint64_t key, int value){
    DBManager& dbmanager = DBManager::getInstance();
    if (memtable.isFull()) {
        try {
            IMemtable* newMemtable = dbmanager.transformM0ToM1(&memtable);
            newMemtable->setStartKey(key);
            newMemtable->put(key, value);
            return true;
        } catch (exception &e) {
            cerr << e.what() << "\n";
        }
    }
    memtable.put(key, value);

    return true;
}

IMemtable* ActiveMemtableController::updateNormalMem(int id) {
    activeNormalMemtable = new NormalMemtable(id);
}

IMemtable* ActiveMemtableController::updateDelayMem(int id) {
    activeDelayMemtable = new DelayMemtable(id);
}