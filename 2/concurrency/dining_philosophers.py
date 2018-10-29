import time
from random import randint
import threading
from threading import Thread, Lock, Condition

forks = [Lock() for _ in range(5)]
philosophers = [
    Philosopher('Musk', [0, 1]), 
    Philosopher('Shaggy', [1, 2]), 
    Philosopher('Spongebob', [2, 3]), 
    Philosopher('Bojack', [3, 4]),  
    Philosopher('Solaire', [4, 0])
]

# def display_forks():
#     for i,fork in enumerate(forks):
#         status = "available"
#         if(fork.locked()):
#             status = "unavailable"
#         print("fork:" + i + status, philosophers[i] + ": " + state, end=' ')
#     print(" ")

def display_forks():
    for i,fork in enumerate(forks):
        if(fork.locked() == False):
            print("| I \t del \t |", end=' ')
        else:
            print("| Occupied by:" , philosophers[i].name, "(", philosophers[i].state,") |", end=' ')
        if(i % 5 == 0):
            print(" ")

class Philosopher(Thread):
    def __init__(self, name, fork):
        Thread.__init__(self)
        self.name = name
        self.fork = fork
        self.state = "thinking"
    
    def run(self):  
        while(True):
            self.think()
            self.get_forks()
            display_forks()
            self.eat()
            self.put_forks()

    def think(self):
        self.state = "thinking"
        delay = randint(1, 20)
        print('Thinking time = ' + str(delay))
        time.sleep(delay)
        
    def get_forks(self):
        self.state = "waiting"

        if (self.fork[0] < self.fork[1]):
            forks[self.fork[0]].acquire(True)
            forks[self.fork[1]].acquire(True)
        else:
            forks[self.fork[1]].acquire(True)
            forks[self.fork[0]].acquire(True)

    def eat(self):
        #print('Got forks ---')
        self.state = "eating"
        delay = randint(2, 9)
        print('Eating time = ' + str(delay))
        time.sleep(delay)
        print(threading.current_thread())

    def put_forks(self):
        if forks[self.fork[0]].locked() and forks[self.fork[1]].locked():
            #print('Putting forks ---')
            forks[self.fork[0]].release()
            forks[self.fork[1]].release()

def main():

    display_forks()

    for ph in philosophers:
#        print(ph.name)
        ph.start()
    
    for ph in philosophers:
        ph.join()

if __name__ == "__main__":
    main()
