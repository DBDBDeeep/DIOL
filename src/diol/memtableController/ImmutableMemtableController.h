#ifndef IMMUTABLEMEMTABLECONTROLLER_H
#define IMMUTABLEMEMTABLECONTROLLER_H

#include "../memtable/IMemtable.h"
#include "../CompactProcessor.h"
#include "../MockDisk.h"
#include "../CompactionController.h"
#include <iostream>
#include <queue>


#define LOG_STR(str) \
//    cout<<str<<endl;

#define LOG_ID(id) \
//    cout<<"(found in id:"<<id<<")\n";

using namespace std;

//static const int LIMIT_SIZE_M1 = 3;
//static const int LIMIT_SIZE_M2 = 3;

static const int LIMIT_SIZE_NORMAL_M1 = 5;
static const int LIMIT_SIZE_DELAY_M1 = 8;
//static const int LIMIT_SIZE_DELAY_M1 = 4;
static const int LIMIT_SIZE_NORMAL_M2 = 5;
static const int LIMIT_SIZE_DELAY_M2 = 8;
//static const int LIMIT_SIZE_DELAY_M2 = 3;

class FlushController;

class ImmutableMemtableController {
public:
    static ImmutableMemtableController& getInstance(){
        static ImmutableMemtableController instance;
        return instance;
    }

    map<int, uint64_t> M1start;
    map<int, uint64_t> M1last;
    map<int, uint64_t> M2start;
    map<int, uint64_t> M2last;
    map<int, uint64_t> DM1start;
    map<int, uint64_t> DM1last;
    map<int, uint64_t> DM2start;
    map<int, uint64_t> DM2last;
    int M1min=0;
    int M1max=0;
    int M2min=0;
    int M2max=0;
    int DM1min=0;
    int DM1max=0;
    int DM2min=0;
    int DM2max=0;
    int diskReadCnt=0;
    uint64_t diskReadData=0;
    vector<IMemtable*> normalImmMemtableList_M1; // limit LIMIT_SIZE_NORMAL_M1
    vector<IMemtable*> delayImmMemtableList_M1; // limit LIMIT_SIZE_DELAY_M1
    vector<IMemtable*> normalImmMemtableList_M2; // limit LIMIT_SIZE_NORMAL_M2
    vector<IMemtable*> delayImmMemtableList_M2; // limit LIMIT_SIZE_DELAY_M2
    queue<IMemtable*> compactionQueue; // if M1 full -> insert compaction Queue
    queue<IMemtable*> flushQueue;
    CompactProcessor* compactProcessor;
    FlushController* flushController = nullptr;
    MockDisk& disk;

    void deleteMem(map<int, uint64_t> imm,int id){
        imm.erase(id);

    }

    // 최소값을 반환하는 함수
uint64_t setMin(const std::map<int, uint64_t>& m) {
    if (m.empty()) {
        throw std::runtime_error("Map is empty");
    }
    int minKey = std::numeric_limits<int>::max();
    for (const auto& [key, value] : m) {
        if (key < minKey) {
            minKey = key;
        }
    }
    return minKey;
}

// 최대값을 반환하는 함수
uint64_t setMax(const std::map<int, uint64_t>& m) {
    if (m.empty()) {
        throw std::runtime_error("Map is empty");
    }
    int maxKey = std::numeric_limits<int>::min();
    for (const auto& [key, value] : m) {
        if (key > maxKey) {
            maxKey = key;
        }
    }
    return maxKey;
}


    //memtable list 관리
    // void deleteImm(int id){

    //     auto it = memtableMap.find(id);
    //     if (it != memtableMap.end()) {
    //         // map에서 항목을 삭제합니다.
    //         memtableMap.erase(it);
    //         //std::cout << "Deleted memtable with ID: " << id << std::endl;
    //     } else {
    //         std::cout << "ID: " << id << " not found in map." << std::endl;
    //     }

    // }

   // int readMap(uint64_t key, map<int, IMemtable*>& map);
   // map<uint64_t, int> rangeMap(uint64_t start, uint64_t end, map<int, IMemtable*>& map);

    int read(uint64_t key);
    map<uint64_t, int> range(uint64_t start, uint64_t end);
    void putMemtableToM1List(IMemtable*);
    void decreaseTTL(vector<IMemtable*>& v);
    void erase(vector<IMemtable*>& v, IMemtable* memtable);
    void transformM1toM2(IMemtable*);
    void setFlushController(FlushController* fc){
        this->flushController = fc;
    }
private:
    ImmutableMemtableController() : disk(MockDisk::getInstance()) {
        compactProcessor = new CompactProcessor();
    }

    ~ImmutableMemtableController(){
        delete flushController;
    }

    ImmutableMemtableController(const ImmutableMemtableController&) = delete;
    void operator=(const ImmutableMemtableController&) = delete;

    void compaction();
    void convertOldDelayToM2();
    int diskRead(uint64_t key);
    map<uint64_t, int> diskRange(uint64_t start, uint64_t end);
    map<uint64_t, int> rangeInVector(uint64_t start, uint64_t end, vector<IMemtable*>& v);
    int readInVector(uint64_t key, vector<IMemtable*>& v);
    
};

#endif // IMMUTABLEMEMTABLECONTROLLER_H
