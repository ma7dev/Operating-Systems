import time
import threading
from random import randint
from threading import Thread, Lock, Condition

forks = [Lock() for _ in range(5)]

class Philosopher(Thread):
    def __init__(self, name, fork):
        Thread.__init__(self)
        self.name = name
        self.fork = fork
        self.state = "Thinking"
    
    def run(self):  
        while(True):
            self.think()
            self.get_forks()
            self.eat()
            self.put_forks()

    def think(self):
        self.state = "Thinking"
        delay = randint(1, 20)
        print(self.name + ': \t Thinking time = ' + str(delay))
        time.sleep(delay)
        
    def get_forks(self):
        global occupied
        self.state = "Waiting"
        if (self.fork[0] < self.fork[1]):
            forks[self.fork[0]].acquire(True)
            print(self.name + ': \t got the fork - ' + str(self.fork[0]))
            forks[self.fork[1]].acquire(True)
            print(self.name + ': \t got the fork - ' + str(self.fork[1]))
        else:
            forks[self.fork[1]].acquire(True)
            print(self.name + ': \t got the fork - ' + str(self.fork[1]))
            forks[self.fork[0]].acquire(True)
            print(self.name + ': \t got the fork - ' + str(self.fork[0]))


    def eat(self):
        global occupied
        self.state = "Eating"
        delay = randint(2, 9)
        print(self.name + ': \t Eating time = ' + str(delay))
        time.sleep(delay)

    def put_forks(self):
        if forks[self.fork[0]].locked() and forks[self.fork[1]].locked():
            forks[self.fork[0]].release()
            forks[self.fork[1]].release()
            print(self.name + ': \t put the forks down - ' + str(self.fork[0]) + ' and ' + str(self.fork[1]))

def main():
    philosophers = [
        Philosopher('Elon Musk', [0, 1]), 
        Philosopher('Shaggy', [1, 2]), 
        Philosopher('Spongebob', [2, 3]), 
        Philosopher('Bojack', [3, 4]),  
        Philosopher('Solaire', [4, 0])
    ]

    for ph in philosophers:
        ph.start()
    
    for ph in philosophers:
        ph.join()

if __name__ == "__main__":
    main()
