#include <iostream>
#include <list>
#include <algorithm>
#include <mutex>
#include <thread>

using namespace std;

// Use mutexes for resource control
mutex mSearch;
mutex mInsert;
mutex mDelete;

// Executed by the searcher threads
// Return true if found, else false
bool searcher(int num, list<int> l){

    // Search for num
    auto it = std::find(l.begin(), l.end(), num);

    // Determine if found and output
    if (it != l.end()) {
        printf("A %d was found during a search", num);
        return false;
    }
    printf("A %d wasn't found during a search", num);
    return true;
}

// Executed by deleter threads. Must be run alone.
// Delete last node in list for simplicity
void deleter(list<int> l){
    l.erase(l.end());
}

// Executed by inserter threads. Cannot run together
// Insert into last for simplicity
void inserter(list<int> l, int num){
    l.push_back(num);
}

class Worker{
private:
	int id;

    // Hold a role:
    // Search: 1
    // Insert: 2
    // Delete: 3
	int role;

public:
	Worker(int input, int job){
		id = input;
        role = job;
	}

	int reserveSpot(){

        Switch(role){

            // Searcher thread
            case 1 : {
                if (mSearch.try_lock()){
                    mSearch.unlock(); // Searchers never lock the search mutex

                    if (mDelete.try_lock()) {// Searches can't happen with deletes
                        mDelete.unlock()
                        return 1;
                    }
                }
                return 0;
            }

            // Inserter Thread
            case 2 : {
                if (mInsert.try_lock()){
                    if (mDelete.try_lock()){ // Inserts can't happen with deletes
                        mDelete.unlock();
                        return 1;
                    }
                    else
                        mInsert.unlock()
                }
                return 0;
            }

            // Deleter Thread
            case 2 : {
                if (mSearch.try_lock()){ // No searches allowed
                    if(mInsert.try_lock()){ // No inserts allowed
                        if(mDelete.try_lock()){ // No other deletes allowed
                            return 1;
                        }
                        mInsert.unlock();
                    }
                    mSearch.unlock()
                }
                return 0;
            }
        }
	}

	void sleepTime(int seconds){
		usleep(seconds * 1000000);
	}

	void unlock(int number){
        Switch(role){
            case 1 : {
                // nothing to unlock
                break;
            }
            case 2 :{
                mInsert.unlock();
                break;
            }
            case 3 :{
                mDelete.unlock();
                mInsert.unlock();
                mSeach.unlock();
            }
		}
	}

    ////////////////////// Edited to here //////////////////////

	int doneRoutine(){
		if(workingOn == 1){
			isRunning1 = false;
			std::cout << "Thread " << id << " done with resource " << workingOn << std::endl << std::flush;
			workingOn = -1;
			if(isOverworked){
				doneRoutine();
				return 0;
			}
			else{
				unlock(1);
				displayStatus();
				return 0;
			}
		}
		else if(workingOn == 2){
			isRunning2 = false;
			std::cout << "Thread " << id << " done with resource " << workingOn << std::endl << std::flush;
			workingOn = -1;
			if(isOverworked){
				doneRoutine();
				return 0;
			}
			else{
				unlock(2);
				displayStatus();
				return 0;
			}
		}
		else if(workingOn == 3){
			isRunning3 = false;
			if(isOverworked){
				if(!isRunning1 && !isRunning2 && !isRunning3){
					isOverworked = false;
					std::cout << "Thread " << id << " done with resource " << workingOn << std::endl << std::flush;
					unlock(1);
					unlock(2);
					unlock(3);
					std::cout << "No longer overworked!" << std::endl << std::flush;
					workingOn = -1;
					displayStatus();
					return 0;
				}
				else{
					sleepTime(1);
					doneRoutine();
					return 0;
				}
			}
			else{
				std::cout << "Thread " << id << " done with resource " << workingOn << std::endl << std::flush;
				workingOn = -1;
				displayStatus();
				return 0;
			}
		}
		else{
			if(isOverworked){
				//We've finished work, now wait for other threads to finish
				sleepTime(1);
				doneRoutine();
				return 0;
			}
			else{
				displayStatus();
				return 0;
			}
		}
		return 1;
	}

	void doStuff(){
		//Snooze for a bit
		int seconds = rand() % 10 + 2;
		std::cout << "Thread " << id << " snoozing for " << seconds << std::endl << std::flush;
		sleepTime(seconds);

		//Try to reserve a spot
		int success = reserveSpot();

		if(success){
			//Act busy for a while
			seconds = rand() % 10 + 2;
			std::cout << "Thread " << id << " working for " << seconds << std::endl << std::flush;
			sleepTime(seconds);
			doneRoutine();
		}

		doStuff();
	}
};



int main() {
    list <int> a




    return 0;
}
