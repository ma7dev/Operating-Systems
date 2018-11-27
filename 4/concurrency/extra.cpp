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
        l.pop_back();
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
                            fprintf(stdout, "Searching spot 1 taken by: thread%d.\n", id);
                            return 1;
                        }
                        else if(spot2.try_lock() && !isOverworked){
                            workingOn = 2;
                            isRunning2 = true;
                            fprintf(stdout, "Searching spot 2 taken by: thread%d.\n", id);
                            return 1;
                        }
                        else if(spot3.try_lock() && !isOverworked){
                            isOverworked = true;
                            workingOn = 3;
                            isRunning3 = true;
                            fprintf(stdout, "Searching spot 3 taken by: thread%d.\n", id);
                            fprintf(stdout, "Searching spots are all full!.\n");
                            return 1;
                        }
                        else{
                            workingOn = -1;
                            fprintf(stdout, "Search thread %d is unable to find a spot\n", id);
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
                else if(spot == 0){
                    //nothing to unlock
                }
                else{
                    fprintf(stdout, "Something went wrong :(\n");
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
                else if(spot == 0){
                    mInsert.unlock();
                }
                else{
                    fprintf(stdout, "Something went wrong :(\n");                }
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
                else if(spot == 0){
                    mDelete.unlock();
                    mInsert.unlock();
                    mSearch.unlock();
                }
                else{
                    fprintf(stdout, "Something went wrong :(\n");
                }
            }
		}
    }


	int doneRoutine() {

        if (role == 1){
            if(workingOn == 1){
                isRunning1 = false;
                fprintf(stdout, "Search thread %d done with resource %d.\n", id, workingOn);
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
                fprintf(stdout, "Search thread %d done with resource %d.\n", id, workingOn);
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
                        fprintf(stdout, "Search thread %d done with resource %d.\n", id, workingOn);
                        unlock(1);
                        unlock(2);
                        unlock(3);
                        fprintf(stdout, "No longer overworked!\n");
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
                    fprintf(stdout, "Search thread %d done with resource %d.\n", id, workingOn);
                    workingOn = -1;
                    return 0;
                }
            }
            else{
                unlock(0);
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
                fprintf(stdout, "Insert thread %d done with resource %d.\n", id, workingOn);
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
                fprintf(stdout, "Insert thread %d done with resource %d.\n", id, workingOn);
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
                        fprintf(stdout, "Insert thread %d done with resource %d.\n", id, workingOn);
                        unlock(1);
                        unlock(2);
                        unlock(3);
                        fprintf(stdout, "No longer overworked!\n");
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
                    fprintf(stdout, "Insert thread %d done with resource %d.\n", id, workingOn);
                    workingOn = -1;
                    return 0;
                }
            }
            else{
                unlock(0);
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
                fprintf(stdout, "Delete thread %d done with resource %d.\n", id, workingOn);
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
                fprintf(stdout, "Delete thread %d done with resource %d.\n", id, workingOn);
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
                        fprintf(stdout, "Delete thread %d done with resource %d.\n", id, workingOn);
                        unlock(1);
                        unlock(2);
                        unlock(3);
                        fprintf(stdout,"No longer overworked!");
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
                    fprintf(stdout, "Delete thread %d done with resource %d.\n", id, workingOn);
                    workingOn = -1;
                    return 0;
                }
            }
            else{
                unlock(0);
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

    // Test with 5 search threads
	Worker* t1 = new Worker(1,1);
	Worker* t2 = new Worker(2,1);
    Worker* t3 = new Worker(3,1);
    Worker* t4 = new Worker(4,1);
    Worker* t5 = new Worker(5,1);

    // Test with two insert threads
	Worker* t6 = new Worker(6,2);
	Worker* t7 = new Worker(7,2);

    // Test with two delete threads
	Worker* t8 = new Worker(8,3);
    Worker* t9 = new Worker(9,3);

	thread thread_1(&Worker::doStuff, t1);
	thread thread_2(&Worker::doStuff, t2);
    thread thread_3(&Worker::doStuff, t3);
	thread thread_4(&Worker::doStuff, t4);
	thread thread_5(&Worker::doStuff, t5);
    thread thread_6(&Worker::doStuff, t6);
    thread thread_7(&Worker::doStuff, t7);
    thread thread_8(&Worker::doStuff, t8);
    thread thread_9(&Worker::doStuff, t9);

	thread_1.join();
	thread_2.join();
	thread_3.join();
	thread_4.join();
	thread_5.join();
    thread_6.join();
    thread_7.join();
    thread_8.join();
    thread_9.join();

	delete t1;
	delete t2;
	delete t3;
	delete t4;
	delete t5;
    delete t6;
	delete t7;
	delete t8;
	delete t9;

    return 0;
}
