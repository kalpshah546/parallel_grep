#include "threadpool.h"
#include <functional>
using namespace std;

ThreadPool::ThreadPool(size_t n){
    stop=false;
    for(size_t i=0;i<n;i++){
        threads.emplace_back(
            [this](){
                function<void()>task;
                while(1){
                    unique_lock<mutex>lock(queueMutex);
                    cv.wait(lock,[this]{
                        return stop || !tasks.empty();
                    });
                    if(stop && tasks.empty()) return;
                    task=move(tasks.front());
                    tasks.pop();
                    lock.unlock();
                    task();
                }
            }
        );
    }
}
ThreadPool::~ThreadPool(){
    {
        unique_lock<mutex>lock(queueMutex);
        stop=true;
    }
    cv.notify_all();
    for(auto &th:threads){
        th.join();
    }

}


