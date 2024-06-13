#include "ActiveMemtableController.h"
#include "../DBManager.h"

//int delayC = 0;
bool ActiveMemtableController::insert(uint64_t key, int value) {
    if(!isDelayData(key)) {
        return insertData(*activeNormalMemtable, key, value);
    }
    else {
        return insertData(*activeDelayMemtable, key, value);
    }
}

bool ActiveMemtableController::isDelayData(uint64_t key) {
    return activeNormalMemtable->startKey > key;
}

bool ActiveMemtableController::insertData(IMemtable& memtable, uint64_t key, int value){
    DBManager& dbmanager = DBManager::getInstance();
    if (memtable.isFull()) {
        try {
            dbmanager.transformM0ToM1(&memtable);
            if(activeNormalMemtable->startKey>key){  //delay data
                insertData(*activeDelayMemtable, key, value);
            }else{ //normal data
                activeNormalMemtable->memTableStatus = INSERTING;
                activeNormalMemtable->setStartKey(key);
                activeNormalMemtable->put(key, value);
                activeNormalMemtable->memTableStatus = WORKING;
            }
            return true;
        } catch (exception &e) {
            cerr << e.what() << "\n";
        }
    }

    memtable.memTableStatus = INSERTING;
    memtable.put(key, value);
    memtable.memTableStatus = WORKING;
    return true;
}

NormalMemtable* ActiveMemtableController::updateNormalMem(int id, uint64_t lastKey) {
    activeNormalMemtable = new NormalMemtable(id);
    activeNormalMemtable->setStartKey(lastKey);
    return activeNormalMemtable;
}

DelayMemtable* ActiveMemtableController::updateDelayMem(int id) {
    activeDelayMemtable = new DelayMemtable(id);
    return activeDelayMemtable;
}