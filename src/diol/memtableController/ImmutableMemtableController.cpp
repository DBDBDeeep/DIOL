#include "ImmutableMemtableController.h"
#include "../FlushController.h"

//// 로깅 ====
//list<string> ids;
//bool flag;
//// ======



map<uint64_t, int> ImmutableMemtableController::rangeInVector(uint64_t start, uint64_t end, vector<IMemtable*>& v){

    map<uint64_t, int> segment;
    


    for (auto imm : v) {
        if(imm->startKey > end || imm->lastKey < start)
            continue;
        imm->memTableStatus = READING;
        map<uint64_t, int> temp;
        for(const auto& entry : imm->mem){
            
            if(entry.first>=start && entry.first <= end){
                temp[entry.first] = entry.second;

            }
            
        }
        imm->increaseAccessCount(temp.size());
        segment.insert(temp.begin(), temp.end());

        imm->memTableStatus = IMMUTABLE;
    }

    return segment;
}



map<uint64_t, int> ImmutableMemtableController::range(uint64_t start, uint64_t end) {

    map<uint64_t, int> results;
    if(M1min > end || M1max < start){

    }else{

        map<uint64_t, int> segment = rangeInVector(start, end, normalImmMemtableList_M1);
        results.insert(segment.begin(), segment.end());
    }

    if(M2min > end || M2max < start){ 

    }else{

        map<uint64_t, int> segment = rangeInVector(start, end, normalImmMemtableList_M2);
        results.insert(segment.begin(), segment.end());
    }
    

    if(DM1min > end || DM1max < start){ 

    }else{

        map<uint64_t, int> segment = rangeInVector(start, end, delayImmMemtableList_M1);
        results.insert(segment.begin(), segment.end());
    }
    

    if(DM2min > end || DM2max < start){ 

    }else{
        map<uint64_t, int> segment = rangeInVector(start, end, delayImmMemtableList_M2);
        results.insert(segment.begin(), segment.end());

    }

    // 만약 start 범위가 disk일 가능성이 있다면?
    map<uint64_t, int> diskData;

    if(results.size()!=(end-start+1)){

        diskData=diskRange(start, end);
        results.insert(diskData.begin(), diskData.end());

        return results;
    }else{
        cout<<"disk range 안함 개꿀\n";
        return results;
    }


}

void ImmutableMemtableController::putMemtableToM1List(IMemtable* memtable) {

    //memtableMap[memtable->memtableId]=memtable;

    if(LIMIT_SIZE_NORMAL_M1*0.9 <= normalImmMemtableList_M1.size()){
        compaction();
    }
    if(LIMIT_SIZE_DELAY_M1*0.9 <= delayImmMemtableList_M1.size()){
        convertOldDelayToM2();
    }
    if (memtable->type == NI){
        normalImmMemtableList_M1.push_back(memtable);
        M1start[memtable->memtableId]=memtable->startKey;
        M1last[memtable->memtableId]=memtable->lastKey;
        M1min=setMin(M1start);
        M1max=setMax(M1last);
    }else if(memtable->type == DI){
        delayImmMemtableList_M1.push_back(memtable);
        DM1start[memtable->memtableId]=memtable->startKey;
        DM1last[memtable->memtableId]=memtable->lastKey;
        DM1min=setMin(DM1start);
        DM1max=setMax(DM1last);
    }
}

int ImmutableMemtableController::readInVector(uint64_t key, vector<IMemtable*>& v){

    for (auto imm : v) {

        MemTableStatus preStatus = imm->memTableStatus;
        imm->memTableStatus = READING;
        if(imm->startKey > key || imm->lastKey < key) continue;
        // 맵에서 키 검색
        auto it = imm->mem.find(key);
        if (it != imm->mem.end()) {
            LOG_ID(imm->memtableId);
            imm->increaseAccessCount(1);
            return it->second;  // 키를 찾았으면 값 반환
        }
        imm->memTableStatus = preStatus;
    }
    return NULL;
}

int ImmutableMemtableController::read(uint64_t key) {


    int value = readInVector(key, normalImmMemtableList_M1);

    if(value == NULL) {
        value = readInVector(key, normalImmMemtableList_M2);
        if(value == NULL) {
            value = readInVector(key, delayImmMemtableList_M1);
            if(value == NULL) {
                value = readInVector(key, delayImmMemtableList_M2);
                if(value == NULL){
                    return diskRead(key);
                }else{
                    return value;
                }
            }else{
                 return value;
            }
        }else{
            return value;
        }
    }else{
        return value;
    }
    

    
}

int ImmutableMemtableController::diskRead(uint64_t key){

    int value = disk.read(key);
    if(value!=NULL){
        diskReadCnt++;
        diskReadData++;
    }
    
    return disk.read(key);
}

map<uint64_t, int> ImmutableMemtableController::diskRange(uint64_t start, uint64_t end){

    map<uint64_t, int> diskData = disk.range(start, end);

    if(!diskData.empty()){

        diskReadData+=diskData.size();
        diskReadCnt++;

    }

    return diskData;
}

void ImmutableMemtableController::convertOldDelayToM2(){
    LOG_STR("ImmutableMemtableController::convertOldDelayToM2()");
    int min = std::numeric_limits<int>::max();
    IMemtable* target = nullptr;
    for (const auto memtable : delayImmMemtableList_M1) {
        if (memtable->ttl <= min) {
            min = memtable->ttl;
            target = memtable;
        }
    }
    cout<<"  ";
    transformM1toM2(target);
}

void ImmutableMemtableController::compaction() {

    CompactionController compactionController;
    IMemtable *normalMemtable = compactionController.checkTimeOut();

        LOG_STR("start까지 ㄱㅊ");

        if (!delayImmMemtableList_M1.empty()) {
            IMemtable *delayMemtable = compactProcessor->compaction(normalMemtable, delayImmMemtableList_M1);

            if (delayMemtable != nullptr) {
                delayMemtable->memTableStatus = COMPACTED;
                transformM1toM2(delayMemtable);
            }
        }

        transformM1toM2(normalMemtable);

}

void ImmutableMemtableController::erase(vector<IMemtable*>& v, IMemtable* memtable){
    for(auto it = v.begin(); it != v.end();){
        auto element = *it;
        if(element == memtable){
            v.erase(it);
            break;
        } else {
            it++;
        }
    }
    LOG_STR("erase 잘돼요");
}


void ImmutableMemtableController::transformM1toM2(IMemtable* memtable) {

    if(LIMIT_SIZE_NORMAL_M2*0.9 <= normalImmMemtableList_M2.size()){
        LOG_STR("==========full!!============");

            flushController->checkTimeout(N);

            /** thread */
        flushController->start(N);
    }
    //while(LIMIT_SIZE_DELAY_M2*0.8 <= delayImmMemtableList_M2.size()){
    if(LIMIT_SIZE_DELAY_M2*0.9<= delayImmMemtableList_M2.size()){
        LOG_STR("==========full!!============");
       

            flushController->checkTimeout(D);
        /** thread */
        flushController->start(D);

    }

    if (memtable->type == NI){

        erase(normalImmMemtableList_M1, memtable);
        memtable->initM2();
        decreaseTTL(normalImmMemtableList_M2);
        normalImmMemtableList_M2.push_back(memtable);
        M2start[memtable->memtableId]=memtable->startKey;
        M2last[memtable->memtableId]=memtable->lastKey;
        M2min=setMin(M2start);
        M2max=setMax(M2last);
        deleteMem(M1start, memtable->memtableId);
        deleteMem(M1last, memtable->memtableId);
        M1min=setMin(M1start);
        M1max=setMax(M1last);
    } else if (memtable->type == DI){
        erase(delayImmMemtableList_M1, memtable);
        memtable->initM2();
        decreaseTTL(delayImmMemtableList_M2);
        delayImmMemtableList_M2.push_back(memtable);
        DM2start[memtable->memtableId]=memtable->startKey;
        DM2last[memtable->memtableId]=memtable->lastKey;
        DM2min=setMin(DM2start);
        DM2max=setMax(DM2last);
        deleteMem(DM1start, memtable->memtableId);
        deleteMem(DM1last, memtable->memtableId);
        DM1min=setMin(DM1start);
        DM1max=setMax(DM1last);
    }
    else
        throw logic_error("ImmutableMemtableController::transformM1toM2 주소비교.. 뭔가 문제가 있는 듯 하오.");
}

// 모든 M1 memtable에 ttl--
void ImmutableMemtableController::decreaseTTL(vector<IMemtable*>& v) {
    for (auto imm : v) {
        imm->ttl--;
    }
}