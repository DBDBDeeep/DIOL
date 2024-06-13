//
// Created by 나경 on 2024/05/11.
//

#ifndef IOTDB_DBManager_WORKLOAD_H
#define IOTDB_DBManager_WORKLOAD_H

#include "../DataFactory.h"
#include "../../diol/DBManager.h"
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <utility>
#include <sstream>
#include <filesystem>
#include <regex>


class Workload {
public:
    vector<int> op;    //INSERT: 0, READ: 1, RANGE: 2
    deque<uint64_t> workloadKey;
    deque<pair<uint64_t, uint64_t> > workloadRangeKey;
    DBManager& tree;
    MockDisk& disk;



    Workload(DBManager& _tree, MockDisk& _disk) : tree(_tree), disk(_disk) {}
    Workload(const Workload&) = delete;
    int extractHalfLinesFromFilename(const string& filePath);
    void readLines(ifstream& file, std::list<Record>& dataset, int linesToRead);
    list<Record> readFileFromStart(const std::string& filePath, int linesToRead);
    list<Record> readFileWhole(const string& filePath);
    void executeInsertWorkload(list<Record>& dataset, int start, int end);
    void executeMixedWorkload(list<Record>& dataset, int start, int end);
    void executeWorkload(list<Record>& dataset, bool isMixedWorkload);


    void cleanup();
    DBManager* getTree();
    void deleteAllSSTable();
    void makeSSTable();
    void printDelayData();
private:
    int iteration=0;
};
#endif //IOTDB_LSM_WORKLOAD_H