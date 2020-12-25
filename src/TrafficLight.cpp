#include <iostream>
#include <random>
#include "TrafficLight.h"
#include <chrono>
#include <thread>
#include <future>
#define   LOOP_UPDATE_DELAY 1
/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    std::unique_lock<std::mutex> uLock(_commlock);
    _condition.wait(uLock,[this]{return(!_queue.empty());});
    T retval=std::move(_queue.front());
    _queue.pop_front();
    return retval;
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> lckgrd(_commlock);
    _queue.emplace_back(msg);
    _condition.notify_one();
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
}


/* Implementation of class "TrafficLight" */

 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    while (true)
    {
        auto futuremsg= std::async(std::launch::async,&MessageQueue<TrafficLightPhase>::receive,&_queue) ;
        TrafficLightPhase resp= futuremsg.get();
        if(int(resp))
        break;

        std::this_thread::sleep_for(std::chrono::milliseconds(LOOP_UPDATE_DELAY));
        /* code */
    }
    
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases,this));
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{ 
    auto now_t=std::chrono::steady_clock::now();
    int random_wait_time= 4000+ rand()%2000;
    while (1)
    { 
        std::unique_lock<std::mutex> lck(_mutex);
        
        
        auto now_e=std::chrono::steady_clock::now()-now_t;
        if(random_wait_time<std::chrono::duration_cast<std::chrono::milliseconds>(now_e).count())
        {
        std::cout<<"time Elapsed since last iteration :"<< std::chrono::duration_cast<std::chrono::milliseconds>(now_e).count()<<std::endl;
        /* code */
       this->_currentPhase=TrafficLightPhase((this->_currentPhase+1)%2);
       auto ftr = std::async(std::launch::async,&MessageQueue<TrafficLightPhase>::send,& _queue,std::move(_currentPhase));
       now_t=std::chrono::steady_clock::now(); 
       lck.unlock();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(LOOP_UPDATE_DELAY));
    }

    
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
}

