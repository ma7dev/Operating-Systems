#include <iostream>
#include <list>
#include <algorithm>
#include <mutex>
#include <thread>
#include <unistd.h>
#include <stdlib.h>

using namespace std;

// Use mutexes for resource control
mutex mSearch;
mutex mInsert;
mutex mDelete;

list <int> l;

mutex spot1;
bool isRunning1 = false;
mutex spot2;
bool isRunning2 = false;
mutex spot3;
bool isRunning3 = false;

bool isOverworked = false;

// Executed by the searcher threads
// Return true if found, else false
bool searcher(int num){

    // Search for num
    auto it = find(l.begin(), l.end(), num);

    // Determine if found and output
    if (it != l.end()) {
        //printf("A %d was found during a search.\n", num);
        return false;
    }
    //printf("A %d wasn't found during a search.\n", num);
    return true;
}

// Executed by deleter threads. Must be run alone.
// Delete last node in list for simplicity
void deleter(){
    if(l.size() > 0)
        l.erase(l.end());
}

// Executed by inserter threads. Cannot run together
// Insert into last for simplicity
void inserter(int num){
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
	int workingOn = -1;

public:
	Worker(int input, int job){
		id = input;
        role = job;
	}

	int reserveSpot(){
        switch(role){

            // Searcher thread
            case 1 : {
                if (mSearch.try_lock()){
                    mSearch.unlock(); // Searchers never lock the search mutex

                    if (mDelete.try_lock()) {// Searches can't happen with deletes
                        mDelete.unlock();
                        if(spot1.try_lock() && !isOverworked){
                            workingOn = 1;
                            isRunning1 = true;
                            cout << "Spot1 taken by "<< id << endl << flush;
                            return 1;
                        }
                        else if(spot2.try_lock() && !isOverworked){
                            workingOn = 2;
                            isRunning2 = true;
                            cout << "Spot2 taken by "<< id << endl << flush;
                            return 1;
                        }
                        else if(spot3.try_lock() && !isOverworked){
                            isOverworked = true;
                            workingOn = 3;
                            isRunning3 = true;
                            cout << "Spot3 taken by "<< id << endl << flush;
                            cout << "Overworked!" << endl << flush;
                            return 1;
                        }
                        else{
                            workingOn = -1;
                            cout << "Thread " << id << " unable to find open resource" << endl << flush;
                            return 0;
                        }
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
                        mInsert.unlock();
                }
                return 0;
            }

            // Deleter Thread
            case 3 : {
                if (mSearch.try_lock()){ // No searches allowed
                    if(mInsert.try_lock()){ // No inserts allowed
                        if(mDelete.try_lock()){ // No other deletes allowed
                            return 1;
                        }
                        mInsert.unlock();
                    }
                    mSearch.unlock();
                }
                return 0;
            }
        }
        return 0;
	}

	void sleepTime(int seconds){
		usleep(seconds * 1000000);
	}

	void unlock(int spot){
        switch(role){
            case 1 : {
                // nothing to unlock
                if(spot == 1){
                    spot1.unlock();
                }
                else if(spot == 2){
                    spot2.unlock();
                }
                else if(spot == 3){
                    spot3.unlock();
                }
                else{
                    cout << "Something went wrong" << endl << flush;
                }
                break;
            }
            case 2 :{
                if(spot == 1){
                    mInsert.unlock();
                    spot1.unlock();
                }
                else if(spot == 2){
                    mInsert.unlock();
                    spot2.unlock();
                }
                else if(spot == 3){
                    mInsert.unlock();
                    spot3.unlock();
                }
                else{
                    cout << "Something went wrong" << endl << flush;
                }
                break;
            }
            case 3 :{
                if(spot == 1){
                    mDelete.unlock();
                    mInsert.unlock();
                    mSearch.unlock();
                    spot1.unlock();
                }
                else if(spot == 2){   
                    mDelete.unlock();
                    mInsert.unlock();
                    mSearch.unlock();
                    spot2.unlock();
                }
                else if(spot == 3){
                    mDelete.unlock();
                    mInsert.unlock();
                    mSearch.unlock();
                    spot3.unlock();
                }
                else{
                    cout << "Something went wrong" << endl << flush;
                }
            }
		}
    }


	int doneRoutine() {

        if (role == 1){
            fprintf(stdout, "Searcher thread (id= %d ) finished.\n", id);
            if(workingOn == 1){
                isRunning1 = false;
                cout << "Thread " << id << " done with resource " << workingOn << endl << flush;
                workingOn = -1;
                if(isOverworked){
                    doneRoutine();
                    return 0;
                }
                else{
                    unlock(1);
                    return 0;
                }
            }
            else if(workingOn == 2){
                isRunning2 = false;
                cout << "Thread " << id << " done with resource " << workingOn << endl << flush;
                workingOn = -1;
                if(isOverworked){
                    doneRoutine();
                    return 0;
                }
                else{
                    unlock(2);
                    return 0;
                }
            }
            else if(workingOn == 3){
                isRunning3 = false;
                if(isOverworked){
                    if(!isRunning1 && !isRunning2 && !isRunning3){
                        isOverworked = false;
                        cout << "Thread " << id << " done with resource " << workingOn << endl << flush;
                        unlock(1);
                        unlock(2);
                        unlock(3);
                        cout << "No longer overworked!" << endl << flush;
                        workingOn = -1;
                        return 0;
                    }
                    else{
                        sleepTime(1);
                        doneRoutine();
                        return 0;
                    }
                }
                else{
                    cout << "Thread " << id << " done with resource " << workingOn << endl << flush;
                    workingOn = -1;
                    return 0;
                }
            }
            else{
                if(isOverworked){
                    //We've finished work, now wait for other threads to finish
                    sleepTime(1);
                    doneRoutine();
                }
                else{
                    return 0;
                }
            }
            return 1;

        }
        else if (role == 2){
            fprintf(stdout, "Inserter thread (id= %d ) finished.\n", id);
            if(workingOn == 1){
                isRunning1 = false;
                cout << "Thread " << id << " done with resource " << workingOn << endl << flush;
                workingOn = -1;
                if(isOverworked){
                    doneRoutine();
                    return 0;
                }
                else{
                    unlock(1);
                    return 0;
                }
            }
            else if(workingOn == 2){
                isRunning2 = false;
                cout << "Thread " << id << " done with resource " << workingOn << endl << flush;
                workingOn = -1;
                if(isOverworked){
                    doneRoutine();
                    return 0;
                }
                else{
                    unlock(2);
                    return 0;
                }
            }
            else if(workingOn == 3){
                isRunning3 = false;
                if(isOverworked){
                    if(!isRunning1 && !isRunning2 && !isRunning3){
                        isOverworked = false;
                        cout << "Thread " << id << " done with resource " << workingOn << endl << flush;
                        unlock(1);
                        unlock(2);
                        unlock(3);
                        cout << "No longer overworked!" << endl << flush;
                        workingOn = -1;
                        return 0;
                    }
                    else{
                        sleepTime(1);
                        doneRoutine();
                        return 0;
                    }
                }
                else{
                    cout << "Thread " << id << " done with resource " << workingOn << endl << flush;
                    workingOn = -1;
                    return 0;
                }
            }
            else{
                if(isOverworked){
                    //We've finished work, now wait for other threads to finish
                    sleepTime(1);
                    doneRoutine();
                }
                else{
                    return 0;
                }
            }
            return 1;

        }
        else if (role == 3){
            fprintf(stdout, "Deleter thread (id= %d ) finished.\n", id);
            if(workingOn == 1){
                isRunning1 = false;
                cout << "Thread " << id << " done with resource " << workingOn << endl << flush;
                workingOn = -1;
                if(isOverworked){
                    doneRoutine();
                    return 0;
                }
                else{
                    unlock(1);
                    return 0;
                }
            }
            else if(workingOn == 2){
                isRunning2 = false;
                cout << "Thread " << id << " done with resource " << workingOn << endl << flush;
                workingOn = -1;
                if(isOverworked){
                    doneRoutine();
                    return 0;
                }
                else{
                    unlock(2);
                    return 0;
                }
            }
            else if(workingOn == 3){
                isRunning3 = false;
                if(isOverworked){
                    if(!isRunning1 && !isRunning2 && !isRunning3){
                        isOverworked = false;
                        cout << "Thread " << id << " done with resource " << workingOn << endl << flush;
                        unlock(1);
                        unlock(2);
                        unlock(3);
                        cout << "No longer overworked!" << endl << flush;
                        workingOn = -1;
                        return 0;
                    }
                    else{
                        sleepTime(1);
                        doneRoutine();
                        return 0;
                    }
                }
                else{
                    cout << "Thread " << id << " done with resource " << workingOn << endl << flush;
                    workingOn = -1;
                    return 0;
                }
            }
            else{
                if(isOverworked){
                    //We've finished work, now wait for other threads to finish
                    sleepTime(1);
                    doneRoutine();
                }
                else{
                    return 0;
                }
            }
            return 1;
        }

        // Iterate and print values of the list
        // for (int n : l) {
        //     cout << n;
        // }
        // cout << endl << flush;
        //
        return 0;
    }



	void doStuff(){
		//Snooze for a bit
		int seconds = rand() % 10 + 2;
        if (role == 1){
            fprintf(stdout, "Searcher thread (id= %d ) snoozing for %d\n", id, seconds);

        }
        else if (role == 2){
            fprintf(stdout, "Inserter thread (id= %d ) snoozing for %d\n", id, seconds);
            //fflush(stdout);
        }
        else if (role == 3){
            fprintf(stdout, "Deleter thread (id= %d ) snoozing for %d\n", id, seconds);
            //fflush(stdout);
        }

		sleepTime(seconds);

		//Try to reserve a spot
		int success = reserveSpot();

		if(success){

            // Do task
            switch(role){

                // Search
                case 1 : {
                searcher(1);
                break;
                }

                // Insert
                case 2 : {
                inserter(1);
                break;
                }

                // Delete
                case 3 : {
                deleter();
                break;
                }
            }

			//Act busy for a while
			seconds = rand() % 10 + 2;

            if (role == 1){
                fprintf(stdout, "Searcher thread (id= %d ) working.\n", id);

            }
            else if (role == 2){
                fprintf(stdout, "Inserter thread (id= %d ) working.\n", id);
                //fflush(stdout);
            }
            else if (role == 3){
                fprintf(stdout, "Deleter thread (id= %d ) working.\n", id);
                //fflush(stdout);
            }

			sleepTime(seconds);
			doneRoutine();
		}

		doStuff();
	}
};



int main() {

    /* initialize random seed: */
	srand (time(NULL));

	Worker* t1 = new Worker(1,1);
	Worker* t2 = new Worker(2,1);
	Worker* t3 = new Worker(3,2);
	Worker* t4 = new Worker(4,2);
	Worker* t5 = new Worker(5,3);

	thread thread_1(&Worker::doStuff, t1);
	thread thread_2(&Worker::doStuff, t2);
    thread thread_3(&Worker::doStuff, t3);
	thread thread_4(&Worker::doStuff, t4);
	thread thread_5(&Worker::doStuff, t5);

	thread_1.join();
	thread_2.join();
	thread_3.join();
	thread_4.join();
	thread_5.join();

	delete t1;
	delete t2;
	delete t3;
	delete t4;
	delete t5;

    return 0;
}
