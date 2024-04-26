//
// Created by 劉軒豪 on 2024/1/16.
//

#include "time_wheel.h"


time_wheel::time_wheel(int slot_number, int slot_interval) {
    current_slot = 0;
    this->slot_interval = slot_interval;
    slot = new tasklist_pointer[slot_number]{nullptr};
}

time_wheel::~time_wheel() {
    for (int i = 0; i < slot_number; i++) {
        tasklist_pointer temp_task_list = slot[i];
        task *temp_task_head = temp_task_list->head;
        task *temp_task = nullptr;
        while (temp_task_head) {
            temp_task = temp_task_head;
            temp_task_head = temp_task_head->next;
            delete temp_task->user_data;
            delete temp_task;
        }
        temp_task_list->head = nullptr;
        temp_task_list->tail = nullptr;
        delete temp_task_list;
        slot[i] = nullptr;
    }
}

void time_wheel::timer_start() {
    stop = false;
    current_point = time(nullptr);
    tick();
}


void time_wheel::timer_clear() {
    stop = true;
}


void time_wheel::tick() {
    while (!stop) {
        time_t current_time = time(nullptr);
        if (current_time != current_point) {
            iterate_task();
            current_slot = (++current_slot) / slot_number;
            current_point = current_time;
        }
        usleep(2000000);//0.2秒檢查一次，因為if(current_time !=current_point）需要執行時間

    }
}

void time_wheel::iterate_task() {

    tasklist_pointer temp_task_list = slot[current_slot];//取出當前指針的task
    tasklist_pointer remain_task_list = new tasklist{nullptr, nullptr};//儲存不是cycle==0的task

    slot[current_slot] = nullptr;
    task *temp = temp_task_list->head;
    while (temp) {
        //添加remain_task_list
        if (temp->cycle > 0) {
            temp->cycle--;//若當次cycle數不到，要先將其減1
            if (remain_task_list->head == nullptr) {
                remain_task_list->head = remain_task_list->tail = temp;
                remain_task_list->head->prev = nullptr;
            } else {
                remain_task_list->tail->next = temp;
                temp->prev = remain_task_list->tail;
                remain_task_list->tail = temp;
            }
        } else {
            //若是cycle==0執行任務
            temp->callback_function(temp->user_data);
            temp_task_list->head = temp->next;
            //刪除對應的task，包括task_map內與task本身
            task_map.erase(temp->user_data->client_fd);
            delete temp->user_data;
            delete temp;
            temp = temp_task_list->head;
        }
    }
    remain_task_list->tail->next = nullptr;
    temp_task_list->tail = nullptr;
    delete temp_task_list;
    //重新指定剩餘的task
    slot[current_slot] = remain_task_list;

}

void time_wheel::add_task(task_pointer new_task) {

    int slot_setting = new_task->time_slot;
    if (slot[slot_setting] == nullptr) {
        slot[slot_setting] = new tasklist{nullptr, nullptr};
        slot[slot_setting]->head = slot[slot_setting]->tail = new_task;
    } else {
        slot[slot_setting]->tail->next = new_task;
        new_task->prev = slot[slot_setting]->tail;
    }
    //添加任務對應
    task_map.insert(std::pair<int, task_pointer>(new_task->user_data->client_fd, new_task));
}

void time_wheel::delete_task(int clientfd) {
    //取得對應的task
    task *del_task = task_map[clientfd];
    //取得對應task的slot位置
    int check_slot = del_task->time_slot;
    task_map.erase(del_task->user_data->client_fd);
    //從tasklist中刪除task，要注意是否為head或tail
    if (del_task == slot[check_slot]->tail) {
        slot[check_slot]->tail = del_task->prev;
        slot[check_slot]->tail->next = nullptr;
        delete del_task->user_data;
        delete del_task;
    } else if (del_task == slot[check_slot]->head) {
        slot[check_slot]->head = slot[check_slot]->head->next;
        slot[check_slot]->head->prev = nullptr;
        delete del_task->user_data;
        delete del_task;

    } else {
        del_task->prev->next = del_task->next;
        del_task->next->prev = del_task->prev;
        delete del_task->user_data;
        delete del_task;
    }
}

void time_wheel::adjust_task(int clientfd, time_t reset_time) {

    delete_task(clientfd);





}
