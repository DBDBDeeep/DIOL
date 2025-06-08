# ✨ DIOL
Minimized **D**isk **I/O** in **L**SM-tree
<br><br>

## 🔍 1. Problem

타임시리즈 데이터베이스에서는 **지연되어 들어오는 데이터(delay data)** 에 대한 정렬과 처리가 필수적입니다. <br>
하지만 현재 대부분의 TSDB는 내부 구조로 **LSM-tree**를 채택하고 있으며, 이는 다음과 같은 문제를 야기합니다:

- **Disk I/O 오버헤드**: delay data 정렬을 위한 Disk Compaction 과정에서 반복적인 Disk 접근 발생
- **SSD 수명 단축**: 과도한 Disk I/O로 인한 하드웨어 소모
<br>

## 💡 2. Solution
> Disk가 처리해야 할 delay data의 양을 줄여 Disk Compaction 시 발생하는 Disk I/O 오버헤드 최소화합니다.

- 이를 위해 다음 두 가지 기술을 제안합니다:
1. delay data를 LSM-tree의 메모리 레벨에서 최대한 처리
2. 자주 접근되는 데이터를 가진 메모리 테이블은 더 오래 유지, 그렇지 않은 테이블은 빠르게 교체

> 결과적으로, **Disk**에서 정렬해야 하는 delay data의 양을 줄이고, **Disk Compaction 시 발생하는 Disk I/O를 감소**시킬 수 있습니다.
<br>

## ⚙️ 3. 기술 개요

| 항목 | 설명 |
|------|------|
| 기반 구조 | LSM-tree |
| 핵심 아이디어 | delay data를 디스크가 아닌 **메모리에서 우선 정렬** |
| 메모리 정책 | LRU(Least Recently Used) |

<br><br>

## 🏗 4. 시스템 아키텍쳐
<img src="https://github.com/user-attachments/assets/a9f42091-b97d-462c-a996-ba6828569f03" alt="DIOL Class Diagram" width="70%">

1. M0 (Active Memtable)<br>
  새로운 데이터는 우선 M0의 Normal Memtable에 저장됩니다.
2. M1 (Immutable Memtable)<br>
  M0가 가득 차면 M1으로 이동되며, 이때 delay data는 시간 범위가 가장 많이 겹치는 Delay Immutable Memtable(DI)과 함께 compaction됩니다.
3. M2로 이동<br>
  Compaction이 완료된 Memtable은 M2로 내려갑니다.
4. Disk로 flush<br>
  M2의 Memtable은 조건을 만족하면 Disk로 flush되며,
  일반 데이터는 TsFile, delay data는 Delay TsFile로 나뉘어 저장됩니다.
<br><br>

## 🧩 5. 클래스 다이어그램
<img src="https://github.com/user-attachments/assets/9cd2fe77-c684-4aca-aa15-a6a034651e38" alt="DIOL Class Diagram" width="80%">


## 🔁 6. 시퀀스 다이어그램
| Data Insert | Read | Range |
|-------------|------|-------|
| <img src="https://github.com/user-attachments/assets/5b4e4e25-563a-4c48-bb86-571db6aac9f1"/> | <img src="https://github.com/user-attachments/assets/6bbea7d2-5a1c-4656-9f0d-00d2aba664bf"/> | <img src="https://github.com/user-attachments/assets/035a8a05-b5ce-467d-afde-449538d41c57"/> |

<br><br>

## ⚖️ 7. 비교군
> <img src="https://github.com/user-attachments/assets/e6971caa-c22a-4184-bf16-f0b09a2e6981" alt="IoTDB" width="10%">

비교 대상은 IoT 환경에서 널리 사용되는 TSDB인 **IoTDB**입니다.  
IoTDB는 LSM-tree 기반 구조로, delay data를 **disk compaction**으로 처리합니다.  
이 과정에서 발생하는 **I/O 병목** 가능성에 주목하여, IoTDB 구조에 솔루션을 적용해 성능을 비교 실험하였습니다.

### 🔍 IoTDB 구현 범위
<img src="https://github.com/user-attachments/assets/2e859a3c-505c-4872-be61-7cbe9d8e5fe6" alt="IoTDB" width="45%">

IoTDB는 Apache Confluence에 공개된 공식 자료를 기반으로 주요 구조를 재현했습니다.  
단, 본 프로젝트의 비교 목적에 맞추어 DIOL이 개선하고자 한 **메모리 레벨**에 한정하여 구현하였습니다.  
그 외의 인코딩, 스토리지 관리, Disk Compaction 등은 실험의 범위에서 제외하고 동일한 조건에서 비교가 가능하도록 하였습니다.


<br><br>
## 🚀 8. 실험 및 결과
### ⚙️ 실험 환경
> **소프트웨어**<br>
- DIOL과 IoTDB는 동일한 total memory(384MB) 조건하에 실험되었으며, 각 시스템은 memtable 구성 방식에서 차이를 가집니다.

|   | total memory | normal memtable size | delay memtable size | # of normal memtables  | # of delay memtables |
|--------|---------------|------------------------|------------------------|--------------------|--------------------|
| IoTDB  | 384MB         | 64MB                   | 64MB                   | x                  | 6−x                |
| DIOL   | 384MB         | 32MB                   | 2MB                    | 11                 | 16                 |

> **하드웨어**
- 운영체제 : ROCKY Linux 9.1
- 컴파일러 : gcc/g++ 11
- 개발 환경 :  Intel Xeon Gold 5318Y 프로세서|

> **데이터셋**
- data 개수 : 1억
- delay data 비율 : 10%, 20%, 30%

> **워크로드**

| Workload	| insert	| read/range |
|-----------|---------|------------|
|A|100%|0%|
|B|98%|2% (1:1)|
|C|95%|5% (1:1)|

insert 중심인 IoT 타임시리즈 환경을 고려하여 위와같이 workload를 구성하였습니다. 

---

### ⚖️ 동작 비교
> **1. Insert 동작 비교**

- O3(Out-of-order 데이터) <br> 메모리에 버퍼링된 데이터들을 순서대로 나열했을 때, 마지막으로 들어온 데이터의 timestamp 보다 이전의 timestamp 를 갖는 데이터가 들어오면, <br>해당 데이터를 Out-of-order 데이터라고 정의
- **Workload A 결과**
  | 시스템\O3 비율 | 10% | 20% | 30% |
  | --- | --- | --- | --- |
  | IoTDB | 18,890.9 ms | 22,708.5 ms | 26,022.7 ms |
  | DIOL | 20,855.8 ms | 22,888.5 ms | 28,563.9 ms |
  | 실행 시간 증가비율 | 9.8% | 0.8% | 8.9% |
  
- **분석**<br>
  DIOL은 메모리에서 delay data를 처리하는 구조를 추가하여 insert 시간에 소폭 증가가 있었지만, <br> disk compaction이 고려되지 않은 상태에서의 결과입니다. <br>
  실제 compaction 과정에서는 disk로 내려가는 delay data가 줄어들어 Disk I/O가 감소하고, 전체 성능이 더 향상될 수 있습니다. <br>증가된 insert 시간은 충분히 수용 가능한 수준입니다.

> **2. Read / Range 동작 비교**
- disk read: disk read 동작이 발생한 횟수
- disk read data: 읽어들인 disk data의 개수

- **Workload B 결과**
  |   | disk read | disk read data |
  | --- | --- | --- |
  | IoTDB | 990,248 | 726,717,909 |
  | DIOL | 961,049 | 686,628,468 |
  | 횟수 감소 비율 | 2.9% | 5.5% |

- **Workload C 결과**
  |   | disk read | disk read data |
  | --- | --- | --- |
  | IoTDB | 2,512,203 | 1,826,130,345 |
  | DIOL | 2,443,721 | 1,731,971,334 |
  | 횟수 감소 비율 | 2.7% | 5.2% |
  
- **분석**<br>
  DIOL은 flush 시 LRU 정책을 적용하여 자주 접근되는 Immutable Memtable이 메모리에 더 오래 유지되도록 설계되었습니다.  
  그 결과, IoTDB 대비 disk read 횟수와 데이터 양이 모두 감소했습니다.  
  실험 워크로드가 최신 데이터를 집중적으로 읽는 환경은 아니었기에, 실제 환경에서는 더욱 큰 효과를 기대할 수 있습니다.
<br><br>

## 🔧 9. Limitations & Future Work
-	**Disk Compaction 구현**<br>
  본 프로젝트에서는 disk compaction을 구현 범위에서 제외했지만, disk로 내려간 delay data의 양과 compaction 시간의 관계성과 정확한 trade-off를 분석할 수 있어 더 정확한 평가가 가능할 것으로 기대됩니다.
-	**Delay Memtable 크기 동적 조절**<br>
  실험 결과, O3 데이터 비율에 따라 delay Memtable의 크기와 개수를 조절하면 처리 효율이 향상됨을 확인했습니다. 이를 기반으로 동적으로 delay Memtable의 크기와 개수를 조절하는 기능을 추가하면 성능이 개선될 것이라 기대합니다.
- **실험 워크로드 개선 필요**<br>
  현재 실험에 사용된 워크로드는 최신 데이터를 집중적으로 읽지 않기 때문에, DIOL의 메모리 상 LRU 정책 효과가 충분히 발휘되지 않았습니다. 실제 IoT 환경에 더 가까운, 최근 데이터 중심의 워크로드를 설계하여 실험을 진행하면 더 정확한 평가가 가능할 것으로 기대됩니다.
<br><br>

## 📚 10. 참고 자료 및 출처
- **참고 자료** <br>
  [1] [IoTDB 공식 Confluence](https://cwiki.apache.org/confluence/display/IOTDB/Storage+Engine)<br>
  [2] [IoTDB User Guide](https://iotdb.apache.org/UserGuide/V1.2.x/QuickStart/QuickStart.html)<br>
  [3] [Apache IoTDB 오픈소스](https://github.com/apache/iotdb)<br>

- **논문**<br>
  [4] A Dataset and a Comparison of Out-of-Order Event Compensation Algorithms<br>
  [5] Separation or Not: On Handing Out-of-Order Time-Series Data in Leveled LSM-Tree
