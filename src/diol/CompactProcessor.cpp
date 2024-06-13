#include "CompactProcessor.h"
#include "memtableController/ImmutableMemtableController.h"

/**
 * param : memtable, delayMemtables\n
 * return : memtable의 시간 범위에 맞는 delayMemtable key의 개수
 * */
long checkTimeRange(IMemtable* memtable, IMemtable* delayMemtable){
    uint64_t s = memtable->startKey;
    uint64_t l = memtable->lastKey;
    long cnt = 0;
    for(auto pair : delayMemtable->mem){
        if(pair.first >= s && pair.first <= l){
            cnt++;
        }
    }
    return cnt;
}

void resetDelayStartLastKey(IMemtable* memtablePtr){
    uint64_t minKey = numeric_limits<uint64_t>::max();
    uint64_t maxKey = numeric_limits<uint64_t>::min();

    for (const auto& entry : memtablePtr->mem) {
        if (entry.first < minKey) minKey = entry.first;
        if (entry.first > maxKey) maxKey = entry.first;
    }
    
    if (minKey != numeric_limits<uint64_t>::max()) {
        memtablePtr->setStartKey(minKey);
    }
    if (maxKey != numeric_limits<uint64_t>::min()) {
        memtablePtr->setLastKey(maxKey);
    }
}

/** Delay Memtable에서 내부적으로 시간이 겹치는 data들을 찾아 부분 compaction을 진행한다. */
DelayMemtable* CompactProcessor::compaction(IMemtable* memtable, vector<IMemtable*>& delayMemtables) {
    IMemtable* delayTable = findTargetMem(memtable, delayMemtables);
    if(delayTable!=nullptr) {
        delayTable->memTableStatus = COMPACTING;
        memtable->memTableStatus = COMPACTING;
    }
    if(delayTable == nullptr){
        return nullptr;
    }

//    cout<<"compact delayTable  "<<delayTable->memtableId << ", size = "<<delayTable->mem.size() << "->";
    for(auto it = delayTable->mem.begin(); it!= delayTable->mem.end();){
        uint64_t delayKey = it->first;
        if(delayKey >= memtable->startKey && delayKey <= memtable->lastKey){
            memtable->mem.insert({delayKey, it->second});
            it = delayTable->mem.erase(it);
        }else{
            ++it;
        }
    }

    resetDelayStartLastKey(delayTable);
    return dynamic_cast<DelayMemtable*>(delayTable);
}

/** compaction을 진행할 DelayMemtable을 찾는다. */
IMemtable* CompactProcessor::findTargetMem(IMemtable* memtable, vector<IMemtable*>& delayMemtables) {
    ImmutableMemtableController& immutableMemtableController = ImmutableMemtableController::getInstance();
    uint64_t maxCnt = 0;
    IMemtable* target = nullptr;

    for (const auto delayMem : delayMemtables) { 
        // 다른 thread가 건드는 중이거나 후보라면 패스
        if(delayMem->memTableStatus == COMPACTING
            || delayMem->memTableStatus == WAITING_FOR_COMPACT) continue;

            long cnt = checkTimeRange(memtable, delayMem);
            if(cnt <= 0) continue;
            if(cnt > maxCnt){
                maxCnt = cnt;
                if(target != nullptr) target->memTableStatus = IMMUTABLE; // 후보에서 제외
                target = delayMem;
                target->memTableStatus = WAITING_FOR_COMPACT; // 후보 등록
            }else if(cnt == maxCnt){
                if(target != nullptr) target->memTableStatus = IMMUTABLE; // 후보에서 제외
                target = (target->ttl < delayMem->ttl) ? target : delayMem;
                target->memTableStatus = WAITING_FOR_COMPACT; // 후보 등록
            }
    }
    return target;
}
