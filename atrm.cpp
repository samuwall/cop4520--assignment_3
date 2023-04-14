 /*    
 *  Assignment 3: Problem 2
 *  ATRM -- Atmospheric Temperature Reading Module 
 *  Samu Wallace 
 *  COP4502, Dr. Parra
 */

#include <iostream>
#include <cstdio>
#include <ctime>
#include <chrono>
#include <random>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <algorithm>

#define MINUTE 1            // 1 = scale down runtime from hourly report to by the minute report instead

#define NUM_THREADS 8       // 8 sensors
#define NUM_READINGS 60     // 1 reading per minute/second
#define REPORT_INTERVAL 60  // report every hour/minute

struct temp_reading {
    int sens_id;
    int timestamp;
    int temp;  
};

std::mutex data_mutex;
std::condition_variable data_cv;
std::vector<temp_reading> shared_temp_data;

std::default_random_engine generator(std::time(0));
std::uniform_int_distribution<int> dist(-100, 70);

void sensor_task(int sens_id);
void report_task();

int main() {
    
    std::vector<std::thread> sensor;
    
    for (int i = 0; i < NUM_THREADS; i++) {
        sensor.emplace_back(sensor_task, i);
    }
    
    std::thread report_thread(report_task);
    
    for (auto& th : sensor) {
        th.join();   
    }
    
    report_thread.join();
    
    return 0;
}



void sensor_task(int sensor_id) {
    
    for (int i = 0; i < NUM_READINGS; i++) {
        
        data_mutex.lock();
        
        temp_reading reading;
        reading.sens_id = sensor_id;
        reading.timestamp = i;
        reading.temp = dist(generator);

        shared_temp_data.push_back(reading);

        data_mutex.unlock();
        data_cv.notify_all();

        // wait until next reading time
        #if MINUTE
            std::this_thread::sleep_for(std::chrono::seconds(1));
        #else
            std::this_thread::sleep_for(std::chrono::minutes(1));
        #endif
    }
    
}

void report_task() {
    std::unique_lock<std::mutex> lock(data_mutex); // unlocks automatically when object leaves scope

    for (int i = 0; i < NUM_READINGS; i += REPORT_INTERVAL) {
        // lock must be std::unique_lock for cv .wait() function
        data_cv.wait(lock, [i] { return shared_temp_data.size() >= NUM_THREADS * (i + REPORT_INTERVAL); }); // lambda functions!

        std::vector<int> temperature;
        
        for (const auto& reading : shared_temp_data) {
            if (reading.timestamp >= i && reading.timestamp < i + REPORT_INTERVAL) {
                temperature.push_back(reading.temp);
            }
        }

        /* sort is from <algorithm> */
        std::sort(temperature.begin(), temperature.end()); 

        #if MINUTE
            std::cout << "\nMinute report:\n\n";
        #else
            std::cout << "\nHourly report:\n\n";
        #endif
        std::cout << "\tTop 5 highest temperatures: ";
        for (int j = temperature.size() - 1; j >= std::max((int)(temperature.size()) - 5, 0); j--) {
            std::cout << temperature[j] << "F ";
        }
        std::cout << "\n";

        std::cout << "\tTop 5 lowest temperatures: ";
        for (int j = 0; j < std::min((int)(temperature.size()), 5); j++) {
            std::cout << temperature[j] << "F ";
        }
        std::cout << "\n";

        int max_diff = 0;
        int max_diff_interval_start = 0;

        for (int j = i; j < i + REPORT_INTERVAL - 10; ++j) {
            int min_temp = 70;
            int max_temp = -100;

            for (const auto& reading : shared_temp_data) {
                if (reading.timestamp >= j && reading.timestamp < j + 10) {
                    min_temp = std::min(min_temp, reading.temp);
                    max_temp = std::max(max_temp, reading.temp);
                }
            }

            int diff = max_temp - min_temp;
            if (diff > max_diff) {
                max_diff = diff;
                max_diff_interval_start = j;
            }
        }
        #if MINUTE
            std::cout << "\tLargest temperature difference observed in a 10-second interval: " << max_diff << "F (second " << max_diff_interval_start << " to " << max_diff_interval_start + 9 << ")\n";
        #else
            std::cout << "\tLargest temperature difference observed in a 10-minute interval: " << max_diff << "F (minute " << max_diff_interval_start << " to " << max_diff_interval_start + 9 << ")\n";
        #endif
    }
    
}
