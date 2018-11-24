#include <iostream>

using namespace std;

struct node {
    int data;
    struct node *next;
};

class linkedList {
    private:
        node *head, *tail;
    
    public:
        linkedList() {
            head = NULL;
            tail = NULL;
        }
}

int main() {
    linkedList a;
    return 0;
}