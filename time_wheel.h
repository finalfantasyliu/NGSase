//
// Created by 劉軒豪 on 2024/1/16.
//

#ifndef NGSASE_TIME_WHEEL_H
#define NGSASE_TIME_WHEEL_H
#include <time.h>
#include <map>
#include <unistd.h>

class tasklist;

class task;

typedef task *task_pointer;
typedef tasklist *tasklist_pointer;

//client data用於callback_function使用
struct client_data {
    int client_fd;
    int kqfd;
};

//用一個tasklist來管理所有task，
//因為是任務導向，所以新增的任務會在tasklist的tail
struct tasklist {
    task *head;
    task *tail;
};


class task {

public:
    task(int cycle, int time_slot) : cycle(cycle), time_slot(time_slot), prev(nullptr), next(nullptr) {};

public:
    int cycle;//時間輪轉幾圈才會執行，避免當下指針指到slot進行iteration時，會將其執行
    int time_slot;//時間輪的slot位置
    client_data *user_data;

    void (*callback_function)(client_data *);//當指針指到slot，需調用的callback_function
    //前後task
    task *prev;
    task *next;
};




class time_wheel {
public:
    time_wheel(int slot_number, int slot_interval);

    ~time_wheel();

public:
    //新增task
    void add_task(task_pointer new_task);
    //刪除task
    void delete_task(int clientfd);
    //調整task，例如因read/write事件發生，所以須重新設定task的timer
    void adjust_task(int clientfd, time_t reset_time);
    //當指針指到slot時，需進行iteration並呼叫callback_function執行任務
    void iterate_task();
    //模擬指針運行
    void tick();
    //啟動timer
    void timer_start();
    //清除timer
    void timer_clear();


private:
    //當前指到的slot位置
    int current_slot;
    //total slot個數
    int slot_number;
    //每個slot的間隔時間
    int slot_interval;
    //停止tick()的bool值
    bool stop;
    //指針動之前的時間，用於檢測時間差
    time_t current_point;
    //用一個array維持一個時間輪
    tasklist_pointer *slot;
    //用map，加快刪除與調整task的運行，但切記std::map不是hashtable
    std::map<int, task_pointer> task_map;

};


#endif //SINGLE_THREAD_TIME_WHEEL_H
