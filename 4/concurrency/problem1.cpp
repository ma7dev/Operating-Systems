

class resource{
private:
	std::mutex spot1;
	std::mutex spot2;
	std::mutex spot3;

	bool isOverworked = false;

public: 
	int reserveSpot(){
		if(spot1.try_lock() && !isOverworked){
			return 1;
		}
		else if(spot2.try_lock() && !isOverworked){
			return 2;
		}
		else if(spot3.try_lock() && !isOverworked){
			isOverworked = true;
			return 3;
		}
	}

	bool isOverworked(){
		return isOverworked;
	}	


};




int main( int argv, char* argc[]){
	

	return 0;
}
