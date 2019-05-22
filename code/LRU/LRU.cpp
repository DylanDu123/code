#include<stdio.h>
#include <iostream>
#include <string>
#include <map>

using namespace std;

struct Node
{
    string key;
    int value;
    Node* next;
    Node* pre;
};


class LRUCache
{
private:
    map <string,int> m;
    Node *head;
    int len;
    int capacity;
    void removeLast();
public:
    LRUCache(int capacity);
    void moveFront(Node* h);
    void log();
    void set(string key, int value);
    int get(string key);
};

LRUCache::LRUCache(int capacity)
{
    this->len = 0;
    this->capacity = capacity;
    this->head = NULL;
}

void LRUCache::log()
{
    Node *h = this->head;
    while (h)
    {
         cout<<h->key<<' '<<h->value<<endl; 
         h = h->next;
        /* code */
    }
}

void LRUCache::removeLast()
{
    Node *h = this->head;
    while (h->next != NULL)
    {
        h = h->next;
    }
    h->pre->next = NULL;
    m.erase(h->key);
    delete h;
}

void LRUCache::moveFront(Node* h)
{
    if (h->pre != NULL)
    {
        h->pre->next = h->next;
    }
    if (h->next != NULL)
    {
        h->next->pre = h->pre;
    }
    this->head->pre = h;
    h->next = this->head;
    this->head = h;
}

int LRUCache::get(string key)
{
    if (this->m.find(key)!=this->m.end())
    {
        Node *h = this->head;
        while (h&&h->key!=key)
        {
            h = h->next;
        }
        this->moveFront(h);
        return this->m[key];
    }else
    {
        return -1;
    }    
}

void LRUCache::set(string key, int value)
{
    if (this->len >= this->capacity)
    {
        this->removeLast();
    }

    int v = this->get(key);
    if (v == -1)
    {
        Node *node = new Node();
        node->key = key;
        node->value = value;
        this->len++;
        this->m[key]=value;
        if (this->head == NULL)
        {
            this->head = node;
        }else
        {
            node->next = this->head;
            node->pre = NULL;
            this->head->pre = node;
            this->head = node;
        }
    }else{
        Node *h = this->head;
        while (h&&h->key!=key)
        {
            h = h->next;
        }
        this->moveFront(h);
    }
}

int main(int argc, char const *argv[])
{
    return 0;
}
