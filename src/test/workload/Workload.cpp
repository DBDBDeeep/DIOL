#include "Workload.h"
#include <queue>

using namespace std;

int Workload::extractHalfLinesFromFilename(const string& filePath) {
    std::regex regexPattern(R"(_c(\d+))");
    std::smatch match;
    if (std::regex_search(filePath, match, regexPattern)) {
        int lineCount = std::stoi(match[1].str());
        return lineCount / 2;
    } else {
        cerr << "ERR: 파일 이름에서 줄 수를 추출할 수 없습니다: " << filePath << endl;
        return 0;
    }
}

void Workload::readLines(std::ifstream& file, std::list<Record>& dataset, int linesToRead) {
    std::string line;
    int lineCount = 0;
    cout<<"file 읽는 중\n";
    while (std::getline(file, line) && (linesToRead == -1 || lineCount < linesToRead)) {
        std::istringstream iss(line);
        std::string op;

        if (std::getline(iss, op, ',')) {
            Record record;
            record.op = op;
            if (op == "RANGE") {
                std::string startKeyStr, endKeyStr;
                if (std::getline(iss, startKeyStr, ' ') && std::getline(iss, endKeyStr)) {
                    record.start_key = std::stoull(startKeyStr);
                    record.end_key = std::stoull(endKeyStr);
                } else {
                    std::cerr << "ERR: 잘못된 형식의 RANGE 레코드입니다: " << line << std::endl;
                    continue;
                }
            } else {
                std::string keyStr;
                if (std::getline(iss, keyStr)) {
                    record.key = std::stoull(keyStr);
                } else {
                    std::cerr << "ERR: 잘못된 형식의 " << op << " 레코드입니다: " << line << std::endl;
                    continue;
                }
            }
            lineCount++;
            dataset.push_back(record);
        } else {
            std::cerr << "ERR: 잘못된 형식의 레코드입니다: " << line << std::endl;
        }

    }
}

list<Record> Workload::readFileFromStart(const std::string &filePath, int linesToRead) {
    std::list<Record> readFileDataset;
    ifstream file(filePath.c_str());
    if (!file.is_open()) {
        cerr << "ERR: 파일을 열 수 없습니다 " << filePath << endl;
        return readFileDataset;
    }
    readLines(file, readFileDataset, linesToRead);
    file.close();
    return readFileDataset;
}
list<Record> Workload::readFileWhole(const string& filePath) {
    std::list<Record> readFileDataset;
    ifstream file(filePath.c_str());
    if (!file.is_open()) {
        cerr << "ERR: 파일을 열 수 없습니다 " << filePath << endl;
        return readFileDataset;
    }
    readLines(file, readFileDataset, -1); // Read all lines
    file.close();
    return readFileDataset;
}

void Workload::executeInsertWorkload(list<Record>& dataset, int start, int end) {
    iteration = 0;

    for (const auto& record : dataset) {
        tree.insert(record.key, record.key * 2);
        iteration++;

    }

}

void Workload::executeMixedWorkload(list<Record>& dataset, int start, int end) {
    // << "Workload Mixed 작업 실행 시작\n";
    iteration=0;

    for (const auto& record : dataset) {

        if (record.op == "READ") {
            //cout<<"read\n";
           tree.readData(record.key);
        } else if (record.op == "RANGE") {
            //cout<<"Range\n";
           tree.range(record.start_key, record.end_key);
            
        } else if (record.op == "INSERT") {
            //cout<<"I";
           tree.insert(record.key, record.key * 2);
        } else {
            cerr << "ERR: 잘못된 형식의 레코드입니다: " << record.op << endl;
            return;
        }
        /** 진행률 출력 */
       //iteration++;


    }

}


void Workload::executeWorkload(list<Record>& dataset, bool isMixedWorkload) {
    // cout << "workload 실행 시작\n";
    if(!isMixedWorkload){
        executeInsertWorkload(dataset, 0, dataset.size());
        return;
    }else {
        auto start = chrono::high_resolution_clock::now();
        executeMixedWorkload(dataset, 0, dataset.size());
        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double, milli> elapsed = end - start;
        cout << "\nWorkload 실행 시간: " << elapsed.count() << " ms" << endl;
        return;
    }

}


void Workload::deleteAllSSTable() {
    std::string directoryPath = "/home/haena/DBDBDeep/DIOL/src/test/SSTable";
    try {
        // 디렉터리 내의 모든 파일 순회
        for (const auto& entry : filesystem::directory_iterator(directoryPath)) {
            filesystem::remove(entry.path()); // 파일 삭제
        }
    } catch (const std::filesystem::filesystem_error& e) {
        cerr << "Error: " << e.what() << endl;
    }
    directoryPath = "/home/haena/DBDBDeep/DIOL/src/test/compactionSSTable";
    try {
        // 디렉터리 내의 모든 파일 순회
        for (const auto& entry : filesystem::directory_iterator(directoryPath)) {
            filesystem::remove(entry.path()); // 파일 삭제
            cout<<"삭제\n";
        }
    } catch (const std::filesystem::filesystem_error& e) {
        cerr << "Error: " << e.what() << endl;
    }
}

void Workload::makeSSTable() {

    deleteAllSSTable(); //기존 파일 삭제

    cout<<"makeSSTable\n";

    int fileCounter=0;

    string filename="/home/haena/DBDBDeep/DIOL/src/test/SSTable/NormalSStable";

    MockDisk& disk = MockDisk::getInstance();

    for(auto sstable:disk.normalSSTables){

        filename="/home/haena/DBDBDeep/DIOL/src/test/SSTable/NormalSStable"+to_string(++fileCounter) + ".txt";

        ofstream outputFile(filename);
        if (!outputFile.is_open()) {
            cerr << "Failed to open output file: ??" << filename << endl;
            return;
        }

        outputFile<<sstable->rows.begin()->first<<"\t"<<sstable->rows.end()->first<<"\n"; //처음키, 마지막키
        for (const auto& pair : sstable->rows) {
            outputFile << pair.first << "\t" << pair.second << "\n";
        }

        outputFile.close();
    }

    fileCounter=0;
    


    
    for(auto sstable:disk.delaySSTables){

        filename="/home/haena/DBDBDeep/DIOL/src/test/SSTable/DelaySStable"+to_string(++fileCounter) + ".txt";

        ofstream outputFile(filename);
        if (!outputFile.is_open()) {
            cerr << "Failed to open output file: " << filename << endl;
            return;
        }

        outputFile<<sstable->rows.begin()->first<<"\t"<<sstable->rows.end()->first<<"\n"; //처음키, 마지막키
        for (const auto& pair : sstable->rows) {
            outputFile << pair.first << "\t" << pair.second << "\n";
        }

        outputFile.close();
    }

}



void Workload::printDelayData(){
    MockDisk& disk = MockDisk::getInstance();
    ActiveMemtableController& actCon = ActiveMemtableController::getInstance();
    ImmutableMemtableController& immCon = ImmutableMemtableController::getInstance();


    int D_memory = 0;
    int D_disk= 0;
    int N_disk = 0;
    int N_memory = actCon.activeNormalMemtable->mem.size();

    // normal memory
    for(auto memtable : immCon.normalImmMemtableList_M1){
        N_memory += memtable->mem.size();
    }
    for(auto memtable : immCon.normalImmMemtableList_M2){
        N_memory += memtable->mem.size();
    }

    //delay imm 세기

    int delayImmMemtableNum=0;
    for(auto memtable : immCon.delayImmMemtableList_M1){
        delayImmMemtableNum+=memtable->mem.size();

    }
    for(auto memtable : immCon.delayImmMemtableList_M2){
        delayImmMemtableNum+=memtable->mem.size();
    
    }


    //flush queue 세기
    std::queue<IMemtable*> tempQueue = immCon.flushQueue;
    while (!tempQueue.empty()) {
        IMemtable* memtable = tempQueue.front();
        if(memtable->type == DI){

            D_disk += memtable->mem.size();
        }else if(memtable->type == NI){

            N_disk += memtable->mem.size();
        }
        tempQueue.pop();
    }


    for( auto SSTable: disk.delaySSTables){
        D_disk+=SSTable->rows.size();
   
    }

    for( auto SSTable: disk.normalSSTables){
        N_disk+=SSTable->rows.size();
    }


    D_memory = delayImmMemtableNum + actCon.activeDelayMemtable->mem.size();

    cout<<"\ndisk read : "<< immCon.diskReadCnt<<"\n";
    cout<<"disk read data : "<< immCon.diskReadData<<" \n\n";

    cout<<"delay data in Memory : "<< D_memory<<"\n";
    cout<<"delay data in Disk : "<<D_disk<<"\n";
    cout<<"normal data in Disk : "<<N_disk<<"\n";
    cout<<"normal data in Memory : "<<N_memory<<"\n";
    cout<<"data in Disk "<<(D_disk+N_disk)<<"\n";
    cout<<"total delay: "<<D_memory+D_disk<<"\n";
    cout<<"total data: "<<D_memory+D_disk+N_disk+N_memory<<"\n";


}