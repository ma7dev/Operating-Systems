import time
from random import randint
import threading
from threading import Thread, Lock, Condition

forks = [Lock() for _ in range(5)]

class Philosopher(Thread):
    def __init__(self, name, fork. neigbors):
        Thread.__init__(self)
        self.name = name
        self.fork = fork
        self.neigbors = neigbors
    
    def run(self):
        self.eat()

    def can_eat(self):
        #print(forks[self.fork[0]].locked())
        if forks[self.fork[0]].locked() or forks[self.fork[1]].locked():
            print('can eat = false')
            return False
        else:
            print('can eat = true')
            return True

    def think(self):
        delay = randint(1, 20)
        time.sleep(delay)
        
    def get_forks(self):
#        c = Condition()
#        if self.can_eat():
#     c.wait_for(self.can_eat)
        print('Getting forks ---')
        if not (forks[self.fork[0]].locked() or forks[self.fork[1]].locked()):
            forks[self.fork[0]].acquire(True)
            forks[self.fork[1]].acquire(True)       

    def eat(self):
        self.get_forks()
        print('Got forks ---')
        delay = randint(2, 9)
        print('Delay time = ' + str(delay))
        time.sleep(delay)
        print(threading.current_thread())
        self.put_forks()

    def put_forks(self):
        if forks[self.fork[0]].locked() and forks[self.fork[1]].locked():
            print('Putting forks ---')
            forks[self.fork[0]].release()
            forks[self.fork[1]].release()

def main():
    philosophers = [
        Philosopher('Musk', [0, 1], [4, 1]), 
        Philosopher('Shaggy', [1, 2], [0, 2]), 
        Philosopher('Spongebob', [2, 3], [1, 3]), 
        Philosopher('Bojack', [3, 4], [2, 4]),  
        Philosopher('Solaire', [4, 0], [3, 0])
    ]
    
    for ph in philosophers:
#        print(ph.name)
        ph.start()
    
    for ph in philosophers:
        ph.join()

if __name__ == "__main__":
    main()
