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
			std::cout << "Spot1 taken by "<< id << std::endl;
			return 1;
		}
		else if(spot2.try_lock() && !isOverworked){
			workingOn = 2;
			std::cout << "Spot2 taken by "<< id << std::endl;
			return 1;
		}
		else if(spot3.try_lock() && !isOverworked){
			isOverworked = true;
			workingOn = 3;
			std::cout << "Spot3 taken by "<< id << std::endl;
			return 1;
		}
		else{
			workingOn = -1;
			std::cout << "Thread " << id << " unable to find open resource" << std::endl;
			return 0;
		}
	}

	void sleepTime(unsigned int microseconds){
		usleep(microseconds);
	}

	int doneRoutine(){
		//Set isRunning to false
		if(id == 1){
			isRunning1 = false;
		}
		else if(id == 2){
			isRunning2 = false;
		}
		else if(id == 3){
			isRunning3 = false;
		}
		else{
			std::cout << "Something went wrong" << std::endl;
		}

		//If all threads are now finished processing
		if(isOverworked){
			if(!isRunning1 && !isRunning2 && !isRunning3){
				isOverworked = false;
				return 0;
			}
			else{
				sleepTime(1000);
				doneRoutine();
			}
		}
		else{
			return 0; 
		}
	}

	void doStuff(){
		//Snooze for a bit
		int milliseconds = rand() % 10000 + 1;
		std::cout << "Thread " << id << " sleeping for " << milliseconds << std::endl;
		sleepTime(milliseconds);

		//Try to reserve a spot 
		int success = reserveSpot();

		if(success == 0){
			//Act busy for a while
			milliseconds = rand() % 10000 + 1;
			sleepTime(milliseconds);
			doneRoutine();
		}

		doStuff();
	}
};

int main( int argv, char* argc[]){

	/* initialize random seed: */
	srand (time(NULL));

	Worker* t1 = new Worker(1);
	// Worker* t2 = new Worker(2);
	// Worker* t3 = new Worker(3);
	// Worker* t4 = new Worker(4);
	// Worker* t5 = new Worker(5);

	std::thread thread_1(&Worker::doStuff, t1);
	// std::thread thread_2(&Worker::doStuff, t2);
	// std::thread thread_3(&Worker::doStuff, t3);
	// std::thread thread_4(&Worker::doStuff, t4);
	// std::thread thread_5(&Worker::doStuff, t5);

	thread_1.join();
	// thread_2.join();
	// thread_3.join();
	// thread_4.join();
	// thread_5.join();

	delete t1;
	// delete t2;
	// delete t3;
	// delete t4;
	// delete t5;

	return 0;
}
