//
// Created by 劉軒豪 on 2023/12/26.
//

#ifndef NGSASE_THREAD_POOL_H
#define NGSASE_THREAD_POOL_H

#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <type_traits>

typedef void(*workerFunction)(void *threadPool, void *workerData);

template<class T>
class ThreadPool {
    static_assert(std::is_pointer_v<T>, "ThreadPool<T> must be a pointer type");
public:
    ThreadPool();

    ~ThreadPool();

public:
    std::queue<T> taskQueue;
    std::vector<std::thread> threads;
    int minThreadNum = 0;
    int maxThreadNum = 0;
    int busyThreadNum = 0;
    int occupyThreadNum;
    int liveThreadNum = 0;
    int killThreadNum = 0;
    bool threadPoolShutdown = false;
    std::mutex threadPoolMutex;
    std::mutex busyThreadMutex;
    std::condition_variable taskQueueCV;

public:
    T fetchTask(bool &threadPooltermination, bool &busyThreadSetted);

    void addTask(T task);

    void workerThread(ThreadPool *threadPool, workerFunction, void *workerData);

    void threadManage(workerFunction, void *workerData, int timesFactor = 1);

    void startThreadManage(int microsecond, workerFunction workfunc, void *workerData, int timesFactor = 1);

    void shutdownThreadPool();

    void createThreadPool(int minThreadNum, int maxThreadNum, workerFunction workfunc, void *workerData);
};

//implementation
template<class T>
ThreadPool<T>::ThreadPool() {
    minThreadNum = 0;
    maxThreadNum = 0;
    busyThreadNum = 0;
    liveThreadNum = 0;
    killThreadNum = 0;
    occupyThreadNum = 0;
    threadPoolShutdown = false;

}

template<class T>
ThreadPool<T>::~ThreadPool() {
    std::unique_lock<std::mutex> threadPoolLock(threadPoolMutex);
    while (!taskQueue.empty()) {

        if (taskQueue.front()) {
            T task = taskQueue.front();
            delete task;
            task = nullptr;
            taskQueue.pop();
        }


    }
    threadPoolLock.unlock();
    if (!threads.empty()) {
        for (auto &t: threads) {
            t.join();
        }
        threads.clear();
    }


}

template<class T>
void ThreadPool<T>::workerThread(ThreadPool *threadPool, workerFunction workFunc, void *workerData) {
    workFunc((void *) threadPool, workerData);
}

/* fetchTask主要執任務如下
 * (1)從taskQueue中取出任務，修改busyThreadNumber數量後，return T*
 * (2)判定是否收到thread自我銷毀的訊號=>killThreadNum>0
 * (3)判定threadPool是否要完全關閉=>threadPoolShutdown=true
 * */
template<class T>
T ThreadPool<T>::fetchTask(bool &threadPooltermination, bool &busyThreadSetted) {
    /*因操作到threadPool，所以先需上鎖*/
    std::unique_lock<std::mutex> threadPoolLock(threadPoolMutex);

    T task = nullptr;
    /*若taskQueue是空的狀態，需使用condition-variable後，交還lock的所有權*/
    while (taskQueue.empty()) {
        taskQueueCV.wait(threadPoolLock);

        /* 被喚醒後需先判定是否收到自我銷毀的訊號或是threadPool要完全終止，
         * 若為終止則將threadPooltermination設為true後回傳nullptr，
         * 讓child thread判斷為需要終止thread
         * */
        if (killThreadNum > 0) {
            /* 因為是自我銷毀，所以killThreadNum--讓其歸零，
             * 並將liveThreadNum--，設置termination為true
             * 回傳nullptr*/

            killThreadNum--;
            liveThreadNum--;
            threadPoolLock.unlock();
            threadPooltermination = true;
            return nullptr;

        }

        if (threadPoolShutdown) {
            /*因為threadPool要終止，
             *直接將termination設為true
             *回傳nullptr*/
            threadPooltermination = true;
            threadPoolLock.unlock();
            return nullptr;
        }


    }
    /*若沒有shutdown則從taskQueue中取出task，
     *若有則將termination設為true後回傳nullptr*/
    if (!threadPoolShutdown) {
        task = taskQueue.front();
        taskQueue.pop();
    } else {
        threadPoolLock.unlock();
        threadPooltermination = true;
        return nullptr;
    }

    threadPoolLock.unlock();
    /*這邊需檢查是否已設定過busyThread，若無則要修改busyThreadNum數量*/
    if (busyThreadSetted == false) {
        std::unique_lock<std::mutex> busyThreadLock(busyThreadMutex);
        busyThreadNum++;
        busyThreadMutex.unlock();
        busyThreadSetted = true;
    }
    return task;

}

template<class T>
void ThreadPool<T>::addTask(T task) {
    std::unique_lock<std::mutex> threadPoolLock(threadPoolMutex);
    taskQueue.push(task);
    threadPoolLock.unlock();
    taskQueueCV.notify_one();
}

/*threadManage()主要的任務如下：
 *(1) 計算taskqueue內的任務數量後，判定是否添加thread數或是通知銷毀thread*/
template<class T>
void ThreadPool<T>::threadManage(workerFunction workfunc, void *workerData, int timesFactor) {
    if (!threadPoolShutdown) {
        std::unique_lock<std::mutex> threadPoolLock(threadPoolMutex);
        int busyThread = busyThreadNum;
        int liveThread = liveThreadNum;
        int queueSize = taskQueue.size();
        threadPoolLock.unlock();

        /*因為已將重要的相關參數取出，所以不用上lock，
         *而maxThreadNum因只有讀，所以不會有data race的情況出現*/
        if (queueSize > ((liveThread - busyThread) * timesFactor) && liveThread < maxThreadNum) {
            int addNumber = (queueSize - ((liveThreadNum - busyThreadNum) * timesFactor)) / timesFactor;
            /*因為要增加thread所以要鎖上*/
            threadPoolLock.lock();
            for (int i = 0; i < addNumber; i++) {
                if (liveThreadNum + i < maxThreadNum) {
                    threads.push_back(std::thread(&ThreadPool<T>::workerThread, this, this, workfunc, workerData));
                    liveThreadNum++;
                } else
                    break;
            }
            threadPoolLock.unlock();

        }

        if ((busyThread * 2) < liveThreadNum && liveThreadNum > minThreadNum) {
            threadPoolLock.lock();
            //之後視情況可以+=1
            killThreadNum=1;
            threadPoolLock.unlock();
            taskQueueCV.notify_one();
        }

    }


}

template<class T>
void ThreadPool<T>::startThreadManage(int microsecond, workerFunction workfunc, void *workerData, int timesFactor) {

    std::lock_guard<std::mutex> threadPoolLock(threadPoolMutex);
    this->threads.push_back(std::thread([this, &microsecond, &workfunc, &workerData, &timesFactor]() {
        while (!this->threadPoolShutdown) {
            this->threadManage(workfunc, workerData, timesFactor);
            std::this_thread::sleep_for(std::chrono::microseconds(microsecond));
        }
    }));
}

/* shutdownThreadPool()的主要任務如下：
 * (1) 修改threadPoolShutdown為true。
 * (2) 通知所有還在block的thread
 * (3) 執行後續thread.join跟清除剩餘的taskqueue內的task*/
template<class T>
void ThreadPool<T>::shutdownThreadPool() {
    std::unique_lock<std::mutex> shutdownThreadPool(threadPoolMutex);
    threadPoolShutdown = true;
    shutdownThreadPool.unlock();
    taskQueueCV.notify_all();
    ThreadPool<T>::~ThreadPool();

}

template<class T>
void ThreadPool<T>::createThreadPool(int minThreadNum, int maxThreadNum, workerFunction workfunc, void *workerData) {
    this->maxThreadNum = maxThreadNum;
    this->minThreadNum = minThreadNum;
    liveThreadNum = minThreadNum;
    for (int i = 0; i < minThreadNum; i++) {
        threads.push_back(
                std::thread(&ThreadPool<T>::workerThread, this, this, workfunc, workerData));
    }

}


#endif //NGSASE_THREAD_POOL_H
