#include "IMemtable.h"
#include <iostream>
NormalMemtable::NormalMemtable(int id) {
    this->state = M0;
    this->type = NI;
    this->startKey = 0;
    this->lastKey = static_cast<uint64_t>(-1);
    this->memtableId = id;
    this->memTableStatus = WORKING;
    //this->memtableSize = 32 * 1024 * 1024;
    this->memtableSize = 3.2 * 1024 * 1024;  //시연용
}

void NormalMemtable::setDelayCount(int cnt) {
    delayCount = cnt;
}


bool NormalMemtable::isFull(){
    size_t incomingDataSize = sizeof(uint64_t) + sizeof(int);
    return (getSize() + incomingDataSize) >= (memtableSize * 0.8);
}