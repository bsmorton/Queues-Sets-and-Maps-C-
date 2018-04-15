// Submitter: bsmorton(Morton, Bradley)
// Partner  : ealkabod(Al-Kabodi, Ebrahim)
// We certify that we worked cooperatively on this programming
//   assignment, according to the rules for pair programming


#ifndef HASH_SET_HPP_
#define HASH_SET_HPP_

#include <string>
#include <iostream>
#include <sstream>
#include <initializer_list>
#include "ics_exceptions.hpp"
#include "pair.hpp"


namespace ics {


#ifndef undefinedhashdefined
#define undefinedhashdefined
template<class T>
int undefinedhash (const T& a) {return 0;}
#endif /* undefinedhashdefined */

//Instantiate the templated class supplying thash(a): produces a hash value for a.
//If thash is defaulted to undefinedhash in the template, then a constructor must supply chash.
//If both thash and chash are supplied, then they must be the same (by ==) function.
//If neither is supplied, or both are supplied but different, TemplateFunctionError is raised.
//The (unique) non-undefinedhash value supplied by thash/chash is stored in the instance variable hash.
template<class T, int (*thash)(const T& a) = undefinedhash<T>> class HashSet {
  public:
    typedef int (*hashfunc) (const T& a);

    //Destructor/Constructors
    ~HashSet ();

    HashSet (double the_load_threshold = 1.0, int (*chash)(const T& a) = undefinedhash<T>);
    explicit HashSet (int initial_bins, double the_load_threshold = 1.0, int (*chash)(const T& k) = undefinedhash<T>);
    HashSet (const HashSet<T,thash>& to_copy, double the_load_threshold = 1.0, int (*chash)(const T& a) = undefinedhash<T>);
    explicit HashSet (const std::initializer_list<T>& il, double the_load_threshold = 1.0, int (*chash)(const T& a) = undefinedhash<T>);

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    explicit HashSet (const Iterable& i, double the_load_threshold = 1.0, int (*chash)(const T& a) = undefinedhash<T>);


    //Queries
    bool empty      () const;
    int  size       () const;
    bool contains   (const T& element) const;
    std::string str () const; //supplies useful debugging information; contrast to operator <<

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    bool contains_all (const Iterable& i) const;


    //Commands
    int  insert (const T& element);
    int  erase  (const T& element);
    void clear  ();

    //Iterable class must support "for" loop: .begin()/.end() and prefix ++ on returned result

    template <class Iterable>
    int insert_all(const Iterable& i);

    template <class Iterable>
    int erase_all(const Iterable& i);

    template<class Iterable>
    int retain_all(const Iterable& i);


    //Operators
    HashSet<T,thash>& operator = (const HashSet<T,thash>& rhs);
    bool operator == (const HashSet<T,thash>& rhs) const;
    bool operator != (const HashSet<T,thash>& rhs) const;
    bool operator <= (const HashSet<T,thash>& rhs) const;
    bool operator <  (const HashSet<T,thash>& rhs) const;
    bool operator >= (const HashSet<T,thash>& rhs) const;
    bool operator >  (const HashSet<T,thash>& rhs) const;

    template<class T2, int (*hash2)(const T2& a)>
    friend std::ostream& operator << (std::ostream& outs, const HashSet<T2,hash2>& s);



  private:
    class LN;

  public:
    class Iterator {
      public:
        typedef pair<int,LN*> Cursor;

        //Private constructor called in begin/end, which are friends of HashSet<T,thash>
        ~Iterator();
        T           erase();
        std::string str  () const;
        HashSet<T,thash>::Iterator& operator ++ ();
        HashSet<T,thash>::Iterator  operator ++ (int);
        bool operator == (const HashSet<T,thash>::Iterator& rhs) const;
        bool operator != (const HashSet<T,thash>::Iterator& rhs) const;
        T& operator *  () const;
        T* operator -> () const;
        friend std::ostream& operator << (std::ostream& outs, const HashSet<T,thash>::Iterator& i) {
          outs << i.str(); //Use the same meaning as the debugging .str() method
          return outs;
        }
        friend Iterator HashSet<T,thash>::begin () const;
        friend Iterator HashSet<T,thash>::end   () const;

      private:
        //If can_erase is false, current indexes the "next" value (must ++ to reach it)
        Cursor              current; //Bin Index and Cursor; stops if LN* == nullptr
        HashSet<T,thash>*   ref_set;
        int                 expected_mod_count;
        bool                can_erase = true;

        //Helper methods
        void advance_cursors();

        //Called in friends begin/end
        Iterator(HashSet<T,thash>* iterate_over, bool from_begin);
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
        LN* next   = nullptr;
    };

public:
  int (*hash)(const T& k);   //Hashing function used (from template or constructor)
private:
  LN** set      = nullptr;   //Pointer to array of pointers: each bin stores a list with a trailer node
  double load_threshold;     //used/bins <= load_threshold
  int bins      = 1;         //# bins in array (should start >= 1 so hash_compress doesn't % 0)
  int used      = 0;         //Cache for number of key->value pairs in the hash table
  int mod_count = 0;         //For sensing concurrent modification


  //Helper methods
  int   hash_compress        (const T& key)              const;  //hash function ranged to [0,bins-1]
  LN*   find_element         (const T& element)          const;  //Returns reference to element's node or nullptr
  LN*   copy_list            (LN*   l)                   const;  //Copy the elements in a bin (order irrelevant)
  LN**  copy_hash_table      (LN** ht, int bins)         const;  //Copy the bins/keys/values in ht tree (order in bins irrelevant)

  void  ensure_load_threshold(int new_used);                     //Reallocate if load_threshold > load_threshold
  void  delete_hash_table    (LN**& ht, int bins);               //Deallocate all LN in ht (and the ht itself; ht == nullptr)
};





//HashSet class and related definitions

////////////////////////////////////////////////////////////////////////////////
//
//Destructor/Constructors

template<class T, int (*thash)(const T& a)>
HashSet<T,thash>::~HashSet() {
    delete [] set;
}


template<class T, int (*thash)(const T& a)>
HashSet<T,thash>::HashSet(double the_load_threshold, int (*chash)(const T& element))
:hash(thash != (hashfunc)undefinedhash<T> ? thash : chash)
{
    if (hash == (hashfunc)undefinedhash<T>)
        throw TemplateFunctionError("HashSet::default constructor: neither specified");
    if (thash != (hashfunc)undefinedhash<T> && chash != (hashfunc)undefinedhash<T> && thash != chash)
        throw TemplateFunctionError("HashSet::default constructor: both specified and different");
    set = new LN*[bins];
    set[0]=new LN;
    load_threshold=1;
}


template<class T, int (*thash)(const T& a)>
HashSet<T,thash>::HashSet(int initial_bins, double the_load_threshold, int (*chash)(const T& element))
: hash(thash != (hashfunc)undefinedhash<T> ? thash : chash), bins(initial_bins)
{
    if (hash == (hashfunc)undefinedhash<T>)
        throw TemplateFunctionError("HashSet::length constructor: neither specified");
    if (thash != (hashfunc)undefinedhash<T> && chash != (hashfunc)undefinedhash<T> && thash != chash)
        throw TemplateFunctionError("HashSet::length constructor: both specified and different");

    bins = initial_bins;
    set = new LN*[bins];
    for(int i=0; i<bins; i++ ){
        set[i] = new LN;
    }
}


template<class T, int (*thash)(const T& a)>
HashSet<T,thash>::HashSet(const HashSet<T,thash>& to_copy, double the_load_threshold, int (*chash)(const T& element))
: hash(to_copy.hash), used(to_copy.used), bins(to_copy.bins)
{
    if (hash == (hashfunc)undefinedhash<T>)
        hash = to_copy.hash;
    if (thash != (hashfunc)undefinedhash<T> && chash != (hashfunc)undefinedhash<T> && thash != chash)
        throw TemplateFunctionError("HashSet::copy constructor: both specified and different");

    set = new LN*[bins];
    if (hash == to_copy.hash) {
        used = to_copy.used;
        for (int i = 0; i < to_copy.bins; ++i)
            set[i] = copy_list(to_copy.set[i]);
    }
    else {
        set = new LN*[bins];
        for (int i = 0; i < bins; ++i)
            set[i] = new LN();
        for (int i = 0; i < to_copy.bins; ++i)
            for (LN * p = to_copy.set[i]; p->next != nullptr; p = p->next)
                insert(p->value);

    }
}


template<class T, int (*thash)(const T& a)>
HashSet<T,thash>::HashSet(const std::initializer_list<T>& il, double the_load_threshold, int (*chash)(const T& element))
: hash(thash != (hashfunc)undefinedhash<T> ? thash : chash), bins(il.size())
{
    if (hash == (hashfunc)undefinedhash<T>)
        throw TemplateFunctionError("HashSet::initializer_list constructor: neither specified");
    if (thash != (hashfunc)undefinedhash<T> && chash != (hashfunc)undefinedhash<T> && thash != chash)
        throw TemplateFunctionError("HashSet::initializer_list constructor: both specified and different");

    set = new LN*[bins];
    for (int i = 0; i<bins; ++i)
        set[i] = new LN();

    for (const T& entry : il)
        insert(entry);
}


template<class T, int (*thash)(const T& a)>
template<class Iterable>
HashSet<T,thash>::HashSet(const Iterable& i, double the_load_threshold, int (*chash)(const T& a))
: hash(thash != (hashfunc)undefinedhash<T> ? thash : chash), bins(i.size())
{
    if (hash == (hashfunc)undefinedhash<T>)
        throw TemplateFunctionError("HashSet::Iterable constructor: neither specified");
    if (thash != (hashfunc)undefinedhash<T> && chash != (hashfunc)undefinedhash<T> && thash != chash)
        throw TemplateFunctionError("HashSet::Iterable constructor: both specified and different");

    set = new LN*[bins];
    for (int i = 0; i<bins; ++i)
        set[i] = new LN();

    for (const T& entry : i)
        insert(entry);
}


////////////////////////////////////////////////////////////////////////////////
//
//Queries

template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::empty() const {
    return used==0;
}


template<class T, int (*thash)(const T& a)>
int HashSet<T,thash>::size() const {
    return used;
}


template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::contains (const T& element) const {
    for(int i=0; i<bins; i++) {
        for (LN *p = set[i]; p->next != nullptr; p = p->next) {
            if (p->value == element) {
                return true;
            }
        }
    }
    return false;
}


template<class T, int (*thash)(const T& a)>
std::string HashSet<T,thash>::str() const {
    std::stringstream temp;
    for(int i=0; i<bins; i++){
        temp << " " << i << " : [";
        for(LN* p=set[i]; p->next!=nullptr; p=p->next){

            temp << '[' << p->value << ']';
        }
        temp << "]";
    }
    return temp.str();
}


template<class T, int (*thash)(const T& a)>
template <class Iterable>
bool HashSet<T,thash>::contains_all(const Iterable& i) const {
    for(T& p:i){
        if(!contains(p))
            return false;
    }
    return true;
}


////////////////////////////////////////////////////////////////////////////////
//
//Commands

template<class T, int (*thash)(const T& a)>
int HashSet<T,thash>::insert(const T& element) {
    //std::cout << "before : " << str() << std::endl;
    if(contains(element)){
        return 0;
    }
    used++;
    ensure_load_threshold(used);
    set[hash_compress(element)]=new LN(element,set[hash_compress(element)]);
    //std::cout << "after : " << str() << std::endl;
    mod_count++;
    return 1;
}


template<class T, int (*thash)(const T& a)>
int HashSet<T,thash>::erase(const T& element) {
    if(!contains(element))
        return 0;
    LN* prev=set[hash_compress(element)];
    if(prev->value==element){
        set[hash_compress(element)]=prev->next;
        delete prev;
    }
    else {
        for (LN *p = set[hash_compress(element)]->next; p->next != nullptr; p = p->next) {
            if(p->value==element){
                LN* to_delete=p;
                prev->next=p->next;
                delete to_delete;
                break;
            }
            prev=prev->next;
        }
    }
    used --;
    mod_count++;
    return 1;
}


template<class T, int (*thash)(const T& a)>
void HashSet<T,thash>::clear() {
    bins=1;
    set = new LN*[bins];
    set[0]=new LN;
    used=0;
    mod_count++;
    load_threshold=1;
}


template<class T, int (*thash)(const T& a)>
template<class Iterable>
int HashSet<T,thash>::insert_all(const Iterable& i) {
    int count=0;
    for(T& p:i){
        count+=insert(p);
    }
    return count;
}


template<class T, int (*thash)(const T& a)>
template<class Iterable>
int HashSet<T,thash>::erase_all(const Iterable& i) {
    int count=0;
    for(T& p:i){
        count+=erase(p);
    }
    return count;
}


template<class T, int (*thash)(const T& a)>
template<class Iterable>
int HashSet<T,thash>::retain_all(const Iterable& i) {
    HashSet<T,thash> temp1(i);
    HashSet<T,thash> temp2;
    for(T& p: *this){
        if(temp1.contains(p)){
            temp2.insert(p);
        }
    }
    *this=temp2;
    return 1;
    
}


////////////////////////////////////////////////////////////////////////////////
//
//Operators

template<class T, int (*thash)(const T& a)>
HashSet<T,thash>& HashSet<T,thash>::operator = (const HashSet<T,thash>& rhs) {
    if (this == &rhs)
        return *this;
    clear();
    hash = rhs.hash;
    for(int i=0; i<rhs.bins; i++){
        for(LN* p=rhs.set[i]; p->next!=nullptr; p=p->next){
            insert(p->value);
        }
    }
    return *this;
}


template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::operator == (const HashSet<T,thash>& rhs) const {
    if (this == &rhs)
        return true;
    if (used != rhs.size())
        return false;

    for(int i=0; i<rhs.bins; i++){
        for(LN* p=rhs.set[i]; p->next!=nullptr; p=p->next){
            if(!contains(p->value)){
                    return false;
            }
        }
    }
    return true;
}


template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::operator != (const HashSet<T,thash>& rhs) const {
    return !(*this == rhs);
}


template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::operator <= (const HashSet<T,thash>& rhs) const {
    return used < rhs.used || used == rhs.used;
}

template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::operator < (const HashSet<T,thash>& rhs) const {
    return used < rhs.used;
}


template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::operator >= (const HashSet<T,thash>& rhs) const {
    return used > rhs.used || used == rhs.used;
}


template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::operator > (const HashSet<T,thash>& rhs) const {
    return used > rhs.used;
}


template<class T, int (*thash)(const T& a)>
std::ostream& operator << (std::ostream& outs, const HashSet<T,thash>& s) {
    outs << "set[";
    int count =0;
    for (int i = 0; i < s.bins; ++i)
        for (typename HashSet<T,thash>::LN* p = s.set[i]; p->next != nullptr; p = p->next){
            if (count++ == 0)
                outs << "" ;
            else
                outs << ",";
            outs << p->value;
        }

    outs << "]";
    return outs;
}


////////////////////////////////////////////////////////////////////////////////
//
//Iterator constructors

template<class T, int (*thash)(const T& a)>
auto HashSet<T,thash>::begin () const -> HashSet<T,thash>::Iterator {
    //std::cout << "begin" << std::endl;
    return Iterator(const_cast<HashSet<T,thash>*>(this),true);
}


template<class T, int (*thash)(const T& a)>
auto HashSet<T,thash>::end () const -> HashSet<T,thash>::Iterator {
   // std::cout << "end" << std::endl;
    return Iterator(const_cast<HashSet<T,thash>*>(this),false);
}


////////////////////////////////////////////////////////////////////////////////
//
//Private helper methods

template<class T, int (*thash)(const T& a)>
int HashSet<T,thash>::hash_compress (const T& element) const {
    return std::abs(thash(element))%bins;
}


template<class T, int (*thash)(const T& a)>
typename HashSet<T,thash>::LN* HashSet<T,thash>::find_element (const T& element) const {
}

template<class T, int (*thash)(const T& a)>
typename HashSet<T,thash>::LN* HashSet<T,thash>::copy_list (LN* l) const {
    if (l == nullptr)
        return nullptr;
    else
        return new LN(l->value, copy_list(l->next));
}


template<class T, int (*thash)(const T& a)>
typename HashSet<T,thash>::LN** HashSet<T,thash>::copy_hash_table (LN** ht, int bins) const {
}


template<class T, int (*thash)(const T& a)>
void HashSet<T,thash>::ensure_load_threshold(int new_used) {
    if(new_used/bins > load_threshold){
        int old_bins = bins;
        bins = 2*bins;
        LN** new_set = new LN*[bins];
        for(int i=0; i<bins; i++ ){
            new_set[i] = new LN;
        }
        for(int i=0; i< old_bins; i++){
            for(LN* p=set[i]; p->next!= nullptr; p=p->next){
                new_set[hash_compress(p->value)]=new LN(p->value, new_set[hash_compress(p->value)]);
            }
        }
        set=new_set;
    }
}


template<class T, int (*thash)(const T& a)>
void HashSet<T,thash>::delete_hash_table (LN**& ht, int bins) {
}






////////////////////////////////////////////////////////////////////////////////
//
//Iterator class definitions

template<class T, int (*thash)(const T& a)>
void HashSet<T,thash>::Iterator::advance_cursors() {
   // std::cout << "advance cursors" << std::endl;
    if (current.second != nullptr && current.second->next != nullptr && current.second->next->next != nullptr) {
        current.second = current.second->next;
        return;
    }
        //it's trailer node so we move to other bins higher than first one
    else
        for (int i = current.first + 1; i < ref_set->bins; ++i)
            if (ref_set->set[i]->next != nullptr) {
                current.first = i;
                current.second = ref_set->set[i];
                return;
            }
    //ran out of bins to check so we set (bin = -1,LN* ptr =  nullptr)
    current.first = -1;
    current.second = nullptr;
}


template<class T, int (*thash)(const T& a)>
HashSet<T,thash>::Iterator::Iterator(HashSet<T,thash>* iterate_over, bool begin)
        : ref_set(iterate_over), expected_mod_count(ref_set->mod_count) {
    //std::cout << "iterate_over" << std::endl;
    current = Cursor(-1, nullptr);
    if (begin)
        advance_cursors();
}


template<class T, int (*thash)(const T& a)>
HashSet<T,thash>::Iterator::~Iterator()
{}


template<class T, int (*thash)(const T& a)>
T HashSet<T,thash>::Iterator::erase() {
   // std::cout << "erase" << std::endl;
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("HashMap::Iterator::erase");
    if (!can_erase)
        throw CannotEraseError("HashMap::Iterator::erase Iterator cursor already erased");
    if (current.second == nullptr)
        throw CannotEraseError("HashMap::Iterator::erase Iterator cursor beyond data structure");

    T to_return = current.second->value;

    LN * to_delete = current.second->next;
    *current.second = *current.second->next;
    delete to_delete;

    --ref_set->used;
    ++ref_set->mod_count;
    expected_mod_count = ref_set->mod_count;
    can_erase = false;

    return to_return;
}


template<class T, int (*thash)(const T& a)>
std::string HashSet<T,thash>::Iterator::str() const {
    //std::cout << "str" << std::endl;
}


template<class T, int (*thash)(const T& a)>
auto  HashSet<T,thash>::Iterator::operator ++ () -> HashSet<T,thash>::Iterator& {
    //std::cout << "++()" << std::endl;
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("HashMap::Iterator::operator ++");

    if (current.second == nullptr)
        return *this;

    if (can_erase || current.second->next == nullptr)
        advance_cursors();

    can_erase = true;

    return *this;
}


template<class T, int (*thash)(const T& a)>
auto  HashSet<T,thash>::Iterator::operator ++ (int) -> HashSet<T,thash>::Iterator {
    //std::cout << "++(int)" << std::endl;
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("HashMap::Iterator::operator ++(int)");

    Iterator to_return(*this);

    if (current.second == nullptr)
        return to_return;

    if (can_erase || current.second->next == nullptr)
        advance_cursors();

    can_erase = true;

    return to_return;
}


template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::Iterator::operator == (const HashSet<T,thash>::Iterator& rhs) const {
   // std::cout << "==" << std::endl;
    const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
    if (rhsASI == 0)
        throw IteratorTypeError("HashSet::Iterator::operator ==");
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("HashSet::Iterator::operator ==");
    if (ref_set != rhsASI->ref_set)
        throw ComparingDifferentIteratorsError("HashSet::Iterator::operator ==");

    return current.second == rhsASI->current.second;
}


template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::Iterator::operator != (const HashSet<T,thash>::Iterator& rhs) const {
   // std::cout << "!=" << std::endl;
    const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
    if (rhsASI == 0)
        throw IteratorTypeError("HashSet::Iterator::operator !=");
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("HashSet::Iterator::operator !=");
    if (ref_set != rhsASI->ref_set)
        throw ComparingDifferentIteratorsError("HashSet::Iterator::operator !=");

    return current.second != rhsASI->current.second;
}

template<class T, int (*thash)(const T& a)>
T& HashSet<T,thash>::Iterator::operator *() const {
   // std::cout << "*()" << std::endl;
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("HashMap::Iterator::operator *");
    if (!can_erase || current.second == nullptr)
        throw IteratorPositionIllegal("HashMap::Iterator::operator * Iterator illegal:");

    return current.second->value;
}

template<class T, int (*thash)(const T& a)>
T* HashSet<T,thash>::Iterator::operator ->() const {
    std::cout << "()" << std::endl;
}

}

#endif /* HASH_SET_HPP_ */
