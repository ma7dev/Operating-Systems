import time
from random import randint
import threading
from threading import Thread, Lock, Condition

forks = [Lock() for _ in range(5)]
occupied = [[" "," "], [" "," "], [" "," "], [" "," "], [" "," "]]

# def display_forks():
#     for i,fork in enumerate(forks):
#         status = "available"
#         if(fork.locked()):
#             status = "unavailable"
#         print("fork:" + i + status, philosophers[i] + ": " + state, end=' ')
#     print(" ")

def display_forks():
    buffer=""
    buffer += '------------------------\n'
    for i,fork in enumerate(forks):
        if(fork.locked() == False):
            buffer += "| \t del \t |\n"
        else:
            buffer += "| \t Occupied by:" + occupied[i][0] + "(" + occupied[i][1] + ") \t |\n"
    print(buffer)
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
            self.eat()
            self.put_forks()

    def think(self):
        self.state = "thinking"
        delay = randint(1, 20)
        buffer=""
        buffer += '-------------------------\n'
        buffer += self.name + ': \t Thinking time = ' + str(delay) + "\n"
        print(buffer)
        time.sleep(delay)
        
    def get_forks(self):
        self.state = "waiting"

        if (self.fork[0] < self.fork[1]):
            forks[self.fork[0]].acquire(True)
            occupied[self.fork[0]][0] = self.name
            occupied[self.fork[1]][0] = self.name
            occupied[self.fork[0]][1] = self.state
            occupied[self.fork[1]][1] = self.state
            forks[self.fork[1]].acquire(True)
        else:
            forks[self.fork[1]].acquire(True)
            occupied[self.fork[0]][0] = self.name
            occupied[self.fork[1]][0] = self.name
            occupied[self.fork[0]][1] = self.state
            occupied[self.fork[1]][1] = self.state
            forks[self.fork[0]].acquire(True)


    def eat(self):
        #pprint('Got forks ---')
        self.state = "eating"
        occupied[self.fork[0]][0] = self.name
        occupied[self.fork[1]][0] = self.name
        occupied[self.fork[0]][1] = self.state
        occupied[self.fork[1]][1] = self.state
        delay = randint(2, 9)
        buffer=""
        buffer += '-------------------------\n'
        buffer += self.name + ': \t Eating time = ' + str(delay) + '\n'
        print(buffer)
        display_forks()
        time.sleep(delay)
        #pprint(threading.current_thread())

    def put_forks(self):
        if forks[self.fork[0]].locked() and forks[self.fork[1]].locked():
            #pprint('Putting forks ---')
            forks[self.fork[0]].release()
            forks[self.fork[1]].release()

def main():
    philosophers = [
        Philosopher('Musk', [0, 1]), 
        Philosopher('Shaggy', [1, 2]), 
        Philosopher('Spongebob', [2, 3]), 
        Philosopher('Bojack', [3, 4]),  
        Philosopher('Solaire', [4, 0])
    ]
    display_forks()

    for ph in philosophers:
#        pprint(ph.name)
        ph.start()
    
    for ph in philosophers:
        ph.join()

if __name__ == "__main__":
    main()
