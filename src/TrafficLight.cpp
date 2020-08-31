#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a - DONE. The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> uLock(_mutex);
    _condition.wait(lock, [this] { return !_queue.empty(); });

    // remove last element from queue
    T msg = std::move(_queue.back());
    _queue.pop_back();

    return msg;

}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a - DONE. The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.emplace_back(msg);
    _condition.notify_one();

}


/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b - DONE. add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.

    while(true)
    {
        TrafficLightPhase phase = _message_queue.receive();
        if (phase == TrafficLightPhase::green)
        {
           return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b - DONE. Finally, the private method „cycleThroughPhases“ should be started in a thread 
    // when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
// https://knowledge.udacity.com/questions/149149

void TrafficLight::cycleThroughPhases()
{
    // FP.2a - DONE. Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    auto last_update = std::chrono::system_clock::now();
    long time_between_loops;
    int cycle_duration;

    // Cycle duration set to random number
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(4000, 6000);
    cycle_duration = dist(mt);

    while (true) {
        time_between_loops = std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::system_clock::now() - last_update)
                                .count();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // Update TrafficLightPhase if enough time has passed
        if (time_between_loops >= cycle_duration) {
            _currentPhase = _currentPhase == TrafficLightPhase::red
                            ? TrafficLightPhase::green
                            : TrafficLightPhase::red;

        _message_queue.send(std::move(_currentPhase));

        last_update = std::chrono::system_clock::now();
        cycle_duration = dist(mt);
        }
    }
    

}
