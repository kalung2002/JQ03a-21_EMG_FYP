#include <stdlib.h>
#include <iostream>
#include <iomanip>
using namespace std;
class cir_buf
{

private:
    // int capacity = 0;
    // int data_count = 0;
    // uint16_t *array = nullptr;
    // uint16_t *head = nullptr;
    // uint16_t *now = nullptr;
    // uint16_t *tail = nullptr; //tail = next one(empty)

public:
    int capacity = 0;
    int data_count = 0;
    uint16_t *array = nullptr;
    uint16_t *head = nullptr;
    uint16_t *now = nullptr;
    uint16_t *tail = nullptr; //tail = next one(empty)
    cir_buf(int capacity = 200)
    {
        if (capacity > 0)
        {
            this->array = new uint16_t[capacity];
            head = this->array;
            now = this->array;
            tail = this->array;
        }
        this->capacity = capacity;
    }
    void add(uint16_t data)
    {
        if (this->isEmpty())
        {
            data_count++;
            *this->now = data;
            this->tail = nextptr(this->tail);
        }
        else if (this->isFull())
        {
            // cout << "****FULLED" << flush;
            *this->tail = data;
            this->now = this->tail;
            this->head = nextptr(this->head);
            this->tail = nextptr(this->tail);
        }
        else
        {
            data_count++;
            *this->tail = data;
            this->now = this->tail;
            this->tail = nextptr(this->tail);
        }
    }
    uint16_t remove()
    {
        uint16_t out = 0;
        if (isEmpty())
        {
            return out;
        }

        if (data_count == 1)
        {
            data_count--;
            this->tail = prev(this->tail);
            out = *this->now;
            *this->now = 64; ////////////////////
                             // cout << "empty" << endl;
        }
        else
        {
            data_count--;
            out = *this->now;
            *this->now = 64; ////////////////////
            this->now = prev(this->now);
            this->tail = prev(this->tail);
        }
        return out;
    }
    uint16_t pop_front()
    {
        if (!isEmpty())
        {
            data_count--;
            uint16_t out = *this->head;
            *this->head = 64;
            this->head = nextptr(this->head);
            if (data_count == 0)
            {
                this->now = nextptr(this->now);
            }
            return out;
        }
        return 0;
    }
    uint16_t *nextptr(uint16_t *pos)
    {
        if (pos + 1 < this->array + this->capacity)
        {
            return pos + 1;
        }
        else
        {
            return this->array;
        }
    }
    uint16_t *prev(uint16_t *pos)
    {
        if (pos - 1 >= this->array)
        {
            return pos - 1;
        }
        else
        {
            return this->array + this->capacity - 1;
        }
    }
    bool isFull() { return (this->head == this->tail) && (this->head != this->now); }
    bool isEmpty() { return (this->head == this->tail) && (data_count == 0); }
    unsigned int getSize()
    {
        if (isFull())
        {
            return this->capacity;
        }
        if (this->now - this->head >= 0)
        {
            return this->now - this->head + 1;
        }
        else
        {
            return ((this->now - this->array) + 1 + (this->array + capacity - this->head));
        }
    }
    uint16_t getLatest()
    {
        if (!isEmpty())
        {
            return *this->now;
        }
        return 0;
    }
    uint16_t getItem(int index)
    {
        uint16_t *out = nullptr;
        if (index < capacity && index >= 0 && index < getSize())
        {
            if (this->head + index < this->array + capacity)
            {
                out = (this->head + index);
            }
            else
            {
                out = (this->array + index - (this->array + capacity - this->head));
            }
        }
        if (out)
        {
            return *out;
        }
        return 0;
    }
    void debug(int i)
    {
        cout << setw(4) << i << "***";
        cout << "|" << head << "|" << now << "|" << tail << "|";
        cout << "isFULL:" << isFull() << " is Empty:" << isEmpty();
        cout << "size:" << getSize();
        cout << "data_count" << data_count;
        cout << endl;
    }
    void printArray(bool abs = true)
    {
        for (int i = 0; i < capacity; i++)
        {
            if (abs)
            {
                cout << (char)this->array[i] << " | ";
            }
            else
            {
                cout << (char)getItem(i) << " | ";
            }
        }
        cout << endl;
    }
};

int main()
{
    cir_buf asd = cir_buf(10);
    cout.fill('0');
    for (size_t i = 0; i < 40; i++)
    {

        asd.printArray(true);
        // asd.debug(i);
        asd.add((65 + i % 26));

        cout << "Insert: " << (char)(65 + i % 26) << endl;
        asd.printArray(true);
        asd.debug(i);
        if (i % 4 == 0)
        {
            cout << "Remove: " << (char)asd.pop_front() << endl;
            asd.printArray(true);
            cout << "Remove: " << (char)asd.remove() << endl;
            asd.printArray(true);
            // cout << (char)asd.remove() << endl;
        }
        // asd.debug(i);
        // cout << endl;

        // cout << (char)asd.getLatest();
    }
    // for (size_t i = 0; i < 10; i++)
    // {
    //     asd.add(i);
    //     // cout << asd.remove() << endl;
    // }
}