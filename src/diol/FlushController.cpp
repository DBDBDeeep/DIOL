#include "FlushController.h"

//#include "stddef.h"

void printFlushQueue(ImmutableMemtableController& imm){
    std::queue<IMemtable*> tempQueue = imm.flushQueue;  // 큐의 복사본 생성
    std::cout << "FlushQueue contains:" << std::endl;
    while (!tempQueue.empty()) {
        IMemtable* memtable = tempQueue.front();
        tempQueue.pop();
        std::cout << "Memtable INFO\n\tID: " << memtable->memtableId;
        std::cout << "\n\trange : " << memtable->startKey <<"~"<<memtable->lastKey;
        std::cout << "\n\tsize : " << memtable->mem.size()<<endl;

    }
}

// TODO : M2 리스트의 80% 이상이 찼을 경우 실행되는 메소드
void FlushController::checkTimeout(Type t) {
    LOG_STR("FlushController::checkTimeout()");
    ImmutableMemtableController& imm = ImmutableMemtableController::getInstance();
    bool timeoutFound = false;
    if(t==N) {
        for (const auto memtable: imm.normalImmMemtableList_M2) {
            if (memtable->ttl <= 0) {
                LOG_STR("1  flushQueue.push\n");
                imm.flushQueue.push(memtable);
                imm.erase(imm.normalImmMemtableList_M2, memtable);
                timeoutFound = true;

                imm.deleteMem(imm.M2start, memtable->memtableId);
                imm.deleteMem(imm.M2last, memtable->memtableId);
                imm.M2min=imm.setMin(imm.M2start);
                imm.M2max=imm.setMax(imm.M2last);
            }
        }
    }else{
    //if(t == D) {
        for (const auto memtable: imm.delayImmMemtableList_M2) {
            if (memtable->ttl <= 0) {
                LOG_STR("2  flushQueue.push\n");
                imm.flushQueue.push(memtable);
                imm.erase(imm.delayImmMemtableList_M2, memtable);
                timeoutFound = true;
                imm.deleteMem(imm.DM2start, memtable->memtableId);
                imm.deleteMem(imm.DM2last, memtable->memtableId);
                imm.DM2min=imm.setMin(imm.DM2start);
                imm.DM2max=imm.setMax(imm.DM2last);
            }
        }
    }
    if (!timeoutFound) {
        // 타임아웃된 Memtable이 없을 경우 access count가 가장 적은 Memtable을 찾아 queue에 삽입
        if(t==N){
            IMemtable* normalMem = findMemtableWithMinAccess(imm.normalImmMemtableList_M2);
            if(normalMem != NULL) {
                LOG_STR("3  flushQueue.push\n");
                imm.flushQueue.push(normalMem);
                imm.erase(imm.normalImmMemtableList_M2, normalMem);
            }
        }else{

            IMemtable *delayMem = findMemtableWithMinAccess(imm.delayImmMemtableList_M2);

            if (delayMem != NULL) {
                LOG_STR("4  flushQueue.push\n");
                imm.flushQueue.push(delayMem);
                imm.erase(imm.delayImmMemtableList_M2, delayMem);
            }
        }
    }
}

// access count가 가장 적은 memtable find
IMemtable* FlushController::findMemtableWithMinAccess(vector<IMemtable*>& v) {
    int minAccess = INT_MAX;
    IMemtable* imm = NULL;
    for (const auto memtable: v) {
        if (memtable->access < minAccess) {
            minAccess = memtable->access;
            imm = memtable;
        }
    }
    return imm;
}
