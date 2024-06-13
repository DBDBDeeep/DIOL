//
// Created by 이해나 on 5/13/24.
//

#include "CompactionTest.h"



void CompactionTest::compactSSTables(std::vector<testSSTable>& normalTables, std::vector<testSSTable>& delayTables, const std::string& outputDir) {
    std::vector<testSSTable> compactionTables = normalTables;

    for (auto& compactionTable : compactionTables) {
        compactionTable.load();
    }

    for (auto& delayTable : delayTables) {
        delayTable.load();
        std::vector<std::pair<uint64_t, int>> remainingData;
        for (const auto& delayPair : delayTable.data) {
            bool inserted = false;
            for (auto& compactionTable : compactionTables) {
                if (delayPair.first >= compactionTable.minKey && delayPair.first <= compactionTable.maxKey) {
                    auto it = std::lower_bound(compactionTable.data.begin(), compactionTable.data.end(), delayPair,
                                               [](const std::pair<uint64_t, int>& a, const std::pair<uint64_t, int>& b) {
                                                   return a.first < b.first;
                                               });
                    compactionTable.data.insert(it, delayPair);
                    inserted = true;
                    break;
                }
            }
            if (!inserted) {
                remainingData.push_back(delayPair);
            }
        }
        delayTable.data = remainingData;  // Remaining data after compaction
    }

    // compaction된 SSTables 저장
    for (size_t i = 0; i < compactionTables.size(); ++i) {
        std::string outname = outputDir + "/CompactionSStable" + std::to_string(i + 1) + ".txt";
        compactionTables[i].save(outname);
    }

    // delayTable 저장
    for (size_t i = 0; i < delayTables.size(); ++i) {
        std::string delayOutname = outputDir + "/DelaySStable_compacted" + std::to_string(i + 1) + ".txt";
        delayTables[i].save(delayOutname);
    }
}



void CompactionTest::runCompaction() {
    std::cout << "runCompaction\n";

    std::string inputDir = "/home/haena/DBDBDeep/DIOL/src/test/SSTable";
    std::string outputDir = "/home/haena/DBDBDeep/DIOL/src/test/compactionSSTable";

    std::vector<std::string> normalFiles;
    std::vector<std::string> delayFiles;

    // SSTable 디렉토리에서 파일 목록 읽기
    for (const auto& entry : std::filesystem::directory_iterator(inputDir)) {
        std::string filename = entry.path().string();
        if (filename.find("NormalSStable") != std::string::npos) {
            normalFiles.push_back(filename);
        } else if (filename.find("DelaySStable") != std::string::npos) {
            delayFiles.push_back(filename);
        }
    }

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<testSSTable> normalTables;
    for (const auto& file : normalFiles) {
        normalTables.emplace_back(file);
    }

    std::vector<testSSTable> delayTables;
    for (const auto& file : delayFiles) {
        delayTables.emplace_back(file);
    }

    try {
        compactSSTables(normalTables, delayTables, outputDir);
        std::cout << "Compaction completed successfully." << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Error during compaction: " << ex.what() << std::endl;
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> elapsed = end - start;

    std::cout << "Compaction test : " << elapsed.count() << " ms\n";

    return;
}

