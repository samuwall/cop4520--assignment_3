 /*    
 *  Assignment 3: Problem 1
 *  The Birthday Presents Party
 *  Samu Wallace 
 *  COP4502, Dr. Parra
 */

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <ctime>

struct Present {
    int tag;
    Present* next;
};

class ConcurrentLinkedList {
public:
    ConcurrentLinkedList() : head(nullptr) {} // constructor -- sets head = NULL

    void insert(int tag) {
        std::unique_lock<std::mutex> lock(data_mutex); // unique_lock unlocks automatically when leaves scope (more modern approach than .lock())
        Present* new_present = new Present{tag, nullptr};

        if (!head) {
            head = new_present;
            return;
        }

        if (head->tag > tag) {
            new_present->next = head;
            head = new_present;
            return;
        }

        Present* current = head;
        while (current->next && current->next->tag < tag) {
            current = current->next;
        }

        new_present->next = current->next;
        current->next = new_present;
    }

    bool find(int tag) {
        std::unique_lock<std::mutex> lock(data_mutex);
        Present* current = head;
        while (current) {
            if (current->tag == tag) {
                return true;
            }
            current = current->next;
        }
        return false;
    }

    bool remove(int tag) {
        std::unique_lock<std::mutex> lock(data_mutex);
        if (!head) {
            return false;
        }

        if (head->tag == tag) {
            Present* to_remove = head;
            head = head->next;
            delete to_remove;
            return true;
        }

        Present* current = head;
        while (current->next && current->next->tag != tag) {
            current = current->next;
        }

        if (!current->next) {
            return false;
        }

        Present* to_remove = current->next;
        current->next = current->next->next;
        delete to_remove;
        return true;
    }

private:
    Present* head;
    std::mutex data_mutex;
};

void servant(ConcurrentLinkedList& list, int id, int presents) {
    unsigned int thankyou = 0;
    unsigned int removed = 0;

    for (int i = 0; i < presents; ++i) {
        int tag = id * presents + i + 1;
        list.insert(tag);
        thankyou++;
        if (list.remove(tag)) {
            removed++;
        }
    }

    std::cout << "Servant " << id << " wrote " << thankyou << " thank you's, and removed " << removed << " presents\n";
}

int main() {
    int num_servants = 4;
    int num_presents_per_servant = 500000 / num_servants;

    ConcurrentLinkedList list;

    std::vector<std::thread> threads;
    
    int time_start = clock();
     
    for (int i = 0; i < num_servants; ++i) {
        threads.emplace_back(servant, std::ref(list), i, num_presents_per_servant);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    int time_end = clock();
    std::cout << "runtime: " << (time_end - time_start) / double(CLOCKS_PER_SEC) << "s\n";
    
    return 0;
}
