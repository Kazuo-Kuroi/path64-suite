// Original identity, pooled lifetime, immobile-value and exception coverage.
// UNSUPPORTED: c++98, c++03
#include <list>
#include <iterator>
#include <memory>
#include <utility>
#include <cassert>
#include <cstdlib>
#include <new>
#include <pthread.h>
static bool deny_new;
void* operator new(std::size_t n) {
    if (deny_new) throw std::bad_alloc();
    void* p=std::malloc(n?n:1); if (!p) throw std::bad_alloc(); return p;
}
void* operator new[](std::size_t n) { return ::operator new(n); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
#if __cplusplus >= 201402L
void operator delete(void* p,std::size_t) noexcept { std::free(p); }
void operator delete[](void* p,std::size_t) noexcept { std::free(p); }
#endif
struct Block { void* pointer; std::size_t count; int owner; };
static Block blocks[4096];
static int outstanding;
static pthread_mutex_t allocation_mutex=PTHREAD_MUTEX_INITIALIZER;
template<class T> struct Audit {
    typedef T value_type;
    template<class U> struct rebind { typedef Audit<U> other; };
    int owner;
    explicit Audit(int id=7) noexcept : owner(id) {}
    template<class U> Audit(const Audit<U>& other) noexcept : owner(other.owner) {}
    T* allocate(std::size_t n) {
        T* p=static_cast<T*>(::operator new(n*sizeof(T)));
        pthread_mutex_lock(&allocation_mutex);
        for (int i=0;i<4096;++i) if (!blocks[i].pointer) {
            blocks[i].pointer=p; blocks[i].count=n; blocks[i].owner=owner;
            ++outstanding; pthread_mutex_unlock(&allocation_mutex); return p;
        }
        assert(false); return 0;
    }
    void deallocate(T* p,std::size_t n) noexcept {
        pthread_mutex_lock(&allocation_mutex);
        for (int i=0;i<4096;++i) if (blocks[i].pointer==p) {
            assert(blocks[i].count==n && blocks[i].owner==owner);
            blocks[i].pointer=0; --outstanding;
            pthread_mutex_unlock(&allocation_mutex); ::operator delete(p); return;
        }
        assert(false);
    }
};
template<class T,class U> bool operator==(const Audit<T>& a,const Audit<U>& b) { return a.owner==b.owner; }
template<class T,class U> bool operator!=(const Audit<T>& a,const Audit<U>& b) { return !(a==b); }
struct Item {
    static int alive;
    int key,id;
    Item(int k,int i) noexcept : key(k),id(i) { __sync_add_and_fetch(&alive,1); }
    Item(const Item&)=delete;
    Item(Item&&)=delete;
    Item& operator=(const Item&)=delete;
    ~Item() { __sync_sub_and_fetch(&alive,1); }
};
int Item::alive;
bool operator<(const Item& a,const Item& b) { return a.key<b.key; }
typedef std::list<Item,Audit<Item>> List;
struct Less {
    int* budget;
    int* comparisons;
    bool operator()(const Item& a,const Item& b) const {
        if (*budget==0) throw 6;
        if (*budget>0) --*budget;
        ++*comparisons; return a.key<b.key;
    }
};
static void order(const List& list,const int* ids,int n) {
    assert(list.size()==std::size_t(n)); int i=0;
    for (auto p=list.begin();p!=list.end();++p,++i) { assert(i<n && p->id==ids[i]); }
    assert(i==n);
}
#ifndef TRANSFER_CONSUMER_ONLY
extern "C" std::list<int>* make_transfer_list() {
    auto result=new std::list<int>;
    result->push_back(37); result->push_back(41); return result;
}
#else
extern "C" std::list<int>* make_transfer_list();
#endif
static void* delete_partition(void* raw) { delete static_cast<List*>(raw); return 0; }
int main() {
    static_assert(noexcept(std::declval<List&>().splice(std::declval<List::const_iterator>(),std::declval<List&>())),"splice");
    {
        deny_new=true;
        List empty;
        List moved(std::move(empty));
        empty.splice(empty.end(),moved);
        deny_new=false;
    }
    {
        List destination;
        Item* saved=0;
        {
            List source;
            for (int i=1;i<=3;++i) source.emplace_back(i,i);
            destination.emplace_back(0,0);
            auto item=std::next(source.begin()); saved=std::addressof(*item);
            auto stable_end=destination.end();
            deny_new=true;
            destination.splice(destination.cend(),source,item);
            destination.splice(item,destination,item);
            destination.splice(std::next(item),destination,item);
            destination.splice(destination.cend(),std::move(source),source.cbegin(),source.cend());
            assert(stable_end==destination.end() && std::addressof(*item)==saved);
            assert(source.empty()); deny_new=false;
        }
        int expected[]={0,2,1,3}; order(destination,expected,4);
        assert(saved->id==2);
        destination.clear();
        deny_new=true;
        destination.emplace_back(8,8); // reuse a node from a dead source pool
        deny_new=false;
        assert(destination.front().id==8);
    }
    assert(outstanding==0 && Item::alive==0);
    {
        List source;
        for (int i=0;i<256;++i) source.emplace_back(i,i);
        auto original=source.begin(); Item* address=std::addressof(*original);
        {
            List destination;
            deny_new=true;
            destination.splice(destination.cend(),std::move(source));
            assert(source.empty() && destination.size()==256);
            assert(std::addressof(*original)==address);
            deny_new=false;
        }
        source.emplace_back(9,9); // source pool remains usable after destination dies
        assert(source.size()==1 && source.front().id==9);
    }
    assert(outstanding==0 && Item::alive==0);
    {
        List left,right;
        left.emplace_back(1,0); left.emplace_back(2,1); left.emplace_back(2,2);
        right.emplace_back(1,3); right.emplace_back(2,4); right.emplace_back(3,5);
        const Item* addresses[6];
        for (const auto& x:left) addresses[x.id]=std::addressof(x);
        for (const auto& x:right) addresses[x.id]=std::addressof(x);
        int budget=-1, comparisons=0;
        deny_new=true;
        left.merge(std::move(right),Less{&budget,&comparisons});
        int expected[]={0,3,1,2,4,5}; order(left,expected,6);
        assert(right.empty() && comparisons<=5);
        for (const auto& x:left) assert(addresses[x.id]==std::addressof(x));
        left.reverse(); left.sort();
        int sorted[]={3,0,4,2,1,5}; order(left,sorted,6);
        left.merge(left);
        deny_new=false;
    }
    for (int stop=0;stop<12;++stop) {
        List left,right;
        const Item* addresses[12];
        for (int i=0;i<12;++i) {
            List& owner=i%2?right:left;
            owner.emplace_back(i,i); addresses[i]=std::addressof(owner.back());
        }
        int budget=stop, comparisons=0;
        deny_new=true;
        try { left.merge(right,Less{&budget,&comparisons}); }
        catch(int error) { assert(error==6); }
        assert(left.size()+right.size()==12);
        bool seen[12]={};
        for (const auto& x:left) { assert(!seen[x.id] && addresses[x.id]==std::addressof(x)); seen[x.id]=true; }
        for (const auto& x:right) { assert(!seen[x.id] && addresses[x.id]==std::addressof(x)); seen[x.id]=true; }
        budget=-1; left.merge(right,Less{&budget,&comparisons});
        assert(right.empty() && left.size()==12);
        deny_new=false;
    }
    assert(outstanding==0 && Item::alive==0);
    for (int round=0;round<20;++round) {
        List* partitions[4];
        for (int i=0;i<4;++i) partitions[i]=new List;
        {
            List origin;
            for (int i=0;i<256;++i) origin.emplace_back(i,i);
            int which=0;
            while (!origin.empty()) {
                List& target=*partitions[which++%4];
                target.splice(target.cend(),origin,origin.cbegin());
            }
        }
        pthread_t threads[4];
        for (int i=0;i<4;++i) assert(pthread_create(threads+i,0,delete_partition,partitions[i])==0);
        for (int i=0;i<4;++i) assert(pthread_join(threads[i],0)==0);
        assert(outstanding==0 && Item::alive==0);
    }
    {
        std::list<int> destination;
        std::list<int>* source=make_transfer_list();
        destination.splice(destination.cend(),*source);
        delete source;
        assert(destination.size()==2 && destination.front()==37 && destination.back()==41);
    }
}
