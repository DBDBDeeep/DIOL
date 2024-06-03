#ifndef MOCKDISK_H
#define MOCKDISK_H

#include <list>
#include "SSTable.h"
#include "memtable/IMemtable.h"
#include "SSTable.h"

#define LOG_STR(str) \
//    cout<<str<<endl;

#define LOG_ID(id) \
//    cout<<id<<endl;


struct MockDisk {
private:
    MockDisk() : flushCount(0), compactionCount(0), readCount(0){};
    int compactionCount;
    int flushCount;
    void doCompaction(){ compactionCount++; }
    void doCompaction(int cnt){ compactionCount += cnt; }
    void doFlush(){ flushCount++; }
    void doFlush(int cnt){ flushCount += cnt; }
public:
    static MockDisk& getInstance(){
        static MockDisk instance;
        return instance;
    }

    std::list<SSTable*> normalSSTables;
    std::list<SSTable*> delaySSTables;
    int readCount;

    bool compaction();
    int read(uint64_t key);
    map<uint64_t, int> range(uint64_t start, uint64_t end);
    bool flush(IMemtable* mem, Type t);
    void printSSTableList();
};

#endif // MOCKDISK_H
