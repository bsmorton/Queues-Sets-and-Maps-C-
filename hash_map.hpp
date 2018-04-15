// Submitter: bsmorton(Morton, Bradley)
// Partner  : ealkabod(Al-Kabodi, Ebrahim)
// We certify that we worked cooperatively on this programming
//   assignment, according to the rules for pair programming


#ifndef HASH_MAP_HPP_
#define HASH_MAP_HPP_

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
template<class KEY,class T, int (*thash)(const KEY& a) = undefinedhash<KEY>> class HashMap {
  public:
    typedef ics::pair<KEY,T>   Entry;
    typedef int (*hashfunc) (const KEY& a);

    //Destructor/Constructors
    ~HashMap ();

    HashMap          (double the_load_threshold = 1.0, int (*chash)(const KEY& a) = undefinedhash<KEY>);
    explicit HashMap (int initial_bins, double the_load_threshold = 1.0, int (*chash)(const KEY& k) = undefinedhash<KEY>);
    HashMap          (const HashMap<KEY,T,thash>& to_copy, double the_load_threshold = 1.0, int (*chash)(const KEY& a) = undefinedhash<KEY>);
    explicit HashMap (const std::initializer_list<Entry>& il, double the_load_threshold = 1.0, int (*chash)(const KEY& a) = undefinedhash<KEY>);

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    explicit HashMap (const Iterable& i, double the_load_threshold = 1.0, int (*chash)(const KEY& a) = undefinedhash<KEY>);


    //Queries
    bool empty      () const;
    int  size       () const;
    bool has_key    (const KEY& key) const;
    bool has_value  (const T& value) const;
    std::string str () const; //supplies useful debugging information; contrast to operator <<


    //Commands
    T    put   (const KEY& key, const T& value);
    T    erase (const KEY& key);
    void clear ();

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    int put_all(const Iterable& i);


    //Operators

    T&       operator [] (const KEY&);
    const T& operator [] (const KEY&) const;
    HashMap<KEY,T,thash>& operator = (const HashMap<KEY,T,thash>& rhs);
    bool operator == (const HashMap<KEY,T,thash>& rhs) const;
    bool operator != (const HashMap<KEY,T,thash>& rhs) const;

    template<class KEY2,class T2, int (*hash2)(const KEY2& a)>
    friend std::ostream& operator << (std::ostream& outs, const HashMap<KEY2,T2,hash2>& m);



  private:
    class LN;

  public:
    class Iterator {
      public:
         typedef pair<int,LN*> Cursor;

        //Private constructor called in begin/end, which are friends of HashMap<T>
        ~Iterator();
        Entry       erase();
        std::string str  () const;
        HashMap<KEY,T,thash>::Iterator& operator ++ ();
        HashMap<KEY,T,thash>::Iterator  operator ++ (int);
        bool operator == (const HashMap<KEY,T,thash>::Iterator& rhs) const;
        bool operator != (const HashMap<KEY,T,thash>::Iterator& rhs) const;
        Entry& operator *  () const;
        Entry* operator -> () const;
        friend std::ostream& operator << (std::ostream& outs, const HashMap<KEY,T,thash>::Iterator& i) {
          outs << i.str(); //Use the same meaning as the debugging .str() method
          return outs;
        }
        friend Iterator HashMap<KEY,T,thash>::begin () const;
        friend Iterator HashMap<KEY,T,thash>::end   () const;

      private:
        //If can_erase is false, current indexes the "next" value (must ++ to reach it)
        Cursor                current; //Bin Index and Cursor; stops if LN* == nullptr
        HashMap<KEY,T,thash>* ref_map;
        int                   expected_mod_count;
        bool                  can_erase = true;

        //Helper methods
        void advance_cursors();

        //Called in friends begin/end
        Iterator(HashMap<KEY,T,thash>* iterate_over, bool from_begin);
    };


    Iterator begin () const;
    Iterator end   () const;


  private:
    class LN {
    public:
      LN ()                         : next(nullptr){}
      LN (const LN& ln)             : value(ln.value), next(ln.next){}
      LN (Entry v, LN* n = nullptr) : value(v), next(n){}

      Entry value;
      LN*   next;
  };

  int (*hash)(const KEY& k);  //Hashing function used (from template or constructor)
  LN** map      = nullptr;    //Pointer to array of pointers: each bin stores a list with a trailer node
  double load_threshold;      //used/bins <= load_threshold
  int bins      = 1;          //# bins in array (should start >= 1 so hash_compress doesn't % 0)
  int used      = 0;          //Cache for number of key->value pairs in the hash table
  int mod_count = 0;          //For sensing concurrent modification


  //Helper methods
  int   hash_compress        (const KEY& key)          const;  //hash function ranged to [0,bins-1]
  LN*   find_key             (LN* front, const KEY& key) const;           //Returns reference to key's node or nullptr
  LN*   copy_list            (LN*   l)                 const;  //Copy the keys/values in a bin (order irrelevant)
  LN**  copy_hash_table      (LN** ht, int bins)       const;  //Copy the bins/keys/values in ht tree (order in bins irrelevant)

  void  ensure_load_threshold(int new_used);                   //Reallocate if load_factor > load_threshold
  void  delete_hash_table    (LN**& ht, int bins);             //Deallocate all LN in ht (and the ht itself; ht == nullptr)
};





////////////////////////////////////////////////////////////////////////////////
//
//HashMap class and related definitions

//Destructor/Constructors

template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>::~HashMap() {
    delete [] map;

}


template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>::HashMap(double the_load_threshold, int (*chash)(const KEY& k))
: hash(thash != (hashfunc)undefinedhash<KEY> ? thash : chash)
{
    if (hash == (hashfunc)undefinedhash<KEY>)
        throw TemplateFunctionError("HashMap::default constructor: neither specified");
    if (thash != (hashfunc)undefinedhash<KEY> && chash != (hashfunc)undefinedhash<KEY> && thash != chash)
        throw TemplateFunctionError("HashMap::default constructor: both specified and different");

    map = new LN*[bins];
    map[0]=new LN();
    load_threshold=1;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>::HashMap(int initial_bins, double the_load_threshold, int (*chash)(const KEY& k))
: hash(thash != (hashfunc)undefinedhash<KEY> ? thash : chash), bins(initial_bins)
{
    if (hash == (hashfunc)undefinedhash<KEY>)
        throw TemplateFunctionError("HashMap::length constructor: neither specified");
    if (thash != (hashfunc)undefinedhash<KEY> && chash != (hashfunc)undefinedhash<KEY> && thash != chash)
        throw TemplateFunctionError("HashMap::length constructor: both specified and different");

    bins = initial_bins;
    map = new LN*[bins];
    for(int i=0; i < bins; i++ ){
        map[i] = new LN();
    }

}


template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>::HashMap(const HashMap<KEY,T,thash>& to_copy, double the_load_threshold, int (*chash)(const KEY& a))
: hash(to_copy.hash), used(to_copy.used), bins(to_copy.bins)
{
    if (hash == (hashfunc)undefinedhash<KEY>)
        hash = to_copy.hash;
    if (thash != (hashfunc)undefinedhash<KEY> && chash != (hashfunc)undefinedhash<KEY> && thash != chash)
        throw TemplateFunctionError("HashMap::copy constructor: both specified and different");

    map = new LN*[bins];
    if (hash == to_copy.hash) {
        used = to_copy.used;
        for (int i = 0; i < to_copy.bins; ++i)
            map[i] = copy_list(to_copy.map[i]);
    }
    else {
        map = new LN*[bins];
        for (int i = 0; i < bins; ++i)
            map[i] = new LN();
        for (int i = 0; i < to_copy.bins; ++i)
            for (LN * p = to_copy.map[i]; p->next != nullptr; p = p->next)
                put(p->value.first, p->value.second);

    }



}


template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>::HashMap(const std::initializer_list<Entry>& il, double the_load_threshold, int (*chash)(const KEY& k))
: hash(thash != (hashfunc)undefinedhash<KEY> ? thash : chash), bins(il.size())
{
    if (hash == (hashfunc)undefinedhash<KEY>)
        throw TemplateFunctionError("HashMap::initializer_list constructor: neither specified");
    if (thash != (hashfunc)undefinedhash<KEY> && chash != (hashfunc)undefinedhash<KEY> && thash != chash)
        throw TemplateFunctionError("HashMap::initializer_list constructor: both specified and different");

    map = new LN*[bins];
    for (int i = 0; i<bins; ++i)
        map[i] = new LN();

    for (const Entry& entry : il)
        put(entry.first,entry.second);
}


template<class KEY,class T, int (*thash)(const KEY& a)>
template <class Iterable>
HashMap<KEY,T,thash>::HashMap(const Iterable& i, double the_load_threshold, int (*chash)(const KEY& k))
: hash(thash != (hashfunc)undefinedhash<KEY> ? thash : chash), bins(i.size())
{
    if (hash == (hashfunc)undefinedhash<KEY>)
        throw TemplateFunctionError("HashMap::Iterable constructor: neither specified");
    if (thash != (hashfunc)undefinedhash<KEY> && chash != (hashfunc)undefinedhash<KEY> && thash != chash)
        throw TemplateFunctionError("HashMap::Iterable constructor: both specified and different");

    map = new LN*[bins];
    for (int i = 0; i<bins; ++i)
        map[i] = new LN();

    for (const Entry& entry : i)
        put(entry.first,entry.second);
}


////////////////////////////////////////////////////////////////////////////////
//
//Queries

template<class KEY,class T, int (*thash)(const KEY& a)>
bool HashMap<KEY,T,thash>::empty() const {
    return used==0;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
int HashMap<KEY,T,thash>::size() const {
    return used;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
bool HashMap<KEY,T,thash>::has_key (const KEY& key) const {
    return find_key(map[hash_compress(key)],key)!=nullptr;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
bool HashMap<KEY,T,thash>::has_value (const T& value) const {
    for(int i=0; i<bins; i++) {
        for (LN *p = map[i]; p->next != nullptr; p = p->next) {
            if (p->value.second == value) {
                return true;
            }
        }
    }
    return false;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
std::string HashMap<KEY,T,thash>::str() const {
    std::stringstream temp;
    for(int i=0; i<bins; i++){
        temp << " " << i << " : [";
        for(LN* p=map[i]; p->next!=nullptr; p=p->next){

            temp << '(' << p->value.first << ',' << p->value.second << ')';
        }
        temp << "]";
    }
    return temp.str();
}


////////////////////////////////////////////////////////////////////////////////
//
//Commands

template<class KEY,class T, int (*thash)(const KEY& a)>
T HashMap<KEY,T,thash>::put(const KEY& key, const T& value) {
    if(has_key(key)){
        T to_return=find_key(map[hash_compress(key)],key)->value.second;
        find_key(map[hash_compress(key)],key)->value.second=value;
        return to_return;
    }
    used++;
    ensure_load_threshold(used);
    map[hash_compress(key)]=new LN(ics::make_pair(key,value),map[hash_compress(key)]);
    ++mod_count;
    return value;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
T HashMap<KEY,T,thash>::erase(const KEY& key) {
    if(!has_key(key))
        throw KeyError("Key not in Map");
    LN* prev=map[hash_compress(key)];
    T to_return=find_key(map[hash_compress(key)], key)->value.second;
    if(prev->value.first==key){
        map[hash_compress(key)]=prev->next;
        delete prev;
    }
    else {
        for (LN *p = map[hash_compress(key)]->next; p->next != nullptr; p = p->next) {
            if(p->value.first==key){
                LN* to_delete=p;
                prev->next=p->next;
                delete to_delete;
                break;
            }
            prev=prev->next;
        }
    }
    used--;
    ++mod_count;
    return to_return;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
void HashMap<KEY,T,thash>::clear() {
    bins=1;
    map = new LN*[bins];
    map[0]=new LN;
    used=0;
    ++mod_count;
    load_threshold=1;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
template<class Iterable>
int HashMap<KEY,T,thash>::put_all(const Iterable& i) {
    int count = 0;
    for (const Entry& m_entry : i) {
        ++count;
        put(m_entry.first, m_entry.second);
    }

    return count;
}


////////////////////////////////////////////////////////////////////////////////
//
//Operators

template<class KEY,class T, int (*thash)(const KEY& a)>
T& HashMap<KEY,T,thash>::operator [] (const KEY& key) {
    T value;
    if(!has_key(key)){
        put(key, value);
    }
    return find_key(map[hash_compress(key)],key)->value.second;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
const T& HashMap<KEY,T,thash>::operator [] (const KEY& key) const {
    return find_key(map[hash_compress(key)],key)->value.second;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>& HashMap<KEY,T,thash>::operator = (const HashMap<KEY,T,thash>& rhs) {
    if (this == &rhs)
        return *this;
    clear();
    hash = rhs.hash;
    for(int i=0; i<rhs.bins; i++){
        for(LN* p=rhs.map[i]; p->next!=nullptr; p=p->next){
            put(p->value.first,p->value.second);
        }
    }
    ++mod_count;
    return *this;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
bool HashMap<KEY,T,thash>::operator == (const HashMap<KEY,T,thash>& rhs) const {
    if (this == &rhs)
        return true;
    if (used != rhs.size())
        return false;

    for(int i=0; i<rhs.bins; i++){
        for(LN* p=rhs.map[i]; p->next!=nullptr; p=p->next){
            if(has_key(p->value.first)){
                if(find_key(map[hash_compress(p->value.first)],p->value.first)->value.second!=p->value.second){
                    return false;
                }
            }
            else{
                return false;
            }
        }
    }
    return true;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
bool HashMap<KEY,T,thash>::operator != (const HashMap<KEY,T,thash>& rhs) const {
    return !(*this == rhs);
}


template<class KEY,class T, int (*thash)(const KEY& a)>
std::ostream& operator << (std::ostream& outs, const HashMap<KEY,T,thash>& m) {
    outs << "map[";
    int count =0;
    for (int i = 0; i < m.bins; ++i)
        for (typename HashMap<KEY,T,thash>::LN* p = m.map[i]; p->next != nullptr; p = p->next){
            if (count++ == 0)
                outs << "" ;
            else
                outs << ",";
            outs << p->value.first << "->" << p->value.second;
        }

    outs << "]";
    return outs;
}


////////////////////////////////////////////////////////////////////////////////
//
//Iterator constructors

template<class KEY,class T, int (*thash)(const KEY& a)>
auto HashMap<KEY,T,thash>::begin () const -> HashMap<KEY,T,thash>::Iterator {
    return Iterator(const_cast<HashMap<KEY,T,thash>*>(this),true);
}


template<class KEY,class T, int (*thash)(const KEY& a)>
auto HashMap<KEY,T,thash>::end () const -> HashMap<KEY,T,thash>::Iterator {
    return Iterator(const_cast<HashMap<KEY,T,thash>*>(this),false);
}


////////////////////////////////////////////////////////////////////////////////
//
//Private helper methods

template<class KEY,class T, int (*thash)(const KEY& a)>
int HashMap<KEY,T,thash>::hash_compress (const KEY& key) const {
    return std::abs(thash(key))%bins;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
typename HashMap<KEY,T,thash>::LN* HashMap<KEY,T,thash>::find_key (LN* front, const KEY& key) const {
    if (front->next==nullptr){
        return front->next;
    }
    if(front->value.first==key){
        return front;
    }
    return find_key(front->next, key);
}


template<class KEY,class T, int (*thash)(const KEY& a)>
typename HashMap<KEY,T,thash>::LN* HashMap<KEY,T,thash>::copy_list (LN* l) const {
    if (l == nullptr)
        return nullptr;
    else
        return new LN(l->value, copy_list(l->next));
}


template<class KEY,class T, int (*thash)(const KEY& a)>
typename HashMap<KEY,T,thash>::LN** HashMap<KEY,T,thash>::copy_hash_table (LN** ht, int bins) const {
}


template<class KEY,class T, int (*thash)(const KEY& a)>
void HashMap<KEY,T,thash>::ensure_load_threshold(int new_used) {
    if(new_used/bins > load_threshold){
        int old_bins = bins;
        bins = 2*bins;
        LN** new_map = new LN*[bins];
        for(int i=0; i<bins; i++ ){
            new_map[i] = new LN;
        }
        for(int i=0; i< old_bins; i++){
            for(LN* p=map[i]; p->next!= nullptr; p=p->next){
                int index = std::abs(thash(p->value.first))%bins;
                new_map[index]=new LN(p->value, new_map[index]);
            }
        }
        map = new_map;
    }
}


template<class KEY,class T, int (*thash)(const KEY& a)>
void HashMap<KEY,T,thash>::delete_hash_table (LN**& ht, int bins) {
}






////////////////////////////////////////////////////////////////////////////////
//
//Iterator class definitions

template<class KEY,class T, int (*thash)(const KEY& a)>
void HashMap<KEY,T,thash>::Iterator::advance_cursors(){
    //not trailer node and contains at least one element
    if (current.second != nullptr && current.second->next != nullptr && current.second->next->next != nullptr) {
        current.second = current.second->next;
        return;
    }
        //it's trailer node so we move to other bins higher than first one
    else
        for (int i = current.first + 1; i < ref_map->bins; ++i)
            if (ref_map->map[i]->next != nullptr) {
                current.first = i;
                current.second = ref_map->map[i];
                return;
            }
        //ran out of bins to check so we set (bin = -1,LN* ptr =  nullptr)
    current.first = -1;
    current.second = nullptr;

}


template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>::Iterator::Iterator(HashMap<KEY,T,thash>* iterate_over, bool from_begin)
: ref_map(iterate_over), expected_mod_count(ref_map->mod_count) {
    current = Cursor(-1, nullptr);
    if (from_begin)
        advance_cursors();

}


template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>::Iterator::~Iterator()
{}


template<class KEY,class T, int (*thash)(const KEY& a)>
auto HashMap<KEY,T,thash>::Iterator::erase() -> Entry {
    if (expected_mod_count != ref_map->mod_count)
        throw ConcurrentModificationError("HashMap::Iterator::erase");
    if (!can_erase)
        throw CannotEraseError("HashMap::Iterator::erase Iterator cursor already erased");
    if (current.second == nullptr)
        throw CannotEraseError("HashMap::Iterator::erase Iterator cursor beyond data structure");

    Entry to_return = current.second->value;

    LN * to_delete = current.second->next;
    *current.second = *current.second->next;
    delete to_delete;

    --ref_map->used;
    ++ref_map->mod_count;
    expected_mod_count = ref_map->mod_count;
    can_erase = false;

    return to_return;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
std::string HashMap<KEY,T,thash>::Iterator::str() const {

}

template<class KEY,class T, int (*thash)(const KEY& a)>
auto  HashMap<KEY,T,thash>::Iterator::operator ++ () -> HashMap<KEY,T,thash>::Iterator& {
    if (expected_mod_count != ref_map->mod_count)
        throw ConcurrentModificationError("HashMap::Iterator::operator ++");

    if (current.second == nullptr)
        return *this;

    if (can_erase || current.second->next == nullptr)
        advance_cursors();

    can_erase = true;

    return *this;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
auto  HashMap<KEY,T,thash>::Iterator::operator ++ (int) -> HashMap<KEY,T,thash>::Iterator {
    if (expected_mod_count != ref_map->mod_count)
        throw ConcurrentModificationError("HashMap::Iterator::operator ++(int)");

    Iterator to_return(*this);

    if (current.second == nullptr)
        return to_return;

    if (can_erase || current.second->next == nullptr)
        advance_cursors();

    can_erase = true;

    return to_return;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
bool HashMap<KEY,T,thash>::Iterator::operator == (const HashMap<KEY,T,thash>::Iterator& rhs) const {
    const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
    if (rhsASI == 0)
        throw IteratorTypeError("HashMap::Iterator::operator ==");
    if (expected_mod_count != ref_map->mod_count)
        throw ConcurrentModificationError("HashMap::Iterator::operator ==");
    if (ref_map != rhsASI->ref_map)
        throw ComparingDifferentIteratorsError("HashMap::Iterator::operator ==");

    return current.second == rhsASI->current.second;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
bool HashMap<KEY,T,thash>::Iterator::operator != (const HashMap<KEY,T,thash>::Iterator& rhs) const {
    const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
    if (rhsASI == 0)
        throw IteratorTypeError("HashMap::Iterator::operator !=");
    if (expected_mod_count != ref_map->mod_count)
        throw ConcurrentModificationError("HashMap::Iterator::operator !=");
    if (ref_map != rhsASI->ref_map)
        throw ComparingDifferentIteratorsError("HashMap::Iterator::operator !=");

    return current.second != rhsASI->current.second;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
pair<KEY,T>& HashMap<KEY,T,thash>::Iterator::operator *() const {
    if (expected_mod_count != ref_map->mod_count)
        throw ConcurrentModificationError("HashMap::Iterator::operator *");
    if (!can_erase || current.second == nullptr)
        throw IteratorPositionIllegal("HashMap::Iterator::operator * Iterator illegal:");

    return current.second->value;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
pair<KEY,T>* HashMap<KEY,T,thash>::Iterator::operator ->() const {
    if (expected_mod_count != ref_map->mod_count)
        throw ConcurrentModificationError("HashMap::Iterator::operator ->");
    if (!can_erase || current.second == nullptr)
        throw IteratorPositionIllegal("HashMap::Iterator::operator -> Iterator illegal:");

    return &(current.second->value);
}

}

#endif /* HASH_MAP_HPP_ */
