#include <unistd.h>
#include <mutex>
#include <thread>
#include <iostream>
#include <stdlib.h>

std::mutex spot1;
bool isRunning1 = false;
std::mutex spot2;
bool isRunning2 = false;
std::mutex spot3;
bool isRunning3 = false;

bool isOverworked = false;

class Worker{
private:
	int id;
	int workingOn = -1;

public: 
	Worker(int input){
		id = input;
	}

	int reserveSpot(){
		if(spot1.try_lock() && !isOverworked){
			workingOn = 1;
			isRunning1 = true;
			std::cout << "Spot1 taken by "<< id << std::endl << std::flush;
			displayStatus();
			return 1;
		}
		else if(spot2.try_lock() && !isOverworked){
			workingOn = 2;
			isRunning2 = true;
			std::cout << "Spot2 taken by "<< id << std::endl << std::flush;
			displayStatus();
			return 1;
		}
		else if(spot3.try_lock() && !isOverworked){
			isOverworked = true;
			workingOn = 3;
			isRunning3 = true;
			std::cout << "Spot3 taken by "<< id << std::endl << std::flush;
			std::cout << "Overworked!" << std::endl << std::flush;
			displayStatus();
			return 1;
		}
		else{
			workingOn = -1;
			std::cout << "Thread " << id << " unable to find open resource" << std::endl << std::flush;
			displayStatus();
			return 0;
		}
	}

	void sleepTime(int seconds){
		usleep(seconds * 1000000);
	}

	void unlock(int number){
		if(number == 1){
			spot1.unlock();
		}
		else if(number == 2){
			spot2.unlock();
		}
		else if(number == 3){
			spot3.unlock();
		}
		else{
			std::cout << "Something went wrong" << std::endl << std::flush;
		}
	}

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

int main( int argv, char* argc[]){

	/* initialize random seed: */
	srand (time(NULL));

	Worker* t1 = new Worker(1);
	Worker* t2 = new Worker(2);
	Worker* t3 = new Worker(3);
	Worker* t4 = new Worker(4);
	Worker* t5 = new Worker(5);

	std::thread thread_1(&Worker::doStuff, t1);
	std::thread thread_2(&Worker::doStuff, t2);
	std::thread thread_3(&Worker::doStuff, t3);
	std::thread thread_4(&Worker::doStuff, t4);
	std::thread thread_5(&Worker::doStuff, t5);

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
