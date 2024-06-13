#include "MockDisk.h"

bool MockDisk::compaction() {
    doCompaction();
}

int MockDisk::read(uint64_t key) {

    //normal SStale 뒤지기
    for (auto ss : normalSSTables) {
        if(ss->startKey > key || ss->lastKey < key)
            continue;

        // 맵에서 키 검색
        auto it = ss->rows.find(key);
        if (it != ss->rows.end()) {
            LOG_STR("(found in normalSSTable:"+ to_string(ss->sstableId)+")");
            return it->second;  // 키를 찾았으면 값 반환
        }
    }

    //없으면 delay SStale 뒤지기
    for (auto ss : delaySSTables) {
        if(ss->startKey > key || ss->lastKey < key)
            continue;

        // 맵에서 키 검색

        auto it = ss->rows.find(key);
        if (it != ss->rows.end()) {
            LOG_STR("(found in delaySStale:"+ to_string(ss->sstableId)+")");
            return it->second;  // 키를 찾았으면 값 반환
        }
    }

    //없어요
    return NULL;
}

map<uint64_t, int> MockDisk::range(uint64_t start, uint64_t end) {

//    // 로깅
//    list<string> normalSSTableIds;
//    list<string> delaySSTableIds;

    map<uint64_t, int> results;

    for (auto ss : normalSSTables) {

        if(ss->startKey > end || ss->lastKey < start)
            continue;

        auto itStart = ss->rows.lower_bound(start); // start 이상의 첫 번째 요소를 찾음
        auto itEnd = ss->rows.upper_bound(end);     // end 이하의 마지막 요소의 다음 요소를 찾음

        for (auto it = itStart; it != itEnd; ++it) {
            results[it->first] = it->second;

        }

    }
    for (auto ss : delaySSTables) {

        if(ss->startKey > end || ss->lastKey < start)
            continue;
        
        auto itStart = ss->rows.lower_bound(start); // start 이상의 첫 번째 요소를 찾음
        auto itEnd = ss->rows.upper_bound(end);     // end 이하의 마지막 요소의 다음 요소를 찾음

        for (auto it = itStart; it != itEnd; ++it) {
            results[it->first] = it->second;

        }

    }
    // 로깅


    return results;
}

bool MockDisk::flush(IMemtable* memtable) {


    SSTable* newSSTable = new SSTable(memtable->memtableId);

    try {
        for (const auto& entry : memtable->mem) {
            newSSTable->put(entry.first, entry.second);
        }
    } catch (const std::exception& e) {
            std::cerr << "Exception caught in main: " << e.what() << std::endl;
    }


    newSSTable->setStartKey(newSSTable->rows.begin()->first);
    newSSTable->setLastKey(newSSTable->rows.rbegin()->first);

    if(memtable->type == NI){
        newSSTable->setType(N);
        normalSSTables.push_back(newSSTable);
        doFlush(memtable->mem.size());
        return true;
    } else{

        newSSTable->setType(D);
        delaySSTables.push_back(newSSTable);
        doFlush(memtable->mem.size());
        return true;
    }

    return false;
}

void MockDisk::printSSTableList() {

    LOG_STR("\n============NormalSSTable===========\n");
    for(auto table: normalSSTables){
        LOG_STR("[ " + to_string(table->sstableId)+ " ]  key: " +to_string(table->startKey)+ " ~ " + to_string(table->lastKey) + " | #cnt: " +to_string(table->rows.size()));
    }
    LOG_STR("\n============DelaySSTable===========\n");

    for(auto table: delaySSTables){
        LOG_STR("[ " + to_string(table->sstableId)+ " ]  key: " +to_string(table->startKey)+ " ~ " + to_string(table->lastKey) + " | #cnt: " +to_string(table->rows.size()));
    }

}
