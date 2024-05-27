#ifndef FLUSHCONTROLLER_H
#define FLUSHCONTROLLER_H

#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include "memtable/IMemtable.h"
#include "memtableController/ImmutableMemtableController.h"
#include "MockDisk.h"

using namespace std;

class FlushController {
public:
    FlushController() : running(false), disk(MockDisk::getInstance()),
//                        immMemtableController(ImmutableMemtableController::getInstance()),
                        taskCompleted(false) {}

    // 시작 메소드
    void start(Type t) {
        running = true;
        worker = std::thread(&FlushController::run, this, t);
//        flush_thread = std::thread(&FlushController::doFlush, this);
    }

    // 중지 메소드
    void stop() {
        running = false;
        if (worker.joinable()) {
            worker.join();
        }
//        if (flush_thread.joinable()) {
//            flush_thread.join();
//        }
    }

    // 작업 완료를 대기
    void waitForCompletion() {
        std::unique_lock<std::mutex> lock(mutex);
        condition.wait(lock, [this] { return taskCompleted; });
    }

    ~FlushController() {
        stop();
    }


private:
    std::atomic<bool> running;
    std::thread worker;
//    ImmutableMemtableController& immMemtableController;
    std::condition_variable condition;
    bool taskCompleted;
    mutex mutex;
    MockDisk& disk;

    IMemtable* findMemtableWithMinAccess(vector<IMemtable*> &v);
    void checkTimeout(Type t);

    void run(Type t){
//        if(immMemtableController.flushQueue.empty())
//            checkTimeout(t);
//        doFlush();
//        {
//            std::lock_guard<std::mutex> lock(mutex);
//            taskCompleted = true;
//        }
//        condition.notify_all();

    }

    void doFlush() {
//        if (!immMemtableController.flushQueue.empty()) {
//            IMemtable *memtable = immMemtableController.flushQueue.front();
//            immMemtableController.flushQueue.pop();
//            disk.flush(memtable);
//            delete memtable;
//        }
    }
};

#endif // FLUSHCONTROLLER_H