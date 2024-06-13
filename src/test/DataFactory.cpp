

#include "DataFactory.h"


using namespace std;

std::mt19937 gen;
/**
 * double형을 string으로 변환하는 함수
 * */
std::string to_string_with_precision(double value, int precision) {
    std::ostringstream out;
    out.precision(precision);
    out << std::fixed << value;
    return out.str();
}
/**
 * opertor<< 오버로딩 함수
 * */
std:: ostream& operator<<(std::ostream& os, const Record& record) {
    if(record.op=="RANGE"){
        os << "op: " << record.op << ", start_key: " << record.start_key << ", end_key: " << record.end_key;
    }else{
        os << "op: " << record.op << ", key: " << record.key;
    }
    return os;
}
void DataFactory::generateO3Dataset(string& dataSetName, int dataNum, double outOfOrderRatio){
    string filePath = "../src/test/dataset/" + dataSetName + "_c" + to_string(dataNum) + "_d" + to_string_with_precision(outOfOrderRatio,2) + ".txt";
    outOfOrderCount = static_cast<int>(dataNum * outOfOrderRatio); // out of order 데이터 총 개수
    int segmentDataNum= outOfOrderCount/2;
    int numOfSegments= std::ceil(segmentDataNum/20.0);  //하이퍼파라미터

    cout <<"segment로 만들어지는 총 데이터 개수: " << segmentDataNum << endl;
    std::vector<std::vector<int>> outOfOrderKeysPerSegment(numOfSegments);
//    cout << ">> Generate O3 Dataset Progress \n\n";

    generateDelaySegments(outOfOrderKeysPerSegment, dataNum, numOfSegments, segmentDataNum);

    /**1~dataNum 범위 dataset 초기화
    * */
    std::set<int> dataSet;
    for (int i = 1; i <= dataNum; ++i) {
        dataSet.insert(i);
    }



    /**out of order segment에 포함되는 key들 찾아 dataSet에서 제거
     * */

    vector<int> removeSegmentKeys;
    for (const auto& segment : outOfOrderKeysPerSegment) {
        for (const auto& key : segment) {
            removeSegmentKeys.push_back(key);
        }
    }
    cout << "removeSegmentKeys size: " << removeSegmentKeys.size() << std::endl;

    // dataSet에서 outOfOrderKeys에 있는 요소 제거
    for (int key : removeSegmentKeys) {
        dataSet.erase(key);
    }

    cout<< "dataset size :"<<dataSet.size()<<endl;



    /**
     * single delay key 생성
     * */
    std::vector<int> copyDataSet(dataSet.begin(), dataSet.end());

    generateSingleDelayDataset(dataSet, copyDataSet, dataNum);
    //생성된 singleDelayKeys의 key를 기존 dataset에서 제거(중복 key 방지)

    for (const auto& it : singleDelayKeys) {
        dataSet.erase(it.second);
//        if (dataSet.erase(it.second) > 0) {
//            std::cout << "Erased value: " << it.second << std::endl;
//        } else {
//            std::cout << "Key not found: " << it.second << std::endl;
//        }
    }
//    cout <<"dataset size :"<<dataSet.size()<<endl;
//    cout <<"singleDelayKeys size"<<singleDelayKeys.size()<<endl;
//    cout<< "dataset size :"<<dataSet.size()<<endl;



    int lineToWrite = dataNum;
    writeToInitFile(filePath, dataSet, outOfOrderKeysPerSegment, lineToWrite);

}

/**
 * Segment단위 delay data 생성 함수
 * */
void DataFactory::generateDelaySegments(std::vector<std::vector<int>>& outOfOrderKeysPerSegment, int dataNum, int numOfSegments, int segmentDataNum) {
//    cout << ">> Generate Delay Segment Progress \n\n";
    std::random_device rd;
    std::mt19937 g(rd());
    /**out of order data 각 segment 크기 설정
     * */
    int count25Percent =(numOfSegments * 0.25);
    int count50Percent = (numOfSegments * 0.50);
    int totalAssigned = count25Percent*10 + count25Percent*30 + count50Percent*20;
    /**
     * 비율에 따라 세그먼트 크기 추가
     * */
    for (int i = 0; i < count25Percent; ++i) {
        sizes.push_back(10);
    }

    for (int i = 0; i < count25Percent; ++i) {
        sizes.push_back(30);
    }

    for (int i = 0; i < count50Percent; ++i) {
        sizes.push_back(20);
    }

    // 생성된 segment 개수와 총 segment개수 불일치할 경우 계산 보정
    if(totalAssigned < segmentDataNum) {
        sizes.push_back(segmentDataNum-totalAssigned);
    } else if(totalAssigned > segmentDataNum) {
        int diff = totalAssigned - segmentDataNum;
        sizes.back() -= diff; // 마지막 요소에서 뺄셈
    }

    std::shuffle(sizes.begin(), sizes.end(), g);



    /**out of order data segment에 들어갈 key 선정
     * */
    int distance = dataNum / numOfSegments; // delay segment 생성될 구간 설정
    int start=1;
    int end=distance;
    int delayedKey;
    for (size_t i = 0; i < sizes.size(); i++) {
        std::uniform_int_distribution<> nextDis(start, end-sizes[i]); // currentIdx 부터 distance-(생성될 segment 사이즈) 사이에서 seg에 들어갈 첫번째 delay key선정

        delayedKey = nextDis(g);


        for (int j = 0; j < sizes[i]; j++) {
            outOfOrderKeysPerSegment[i].push_back(delayedKey + j);
        }
        setSegmentDelayOffset(outOfOrderKeysPerSegment[i], dataNum);
        start = end+1;
        end = start+distance-1;

    }
//    cout << ">> Generate Delay Segment Complete \n\n";
}
/**
 * single delay data 생성 함수
 * */
std::map<int, int> DataFactory::generateSingleDelayDataset( set<int>& dataSet, vector<int> copyDataSet, int dataNum) {
//    cout << ">> Generate Single Delay Progress \n\n";

    std::random_device rd;
    std::mt19937 gen(rd());

    /**out of order 단일 key 선정
   * */



    int numberOfKeysToSelect = outOfOrderCount / 2;
    //numberOfKeysToSelect만큼의 single delay key 생성
    int distance = dataSet.size() / numberOfKeysToSelect; // delay segment 생성될 구간 설정
    int start=0;
    int end=distance;
    int delayIndex;
    unordered_set<int> copySingleDelayKeys;
    for(int i=0; i<numberOfKeysToSelect; i++){
        do{
            std::uniform_int_distribution<> dis(start, end);
            delayIndex = dis(gen);
        }while(copySingleDelayKeys.find(delayIndex) != copySingleDelayKeys.end());
        copySingleDelayKeys.insert(delayIndex);
        setSingleDelayOffset(copyDataSet[delayIndex], dataSet);

        start = end+1;
        end = start+distance-1;
        if ( i % (numberOfKeysToSelect / 100) == 0) {
            INT_LOG_PROGRESS(i, numberOfKeysToSelect);
        }
    }
    return singleDelayKeys;
}

void DataFactory::setSingleDelayOffset(int key,  set<int>& dataSet) {
    int randomChoice = rand() % 100 + 1;
    do{
        if (randomChoice <= 30) {
            randomIndex = key + (rand() % 500 + 1); // 1~500 범위 내
        } else if (randomChoice <= 60) {
            randomIndex = key + (rand() % 500 + 501); // 501~1000 범위 내
        } else if (randomChoice <= 80) {
            randomIndex = key + (rand() % 1000 + 1001); // 1001~2000 범위 내
        } else {
            randomIndex = rand() % dataSet.size() + key;
        }
    }while(randomIndexMap.find(randomIndex) != randomIndexMap.end());

    randomIndexMap.insert(randomIndex); //선택된 delay offset을 저장
    singleDelayKeys.insert({randomIndex, key});

}

/** Delay Segment Offset 설정 함수
 * */
void DataFactory::setSegmentDelayOffset(const vector<int> &segment, size_t dataSetSize) {

    std::uniform_int_distribution<> dis(1, 100); // 1~1000 범위 내 난수 생성
    int randomChoice = dis(gen);
    int isIndexValid=0;
    int randomIndex=0;

    //(30%, 30%, 20%, 20%) 비율에 따라 delay되는 offset 설정
    if (randomChoice <= 30) {
        uniform_int_distribution<> dis(1, 500);
        randomIndex = dis(gen); // 1~500 범위 내
        isIndexValid = segment.back() + randomIndex;
    } else if (randomChoice <= 60) {
        uniform_int_distribution<> dis(501, 1000);
        randomIndex = dis(gen); // 501~1000 범위 내
        isIndexValid = segment.back() + randomIndex;
    } else if (randomChoice <= 80) {
        uniform_int_distribution<> dis(1001, 2000);
        randomIndex = dis(gen); // 1001~2000 범위 내
        isIndexValid = segment.back()  + randomIndex;
    } else {
        randomIndex = rand() % dataSetSize + 1; // 1 ~ dataSet.size() 범위 내
        isIndexValid = segment.back() + randomIndex;
    }
    segmentDelayOffsets.push_back(isIndexValid);


}




/**
 * Workload에 추가할 Read, Range 작업 생성 함수
 * */
void DataFactory::generateReadRangeDataset(double readProportion, double insertProportion, double singleReadProportion, double rangeProportion, list<Record>& initDataSet) {
    cout<<"start"<<endl;

    int initFileRecordCount = initDataSet.size();
    int singleReadCount = initFileRecordCount * (readProportion / insertProportion) * singleReadProportion;
    int rangeCount = initFileRecordCount * (readProportion / insertProportion) * rangeProportion;
    //SingleRead 총 작업 생성
    for(int i=0; i< singleReadCount; i++){
        randomReadKey = rand() % initFileRecordCount + 1;
        Record record;
        record.key = randomReadKey;
        record.op = "READ";
        singleReadSet.insert({randomReadKey, record});
    }
    //RangeRead 총 작업 생성
    for(int i=0; i<rangeCount; i++) {
        int rangeStart = rand() % initFileRecordCount + 1;
        int rangeEnd = rand() % initFileRecordCount + 1;
        if (rangeStart > rangeEnd) {
            std::swap(rangeStart, rangeEnd);
        }
        Record record;
        record.start_key = rangeStart;
        record.end_key = rangeEnd;
        record.op = "RANGE";
        rangeSet.insert({rangeStart, record});
    }
    cout<<"end"<<endl;
}



void DataFactory::transferLinesToWorkloadFile(const std::string &initFilePath, string &workloadDataName, int linesToRead, double readProportion,
                                              double insertProportion, double singleReadProportion) {
    //read용 dataset input file 열기
    std::ifstream file(initFilePath.c_str());
    if (!file.is_open()) {
        cerr << "ERR: 파일을 열 수 없습니다 " << initFilePath << endl;
        return;
    }
    string version;

    if(singleReadProportion==0.5){
        version = "V1";
    }

    //write용 workload dataset output file 열기
    std::string outputFilePath = "../src/test/dataset/workload/"+workloadDataName+"_i"+to_string_with_precision(insertProportion,2)+"_r"+to_string_with_precision(readProportion,2)+"_"+version+"_"+initFilePath.substr(initFilePath.find_last_of("/")+1);
    std::ofstream outputFile(outputFilePath);
    if (!outputFile.is_open()) {
        std::cerr << "ERR: workload 파일 열기 오류" << outputFilePath << std::endl;
        return;
    }

    std::string line;
    int lineCount = 0;
    std::string op, key, start_key, end_key;
    while (lineCount < linesToRead && std::getline(file, line)) {
        lineCount++;
    }
    lineCount=0;
    int nextLogProgress = linesToRead / 10;
    int progressStep = linesToRead / 10;
    while (lineCount <= linesToRead && std::getline(file, line)) {
        std::istringstream iss(line);
        //input file에서 한 줄 읽어오기
        if (std::getline(iss, op, ',') && std::getline(iss, key)) {
            outputFile << op << "," << stoull(key) << std::endl;
        }
        //lineCount와  singleReadSet, rangeSet의 key를 비교하여 일치하는 경우 workload dataset에 추가
        if (singleReadSet.find(lineCount) != singleReadSet.end()) {
            //outputFile에 singleReadSet의 record.op와 record.key를 쓰기
            outputFile << "READ" << "," << lineCount << std::endl;
        } else if (rangeSet.find(lineCount) != rangeSet.end()) {
            outputFile << "RANGE" << "," << rangeSet[lineCount].start_key << " " << rangeSet[lineCount].end_key
                       << std::endl;
        }

        lineCount++;
        //진행률
        if (lineCount >= nextLogProgress) {
            FILE_LOG_PROGRESS(lineCount, linesToRead);
            nextLogProgress += progressStep;
        }
    }
}


void DataFactory::writeToInitFile(string filePath,  set<int>& dataSet, vector<std::vector<int>> segmentRandomKeys, int lineToWrite){

    ofstream outputFile(filePath);
    int lineCount = 0;
    //segmentRandomKeys 총 크기만큼 vec<int>와 offset을 저장할 map<int,vec<int>> 생성(vector<vector<int>>는 find로 lineCount비교 오래 걸려서)
    std::map<int,vector<int>> copyOutofOrderKeysPerSegment;
    auto offsetIdx=0;

    cout << "single delay key 개수" <<singleDelayKeys.size() << endl;
    cout << "segment 개수" <<segmentDelayOffsets.size() << endl;



    for(const auto &segment : segmentRandomKeys){

        for(const auto &key : segment){
            copyOutofOrderKeysPerSegment[segmentDelayOffsets[offsetIdx]].push_back(key);
        }
        offsetIdx++;
    }

    for(const auto &it : copyOutofOrderKeysPerSegment){
        if(it.first<it.second.back()){
            cout << "offset이 seg의 마지막 요소보다 작습니다. delay가 아닐 수 있음"<<endl;
            cout<<it.first<<","<<it.second.back()<<endl;
        }

    }

    if (!outputFile.is_open()) {
        cerr << "ERR: workload dataset 파일 열기 오류"  << endl;
        return;
    }
    std::cout << ">> Write to File Progress \n\n";


    auto singleIt = singleDelayKeys.begin();
    auto segIt = copyOutofOrderKeysPerSegment.begin();

    for (const auto& element : dataSet) {

        lineCount++;
        if(singleIt->first==lineCount){

            singleIt++;
        }

        if(segIt->first==lineCount){
            for(const auto &key : segIt->second){
//                outputFile << "INSERT," << key << std::endl;
            }
            segIt++;
        }


        if (lineCount % (lineToWrite / 10) == 0) {
            INT_LOG_PROGRESS(lineCount, lineToWrite);
        }
    }



    /**
     * singleDelayKeys에 남아있는 요소가 있다면 파일에 쓰기
     * */
    while((singleIt->first>lineCount)&& singleIt!=singleDelayKeys.end()){
        outputFile << "INSERT," << singleIt->second << std::endl;
        singleIt++;
    }


    /**
     * copyOutofOrderKeysPerSegment의 key가 lineCount보다 큰 경우 copyOutofOrderKeysPerSegment의 value를 파일에 쓰기
     * */
    while((segIt->first>lineCount)&& segIt!=copyOutofOrderKeysPerSegment.end()){
        for(const auto &key : segIt->second){
            outputFile << "INSERT," << key << std::endl;
        }
        segIt++;
    }

    cout<<"filewrite완료\n";
    outputFile.close();
}


/**File에 Workload 쓰기 함수*/
//void DataFactory::writeToWorkloadFile(const std::string& filePath, std::list<Record>& dataset) {
//    std::ofstream outputFile(filePath);
//    if (!outputFile.is_open()) {
//        std::cerr << "workload dataset 파일 열기 오류" << std::endl;
//        return;
//    }
//    std::cout << "\n>> Write to Workload File Progress \n";
//    size_t datasetSize = dataset.size();
//    iteration = 0;
//
//    for (const auto& record : dataset) {
//        if (record.op == "RANGE") {
//            outputFile << record.op << "," << record.start_key << " " << record.end_key << std::endl;
//        } else {
//            outputFile << record.op << "," << record.key << std::endl;
//        }
//        iteration++;
//        if ((iteration) % datasetSize == 0) {
//            INT_LOG_PROGRESS(iteration, datasetSize);
//        }
//    }
//
//    outputFile.close();
//}