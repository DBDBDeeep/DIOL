#include "DBManager.h"
#include "DelayDetector.h"

bool DBManager::insert(uint64_t key, uint64_t value){
    activeMemtableController.insert(key, value);
}
int DBManager::readData(uint64_t key){
    immMemtableController.read(key);
}
map<unsigned int, int> DBManager::range(uint64_t start, uint64_t end){
    immMemtableController.range(start, end);
}
//bool DBManager::isFull(IMemtable& memtable){
//
//}

int DBManager::getIdAndIncrement(){
    return ++currentId;
}

IMemtable* DBManager::transformM0ToM1(IMemtable* memtable) {
    // TODO (설명) : 싱글톤으로 함
    ActiveMemtableController& activeController = ActiveMemtableController::getInstance();
    ImmutableMemtableController& immutableController = ImmutableMemtableController::getInstance();

    uint64_t minKey = numeric_limits<uint64_t>::max();
    uint64_t maxKey = numeric_limits<uint64_t>::min();

    auto updateKeys = [&](auto memtablePtr) {
        for (const auto& entry : memtablePtr->mem) {
            if (entry.first < minKey) minKey = entry.first;
            if (entry.first > maxKey) maxKey = entry.first;
        }
        memtablePtr->initM1();
        if (minKey != numeric_limits<uint64_t>::max()) {
            memtablePtr->setStartKey(minKey);
        }
        if (maxKey != numeric_limits<uint64_t>::min()) {
            memtablePtr->setLastKey(maxKey);
        }
        immutableController.putMemtableToQueue(memtablePtr);
    };

    if (auto normalPtr = dynamic_cast<NormalMemtable*>(memtable)){
        immutableController.decreaseTTL();
        updateKeys(normalPtr);
        normalPtr->setDelayCount(DelayDetector::detect(normalPtr));
        IMemtable* activeNormalMemtable = activeController.updateNormalMem(getIdAndIncrement());
        activeNormalMemtable->setStartKey(maxKey);
        return dynamic_cast<NormalMemtable*>(activeNormalMemtable);
    } else if (auto delayPtr = dynamic_cast<DelayMemtable*>(memtable)){
        updateKeys(delayPtr);
        return dynamic_cast<NormalMemtable*>(activeController.updateDelayMem(getIdAndIncrement()));
    }else
        throw logic_error("DBManager::transformM0ToM1 주소비교.. 뭔가 문제가 있는 듯 하오.");
}