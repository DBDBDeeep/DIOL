#include "CompactionController.h"
#include "memtableController/ImmutableMemtableController.h"

IMemtable* CompactionController::checkTimeOut() {
    LOG_STR("CompactionController::checkTimeOut()");
    ImmutableMemtableController& immController = ImmutableMemtableController::getInstance();
    IMemtable* finalMem = nullptr;
    bool timeoutFound = false;
    if(!immController.normalImmMemtableList_M1.empty()){
        for (const auto memtable : immController.normalImmMemtableList_M1) {
            if (memtable->ttl == 0) {
                LOG_STR("compactionQueue.push - timeout");
                finalMem = memtable;
                timeoutFound = true;
            }
        }
    }

    if (!timeoutFound) {
        // 타임아웃된 Memtable이 없을 경우 가장 delay 추정치가 적은 Memtable을 찾아 queue에 삽입
        finalMem = findMemtableWithMinDelay();

            LOG_STR("compactionQueue.push - least delay");
            LOG_STR("immController.compactionQueue에서 erase 잘 끝냈어요.");

    }
    LOG_STR("immControll 잘 끝냈어요.");
    return finalMem;
}

// delay count가 가장 적은 memtable find
IMemtable* CompactionController::findMemtableWithMinDelay() {
    ImmutableMemtableController& immContoller = ImmutableMemtableController::getInstance();

    int minDelay = INT_MAX;
    IMemtable* imm = nullptr;
    if(!immContoller.normalImmMemtableList_M1.empty()) {
        for (const auto memtable: immContoller.normalImmMemtableList_M1) {
            if (auto normalPtr = dynamic_cast<NormalMemtable *>(memtable)) {
                if (normalPtr->delayCount < minDelay) {
                    minDelay = normalPtr->delayCount;
                    imm = normalPtr;
                }
            }
        }
    }
    return imm;
}