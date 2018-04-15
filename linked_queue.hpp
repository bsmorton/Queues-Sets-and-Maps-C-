// Submitter: bsmorton(Morton, Bradley)
#ifndef LINKED_QUEUE_HPP_
#define LINKED_QUEUE_HPP_

#include <string>
#include <iostream>
#include <sstream>
#include <initializer_list>
#include "ics_exceptions.hpp"


namespace ics {


template<class T> class LinkedQueue {
  public:
    //Destructor/Constructors
    ~LinkedQueue();

    LinkedQueue          ();
    LinkedQueue          (const LinkedQueue<T>& to_copy);
    explicit LinkedQueue (const std::initializer_list<T>& il);

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    explicit LinkedQueue (const Iterable& i);


    //Queries
    bool empty      () const;
    int  size       () const;
    T&   peek       () const;
    std::string str () const; //supplies useful debugging information; contrast to operator <<


    //Commands
    int  enqueue (const T& element);
    T    dequeue ();
    void clear   ();

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    int enqueue_all (const Iterable& i);


    //Operators
    LinkedQueue<T>& operator = (const LinkedQueue<T>& rhs);
    bool operator == (const LinkedQueue<T>& rhs) const;
    bool operator != (const LinkedQueue<T>& rhs) const;

    template<class T2>
    friend std::ostream& operator << (std::ostream& outs, const LinkedQueue<T2>& q);



  private:
    class LN;

  public:
    class Iterator {
      public:
        //Private constructor called in begin/end, which are friends of LinkedQueue<T>
        ~Iterator();
        T           erase();
        std::string str  () const;
        LinkedQueue<T>::Iterator& operator ++ ();
        LinkedQueue<T>::Iterator  operator ++ (int);
        bool operator == (const LinkedQueue<T>::Iterator& rhs) const;
        bool operator != (const LinkedQueue<T>::Iterator& rhs) const;
        T& operator *  () const;
        T* operator -> () const;
        friend std::ostream& operator << (std::ostream& outs, const LinkedQueue<T>::Iterator& i) {
          outs << i.str(); //Use the same meaning as the debugging .str() method
          return outs;
        }
        friend Iterator LinkedQueue<T>::begin () const;
        friend Iterator LinkedQueue<T>::end   () const;

      private:
        //If can_erase is false, current indexes the "next" value (must ++ to reach it)
        LN*             prev = nullptr;  //if nullptr, current at front of list
        LN*             current;         //current == prev->next (if prev != nullptr)
        LinkedQueue<T>* ref_queue;
        int             expected_mod_count;
        bool            can_erase = true;

        //Called in friends begin/end
        Iterator(LinkedQueue<T>* iterate_over, LN* initial);
    };


    Iterator begin () const;
    Iterator end   () const;


  private:
    class LN {
      public:
        LN ()                      {}
        LN (const LN& ln)          : value(ln.value), next(ln.next){}
        LN (T v,  LN* n = nullptr) : value(v), next(n){}

        T   value;
        LN* next = nullptr;
    };


    LN* front     =  nullptr;
    LN* rear      =  nullptr;
    int used      =  0;            //Cache the number of values in linked list
    int mod_count =  0;            //For sensing all concurrent modifications

    //Helper methods
    void delete_list(LN*& front);  //Deallocate all LNs, and set front's argument to nullptr;
};





////////////////////////////////////////////////////////////////////////////////
//
//LinkedQueue class and related definitions

//Destructor/Constructors

template<class T>
LinkedQueue<T>::~LinkedQueue() {
    LN* current = front;
    while( current != nullptr ) {
        LN* next = current->next;
        delete current;
        current = next;
    }
    front = nullptr;

}


template<class T>
LinkedQueue<T>::LinkedQueue() {
}


template<class T>
LinkedQueue<T>::LinkedQueue(const LinkedQueue<T>& to_copy) {
    enqueue_all(to_copy);
}


template<class T>
LinkedQueue<T>::LinkedQueue(const std::initializer_list<T>& il) {
    enqueue_all(il);
}


template<class T>
template<class Iterable>
LinkedQueue<T>::LinkedQueue(const Iterable& i) {
    enqueue_all(i);
}


////////////////////////////////////////////////////////////////////////////////
//
//Queries

template<class T>
bool LinkedQueue<T>::empty() const {
    return front == nullptr;
}


template<class T>
int LinkedQueue<T>::size() const {
    int size=0;
    if(front!= nullptr){
        for (LN *p = front; p != nullptr; p = p->next){
            size+=1;
        }
    }
    return size;
}


template<class T>
T& LinkedQueue<T>::peek () const {
    if(front== nullptr){
        throw ics::EmptyError("Queue is Empty");
    }
    return front->value;
}


template<class T>
std::string LinkedQueue<T>::str() const {
}


////////////////////////////////////////////////////////////////////////////////
//
//Commands

template<class T>
int LinkedQueue<T>::enqueue(const T& element) {

    if(front == nullptr){
        front = rear = new LN(element);
    }
    else
    {
        rear->next = new LN(element);
        rear = rear-> next;
    }
    ++mod_count;
    return 1;
}


template<class T>
T LinkedQueue<T>::dequeue() {
    if (empty()==1){
        throw EmptyError("ArrayQueue::dequeue");
    }
    T val=front->value;
    front=front->next;
    ++mod_count;
    return val;
}


template<class T>
void LinkedQueue<T>::clear() {
    front= nullptr;
    rear=nullptr;
    ++mod_count;
}


template<class T>
template<class Iterable>
int LinkedQueue<T>::enqueue_all(const Iterable& i) {
    int count = 0;
    for (const T& v : i){
        count += enqueue(v);
    }
    return count;
}


////////////////////////////////////////////////////////////////////////////////
//
//Operators

template<class T>
LinkedQueue<T>& LinkedQueue<T>::operator = (const LinkedQueue<T>& rhs) {
    if (this == &rhs){
        return *this;
    }
    clear();
    for (LN *p = rhs.front; p != nullptr; p = p->next){
        enqueue(p->value);
    }
    ++mod_count;
    return *this;
}


template<class T>
bool LinkedQueue<T>::operator == (const LinkedQueue<T>& rhs) const {
    if(this->size() != rhs.size()){
        return false;
    }
    LN* temp=rhs.front;
    for (LN *p = this -> front; p != nullptr; p = p->next){
        if(temp->value==p->value){
            temp=temp->next;
        }
        else{
            return false;
        }
    }
    return true;
}


template<class T>
bool LinkedQueue<T>::operator != (const LinkedQueue<T>& rhs) const {
    return !(*this == rhs);
}


template<class T>
std::ostream& operator << (std::ostream& outs, const LinkedQueue<T>& q) {
    outs << "queue[";
    if(q.size()!=0){
        for (typename LinkedQueue<T>::LN *p = q.front; p != nullptr; p = p->next){
            if(p->next!= nullptr){
                outs << p->value+",";
            }
            else{
                outs << p->value;
            }
        }
    }
    outs << "]:rear";
    return outs;
}


////////////////////////////////////////////////////////////////////////////////
//
//Iterator constructors

template<class T>
auto LinkedQueue<T>::begin () const -> LinkedQueue<T>::Iterator {
    //std::cout << "begin" << std::endl;
    return Iterator(const_cast<LinkedQueue<T>*>(this),front);
}

template<class T>
auto LinkedQueue<T>::end () const -> LinkedQueue<T>::Iterator {
    //std::cout << "end" << std::endl;
    return Iterator(const_cast<LinkedQueue<T>*>(this), nullptr);

}


////////////////////////////////////////////////////////////////////////////////
//
//Private helper methods

template<class T>
void LinkedQueue<T>::delete_list(LN*& front) {
    std::cout << "delete_list" << std::endl;
}





////////////////////////////////////////////////////////////////////////////////
//
//Iterator class definitions

template<class T>
LinkedQueue<T>::Iterator::Iterator(LinkedQueue<T>* iterate_over, LN* initial):
current(initial), ref_queue(iterate_over), expected_mod_count(ref_queue->mod_count)
{
    //std::cout << "initial" << std::endl;
}


template<class T>
LinkedQueue<T>::Iterator::~Iterator()
{}


template<class T>
T LinkedQueue<T>::Iterator::erase() {
    //std::cout << "erase" << std::endl;
    if (expected_mod_count != ref_queue->mod_count)
        throw ConcurrentModificationError("ArrayQueue::Iterator::erase");
    if (!can_erase || current==nullptr)
        throw CannotEraseError("LinkedQueue::Iterator::erase Iterator cursor already erased");
    can_erase = false;
    T to_return = current->value;
    typename LinkedQueue<T>::LN* to_delete=current;
    if(current == ref_queue->front){
        current = current->next;
        ref_queue->front=current;
        delete to_delete;
    }
    else if(prev!=nullptr){
        current=current->next;
        prev->next=current;
        delete to_delete;
    }

    if(current== nullptr){
        ref_queue->rear=prev;
    }
    expected_mod_count = ref_queue->mod_count;
    can_erase = false;
    return to_return;
}


template<class T>
std::string LinkedQueue<T>::Iterator::str() const {
}


template<class T>
auto LinkedQueue<T>::Iterator::operator ++ () -> LinkedQueue<T>::Iterator& {
    //std::cout << "++i" << std::endl;
    if (expected_mod_count != ref_queue->mod_count)
        throw ConcurrentModificationError("ArrayQueue::Iterator::operator ++");
    if(current==nullptr){
        return *this;
    }
    if (can_erase) {
        prev=current;
        current=current->next;
    }

    else{
        can_erase = true;
    }
    return *this;
}


template<class T>
auto LinkedQueue<T>::Iterator::operator ++ (int) -> LinkedQueue<T>::Iterator {
    //std::cout << "i++" << std::endl;
    if (expected_mod_count != ref_queue->mod_count)
        throw ConcurrentModificationError("ArrayQueue::Iterator::operator ++(int)");
    if (current == ref_queue->rear->next) {

        return *this;
    }
    Iterator to_return(*this);
    if (can_erase) {
        prev=current;
        current=current->next;
    }
    else{
        can_erase = true;
    }

    return to_return;
}


template<class T>
bool LinkedQueue<T>::Iterator::operator == (const LinkedQueue<T>::Iterator& rhs) const {
    //std::cout << "==" << std::endl;
    const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
    if (rhsASI == 0)
        throw IteratorTypeError("ArrayQueue::Iterator::operator ==");
    if (expected_mod_count != ref_queue->mod_count)
        throw ConcurrentModificationError("ArrayQueue::Iterator::operator !=");
    if (ref_queue != rhsASI->ref_queue)
        throw ComparingDifferentIteratorsError("ArrayQueue::Iterator::operator !=");
    return current == rhsASI-> current;
}


template<class T>
bool LinkedQueue<T>::Iterator::operator != (const LinkedQueue<T>::Iterator& rhs) const {
    //std::cout << "!=" << std::endl;
    const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
    if (rhsASI == 0)
        throw IteratorTypeError("ArrayQueue::Iterator::operator ==");
    if (expected_mod_count != ref_queue->mod_count)
        throw ConcurrentModificationError("ArrayQueue::Iterator::operator !=");
    if (ref_queue != rhsASI->ref_queue)
        throw ComparingDifferentIteratorsError("ArrayQueue::Iterator::operator !=");
    return current  != rhsASI-> current;
}


template<class T>
T& LinkedQueue<T>::Iterator::operator *() const {
    //std::cout << "*()" << std::endl;
    if (expected_mod_count != ref_queue->mod_count)
        throw ConcurrentModificationError("ArrayQueue::Iterator::operator *");
    if (!can_erase || current== nullptr) {
        throw IteratorPositionIllegal("LinkedQueue::Iterator::operator * Iterator illegal: ");
    }
    return current->value;
}


template<class T>
T* LinkedQueue<T>::Iterator::operator ->() const {
    //std::cout << "()" << std::endl;
    if (expected_mod_count != ref_queue->mod_count)
        throw ConcurrentModificationError("ArrayQueue::Iterator::operator ->");
    return &current->value;
}
}

#endif /* LINKED_QUEUE_HPP_ */
