// Submitter: bsmorton(Morton, Bradley)
// Partner  : ealkabod(Al-Kabodi, Ebrahim)
// We certify that we worked cooperatively on this programming
//   assignment, according to the rules for pair programming


#ifndef HEAP_PRIORITY_QUEUE_HPP_
#define HEAP_PRIORITY_QUEUE_HPP_

#include <string>
#include <iostream>
#include <sstream>
#include <initializer_list>
#include "ics_exceptions.hpp"
#include <utility>              //For std::swap function
#include "array_stack.hpp"      //See operator <<


namespace ics {


#ifndef undefinedgtdefined
#define undefinedgtdefined
template<class T>
bool undefinedgt (const T& a, const T& b) {return false;}
#endif /* undefinedgtdefined */

//Instantiate the templated class supplying tgt(a,b): true, iff a has higher priority than b.
//If tgt is defaulted to undefinedgt in the template, then a constructor must supply cgt.
//If both tgt and cgt are supplied, then they must be the same (by ==) function.
//If neither is supplied, or both are supplied but different, TemplateFunctionError is raised.
//The (unique) non-undefinedgt value supplied by tgt/cgt is stored in the instance variable gt.
template<class T, bool (*tgt)(const T& a, const T& b) = undefinedgt<T>> class HeapPriorityQueue {
  public:
    typedef bool (*gtfunc) (const T& a, const T& b);
        
    //Destructor/Constructors
    ~HeapPriorityQueue();

    HeapPriorityQueue(bool (*cgt)(const T& a, const T& b) = undefinedgt<T>);
    explicit HeapPriorityQueue(int initial_length, bool (*cgt)(const T& a, const T& b) = undefinedgt<T>);
    HeapPriorityQueue(const HeapPriorityQueue<T,tgt>& to_copy, bool (*cgt)(const T& a, const T& b) = undefinedgt<T>);
    explicit HeapPriorityQueue(const std::initializer_list<T>& il, bool (*cgt)(const T& a, const T& b) = undefinedgt<T>);

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    explicit HeapPriorityQueue (const Iterable& i, bool (*cgt)(const T& a, const T& b) = undefinedgt<T>);


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
    HeapPriorityQueue<T,tgt>& operator = (const HeapPriorityQueue<T,tgt>& rhs);
    bool operator == (const HeapPriorityQueue<T,tgt>& rhs) const;
    bool operator != (const HeapPriorityQueue<T,tgt>& rhs) const;

    template<class T2, bool (*gt2)(const T2& a, const T2& b)>
    friend std::ostream& operator << (std::ostream& outs, const HeapPriorityQueue<T2,gt2>& pq);



    class Iterator {
      public:
        //Private constructor called in begin/end, which are friends of HeapPriorityQueue<T,tgt>
        ~Iterator();
        T           erase();
        std::string str  () const;
        HeapPriorityQueue<T,tgt>::Iterator& operator ++ ();
        HeapPriorityQueue<T,tgt>::Iterator  operator ++ (int);
        bool operator == (const HeapPriorityQueue<T,tgt>::Iterator& rhs) const;
        bool operator != (const HeapPriorityQueue<T,tgt>::Iterator& rhs) const;
        T& operator *  () const;
        T* operator -> () const;
        friend std::ostream& operator << (std::ostream& outs, const HeapPriorityQueue<T,tgt>::Iterator& i) {
          outs << i.str(); //Use the same meaning as the debugging .str() method
          return outs;
        }

        friend Iterator HeapPriorityQueue<T,tgt>::begin () const;
        friend Iterator HeapPriorityQueue<T,tgt>::end   () const;

      private:
        //If can_erase is false, the value has been removed from "it" (++ does nothing)
        HeapPriorityQueue<T,tgt>  it;                 //copy of HPQ (from begin), to use as iterator via dequeue
        HeapPriorityQueue<T,tgt>* ref_pq;
        int                       expected_mod_count;
        bool                      can_erase = true;

        //Called in friends begin/end
        //These constructors have different initializers (see it(...) in first one)
        Iterator(HeapPriorityQueue<T,tgt>* iterate_over, bool from_begin);    // Called by begin
        Iterator(HeapPriorityQueue<T,tgt>* iterate_over);                     // Called by end
    };


    Iterator begin () const;
    Iterator end   () const;


  private:
    bool (*gt) (const T& a, const T& b); //The gt used by enqueue (from template or constructor)
    T*  pq;                              //Array represents a heap, so it uses the heap ordering property
    int length    = 0;                   //Physical length of array: must be >= .size()
    int used      = 0;                   //Amount of array used:  invariant: 0 <= used <= length
    int mod_count = 0;                   //For sensing concurrent modification


    //Helper methods
    void ensure_length  (int new_length);
    int  left_child     (int i) const;         //Useful abstractions for heaps as arrays
    int  right_child    (int i) const;
    int  parent         (int i) const;
    bool is_root        (int i) const;
    bool in_heap        (int i) const;
    void percolate_up   (int i);
    void percolate_down (int i);
    void heapify        ();                   // Percolate down all value is array (from indexes used-1 to 0): O(N)
  };





////////////////////////////////////////////////////////////////////////////////
//
//HeapPriorityQueue class and related definitions

//Destructor/Constructors

template<class T, bool (*tgt)(const T& a, const T& b)>
HeapPriorityQueue<T,tgt>::~HeapPriorityQueue() {
    delete[] pq;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
HeapPriorityQueue<T,tgt>::HeapPriorityQueue(bool (*cgt)(const T& a, const T& b))
        : gt(tgt != (bool (*)(const T& a, const T& b))undefinedgt<T> ? tgt : cgt) {
    if (gt == (gtfunc)undefinedgt<T>)
        throw TemplateFunctionError("HeapPriorityQueue::default constructor: neither specified");
    if (tgt != (gtfunc)undefinedgt<T> && cgt != (gtfunc)undefinedgt<T> && tgt != cgt)
        throw TemplateFunctionError("HeapPriorityQueue::default constructor: both specified and different");

    pq = new T[length];
}


template<class T, bool (*tgt)(const T& a, const T& b)>
HeapPriorityQueue<T,tgt>::HeapPriorityQueue(int initial_length, bool (*cgt)(const T& a, const T& b))
: gt(tgt != (gtfunc)undefinedgt<T> ? tgt : cgt), length(initial_length)
{
    if (gt == (gtfunc)undefinedgt<T>)
        throw TemplateFunctionError("ArrayPriorityQueue::length constructor: neither specified");
    if (tgt != (gtfunc)undefinedgt<T> && cgt != (gtfunc)undefinedgt<T> && tgt != cgt)
        throw TemplateFunctionError("ArrayPriorityQueue::length constructor: both specified and different");

    if (length < 0)
        length = 0;
    pq = new T[length];
}


template<class T, bool (*tgt)(const T& a, const T& b)>
HeapPriorityQueue<T,tgt>::HeapPriorityQueue(const HeapPriorityQueue<T,tgt>& to_copy, bool (*cgt)(const T& a, const T& b))
    : gt(to_copy.gt), used(to_copy.used), length(to_copy.length)//gt(tgt != (gtfunc)undefinedgt<T> ? tgt : cgt)
{
        //std::cout << "copy" << std::endl;
        if (gt == (gtfunc)undefinedgt<T>)
            gt = to_copy.gt;//throw TemplateFunctionError("ArrayPriorityQueue::copy constructor: neither specified");
        if (tgt != (gtfunc)undefinedgt<T> && cgt != (gtfunc)undefinedgt<T> && tgt != cgt)
            throw TemplateFunctionError("ArrayPriorityQueue::copy constructor: both specified and different");

        pq = new T[length];

        if (gt == to_copy.gt) {
            used = to_copy.used;
            for (int i=0; i<to_copy.used; ++i)
                pq[i] = to_copy.pq[i];
        }else
            for (int i=0; i<to_copy.used; ++i)
                enqueue(to_copy.pq[i]);
}


template<class T, bool (*tgt)(const T& a, const T& b)>
HeapPriorityQueue<T,tgt>::HeapPriorityQueue(const std::initializer_list<T>& il, bool (*cgt)(const T& a, const T& b))
:gt(tgt != (gtfunc)undefinedgt<T> ? tgt: cgt), length(il.size())
{
    if (gt == (gtfunc)undefinedgt<T>)
        throw TemplateFunctionError("HeapPriorityQueue::length constructor: neither specified");
    if (tgt != (gtfunc)undefinedgt<T> && cgt != (gtfunc)undefinedgt<T> && tgt != cgt)
        throw TemplateFunctionError("HeapPriorityQueue::initializer_list constructor: both specified and different");

    int i = 0;
    pq = new T[length];
    for (auto& value: il) {
        pq[i] = value;
        ++i;
    }
    used = length;

    heapify();

}


template<class T, bool (*tgt)(const T& a, const T& b)>
template<class Iterable>
HeapPriorityQueue<T,tgt>::HeapPriorityQueue(const Iterable& i, bool (*cgt)(const T& a, const T& b))
:gt(tgt != (gtfunc)undefinedgt<T> ? tgt : cgt), length(i.size())
{
    if (gt == (gtfunc)undefinedgt<T>)
        throw TemplateFunctionError("HeapPriorityQueue::length constructor: neither specified");
    if (tgt != (gtfunc)undefinedgt<T> && cgt != (gtfunc)undefinedgt<T> && tgt != cgt)
        throw TemplateFunctionError("HeapPriorityQueue::initializer_list constructor: both specified and different");

    int j = 0;
    pq = new T[length];
    for (auto& value: i) {
        pq[j] = value;
        ++j;
    }
    used = length;

    heapify();
}


////////////////////////////////////////////////////////////////////////////////
//
//Queries

template<class T, bool (*tgt)(const T& a, const T& b)>
bool HeapPriorityQueue<T,tgt>::empty() const {
    return used==0;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
int HeapPriorityQueue<T,tgt>::size() const {
    return used;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
T& HeapPriorityQueue<T,tgt>::peek () const {
    //std::cout << "here" << std::endl;
    if (empty())
        throw EmptyError("HeapPriorityQueue::peek");
    //std::cout << pq[used-1] << std::endl;
    return pq[0];
}


template<class T, bool (*tgt)(const T& a, const T& b)>
std::string HeapPriorityQueue<T,tgt>::str() const {
    std::ostringstream answer;
    answer << "HeapPriorityQueue[";

    if (length != 0) {
        answer << "0:" << pq[0];
        for (int i=1; i<length; ++i)
            answer << "," << i << ":" << pq[i];
    }

    answer << "](length=" << length << ",used=" << used << ",mod_count=" << mod_count << ")";
    return answer.str();
}


////////////////////////////////////////////////////////////////////////////////
//
//Commands

template<class T, bool (*tgt)(const T& a, const T& b)>
int HeapPriorityQueue<T,tgt>::enqueue(const T& element) {
    ensure_length(used+1);
    pq[used]=element;
    percolate_up(used);
    used++;
    ++mod_count;
    return 1;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
T HeapPriorityQueue<T,tgt>::dequeue() {
    T value=pq[0];
    std::swap(pq[0],pq[used-1]);
    used--;
    percolate_down(0);
    ++mod_count;
    return value;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
void HeapPriorityQueue<T,tgt>::clear() {
    used = 0;
    ++mod_count;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
template <class Iterable>
int HeapPriorityQueue<T,tgt>::enqueue_all (const Iterable& i) {
    int count = 0;
    for (const T& v : i)
        count += enqueue(v);
    return count;
}


////////////////////////////////////////////////////////////////////////////////
//
//Operators

template<class T, bool (*tgt)(const T& a, const T& b)>
HeapPriorityQueue<T,tgt>& HeapPriorityQueue<T,tgt>::operator = (const HeapPriorityQueue<T,tgt>& rhs) {
    //std::cout << "assignment" << std::endl;
    if (this == &rhs)
        return *this;
    gt = rhs.gt;
    this->ensure_length(rhs.used);
    used = rhs.used;
    for (int i = 0; i < used; ++i)
        pq[i] = rhs.pq[i];
    ++mod_count;
    return *this;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
bool HeapPriorityQueue<T,tgt>::operator == (const HeapPriorityQueue<T,tgt>& rhs) const {
    //std::cout <<  "check1" << std::endl;
    if (this == &rhs)
        return true;
    if ((void*)gt != (void*)rhs.gt)
        return false;
    if (used != rhs.used)
        return false;
    ics::HeapPriorityQueue<T,tgt>::Iterator temp=rhs.begin();
    for(ics::HeapPriorityQueue<T,tgt>::Iterator p=begin(); p!=end(); ++p){
        if(*p!=*temp){
            return false;
        }
        ++temp;
    }

    return true;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
bool HeapPriorityQueue<T,tgt>::operator != (const HeapPriorityQueue<T,tgt>& rhs) const {
    return !(*this == rhs);
}


template<class T, bool (*tgt)(const T& a, const T& b)>
std::ostream& operator << (std::ostream& outs, const HeapPriorityQueue<T,tgt>& p){
    outs << "priority_queue[";
    if (!p.empty()){
        ArrayStack<T> newStack;
        for (auto i : p)
            newStack.push(i);

        outs << newStack.pop();
        while (!newStack.empty())
            outs << "," << newStack.pop();
    }

    outs << "]:highest";
    return outs;
}


////////////////////////////////////////////////////////////////////////////////
//
//Iterator constructors

template<class T, bool (*tgt)(const T& a, const T& b)>
auto HeapPriorityQueue<T,tgt>::begin () const -> HeapPriorityQueue<T,tgt>::Iterator {
    //std::cout << "begin" << std::endl;
    return Iterator(const_cast<HeapPriorityQueue<T,tgt>*>(this), true);
}


template<class T, bool (*tgt)(const T& a, const T& b)>
auto HeapPriorityQueue<T,tgt>::end () const -> HeapPriorityQueue<T,tgt>::Iterator {
    //std::cout << "end" << std::endl;
    return Iterator(const_cast<HeapPriorityQueue<T,tgt>*>(this));
}


////////////////////////////////////////////////////////////////////////////////
//
//Private helper methods

template<class T, bool (*tgt)(const T& a, const T& b)>
void HeapPriorityQueue<T,tgt>::ensure_length(int new_length) {
    if (length >= new_length)
        return;
    T* old_pq = pq;
    length = std::max(new_length,2*length);
    pq = new T[length];
    for (int i=0; i<used; ++i)
        pq[i] = old_pq[i];
    delete[] old_pq;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
int HeapPriorityQueue<T,tgt>::left_child(int i) const
{
    return(2*i+1);
}

template<class T, bool (*tgt)(const T& a, const T& b)>
int HeapPriorityQueue<T,tgt>::right_child(int i) const
{
    return(2*i+2);
}

template<class T, bool (*tgt)(const T& a, const T& b)>
int HeapPriorityQueue<T,tgt>::parent(int i) const
{
    return (i-1)/2;
}

template<class T, bool (*tgt)(const T& a, const T& b)>
bool HeapPriorityQueue<T,tgt>::is_root(int i) const
{
    return i==0;
}

template<class T, bool (*tgt)(const T& a, const T& b)>
bool HeapPriorityQueue<T,tgt>::in_heap(int i) const
{
    return i < used;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
void HeapPriorityQueue<T,tgt>::percolate_up(int i) {
    //std::cout<< "check1: " <<  i << std::endl;
    int N=i;
    while(N!=0){
        //std::cout<< "check2: " <<  pq[N] << " : " << pq[parent(N)] << " : " << gt(pq[N],pq[parent(N)]) <<std::endl;
        if(gt(pq[N],pq[parent(N)])){
            std::swap(pq[parent(N)],pq[N]);
        }
        N=parent(N);
    }
    return;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
void HeapPriorityQueue<T,tgt>::percolate_down(int i) {
    int deepest_child;
    int l = left_child(i);
    while (in_heap(l)){
        int r = right_child(i);

        if (!in_heap(r) || gt(pq[l], pq[r]))
            deepest_child = l;
        else
            deepest_child = r;

        if (gt(pq[i],pq[deepest_child]))
            break;
        else
            std::swap(pq[deepest_child], pq[i]);
        i = deepest_child;
        l = left_child(i);
    }

}


template<class T, bool (*tgt)(const T& a, const T& b)>
void HeapPriorityQueue<T,tgt>::heapify() {
for (int i = used-1; i >= 0; --i)
    percolate_down(i);

}


////////////////////////////////////////////////////////////////////////////////
//
//Iterator class definitions

template<class T, bool (*tgt)(const T& a, const T& b)>
HeapPriorityQueue<T,tgt>::Iterator::Iterator(HeapPriorityQueue<T,tgt>* iterate_over, bool tgt_nullptr)
       :ref_pq(iterate_over), it(*iterate_over)
{
    //it = *ref_pq;
    //std::cout << "Initial 1" << std::endl;
    expected_mod_count=ref_pq->mod_count;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
HeapPriorityQueue<T,tgt>::Iterator::Iterator(HeapPriorityQueue<T,tgt>* iterate_over)
{
}


template<class T, bool (*tgt)(const T& a, const T& b)>
HeapPriorityQueue<T,tgt>::Iterator::~Iterator()
{}


template<class T, bool (*tgt)(const T& a, const T& b)>
T HeapPriorityQueue<T,tgt>::Iterator::erase() {
    //std::cout << "erase" << std::endl;
    if (expected_mod_count != ref_pq->mod_count)
        throw ConcurrentModificationError("HeapPriorityQueue::Iterator::erase");
    if (!can_erase)
        throw CannotEraseError("ArrayPriorityQueue::Iterator::erase Iterator cursor already erased");

    can_erase = false;
    T to_return = it.dequeue();
    for(int i=0; i<ref_pq->size(); i++){
        if(ref_pq->pq[i]==to_return){
            std::swap(ref_pq->pq[i],ref_pq->pq[ref_pq->used-1]);
            ref_pq->used--;
            ref_pq->percolate_down(i);
        }
    }

    expected_mod_count = ref_pq->mod_count;
    return to_return;

}


template<class T, bool (*tgt)(const T& a, const T& b)>
std::string HeapPriorityQueue<T,tgt>::Iterator::str() const {
}


template<class T, bool (*tgt)(const T& a, const T& b)>
auto HeapPriorityQueue<T,tgt>::Iterator::operator ++ () -> HeapPriorityQueue<T,tgt>::Iterator& {
    //std::cout << "++()" << std::endl;
    if (expected_mod_count != ref_pq->mod_count)
        throw ConcurrentModificationError("HeapPriorityQueue::Iterator::operator ++");

    if(it.size()==0){
        return *this;
    }
    if(can_erase){
        it.dequeue();
    }
    else{
        can_erase=true;
    }
    return *this;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
auto HeapPriorityQueue<T,tgt>::Iterator::operator ++ (int) -> HeapPriorityQueue<T,tgt>::Iterator {
    //std::cout << "++(int)" << std::endl;
    if (expected_mod_count != ref_pq->mod_count)
        throw ConcurrentModificationError("HeapPriorityQueue::Iterator::operator ++(int)");
    if(it.size()==0){
        return *this;
    }
    Iterator to_return(*this);
    if(can_erase){
        it.dequeue();
    }
    else{
        can_erase=true;
    }
    return to_return;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
bool HeapPriorityQueue<T,tgt>::Iterator::operator == (const HeapPriorityQueue<T,tgt>::Iterator& rhs) const {
    //std::cout << "==" << std::endl;
    const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
//    if (expected_mod_count != ref_pq->mod_count)
//        throw ConcurrentModificationError("HeapPriorityQueue::Iterator::operator ==");
//    if (rhsASI == 0)
//        throw IteratorTypeError("HeapPriorityQueue::Iterator::operator ==");
//    if (ref_pq != rhsASI->ref_pq)
//        throw ComparingDifferentIteratorsError("HeapPriorityQueue::Iterator::operator ==");

    return it.size() == rhsASI->it.size();

}



template<class T, bool (*tgt)(const T& a, const T& b)>
bool HeapPriorityQueue<T,tgt>::Iterator::operator != (const HeapPriorityQueue<T,tgt>::Iterator& rhs) const {
    //std::cout << "!=" << std::endl;
    const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
//    if (expected_mod_count != ref_pq->mod_count)
//        throw ConcurrentModificationError("ArrayPriorityQueue::Iterator::operator !=");
//    if (rhsASI == 0)
//        throw IteratorTypeError("HeapPriorityQueue::Iterator::operator !=");
//    if (ref_pq != rhsASI->ref_pq)
//        throw ComparingDifferentIteratorsError("HeapPriorityQueue::Iterator::operator !=");
    return it.size() != rhsASI->it.size();;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
T& HeapPriorityQueue<T,tgt>::Iterator::operator *() const {
    //std::cout << "*()" << std::endl;
    if (expected_mod_count != ref_pq->mod_count)
        throw ConcurrentModificationError("ArrayPriorityQueue::Iterator::operator *");
    if (!can_erase) {
//        std::ostringstream where;
//        where << current << " when size = " << ref_pq->size();
        throw IteratorPositionIllegal("HeapPriorityQueue::Iterator::operator * Iterator illegal: ");
    }
    return it.peek();
}


template<class T, bool (*tgt)(const T& a, const T& b)>
T* HeapPriorityQueue<T,tgt>::Iterator::operator ->() const {
    //std::cout << "()" << std::endl;
    if (expected_mod_count != ref_pq->mod_count)
        throw ConcurrentModificationError("ArrayPriorityQueue::Iterator::operator *");
    if (!can_erase) {
//        std::ostringstream where;
//        where << current << " when size = " << ref_pq->size();
        throw IteratorPositionIllegal("HeapPriorityQueue::Iterator::operator -> Iterator illegal: ");
    }
    return &it.peek();
}

}

#endif /* HEAP_PRIORITY_QUEUE_HPP_ */
