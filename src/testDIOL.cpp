//
// Created by 이해나 on 5/25/24.
//
#include "test/DataFactory.h"
#include "test/workload/Workload.h"
#include "test/CompactionTest.h"


int main(int argc, char* argv[]) {
    Workload workloadA(DBManager::getInstance(), MockDisk::getInstance());
    string initDataNum = argv[1]; //ex) "10000";
    string initFileName = argv[2]; //ex) "dataB_c20000000_d0.2";
    string workloadFileName = argv[3]; //ex) "workloadA_r0.3_i0.7_V2";

    string initFilePath = "../src/test/dataset/" + initFileName+".txt";
    string workloadFilePath = "../src/test/dataset/workload/" + workloadFileName+".txt";
    int halfLines = workloadA.extractHalfLinesFromFilename(initFilePath);

    std::list<Record> initWorkload = workloadA.readFileFromStart(initFilePath, halfLines);
    std::list<Record> mixedWorkload = workloadA.readFileWhole(workloadFilePath);

    workloadA.executeWorkload(initWorkload, false);
    initWorkload.clear();

    workloadA.executeWorkload(mixedWorkload, true);

    workloadA.printDelayData();

    // workloadA.deleteAllSSTable();
    // workloadA.makeSSTable();
    // CompactionTest compacton;
    // compacton.runCompaction();


}