#include<iostream>
using namespace std;

template<typename T>


class UniquePointer{

    private:
    T* ptr;
    public:
    explicit UniquePointer(T* p) : ptr(p){}
    ~UniquePointer(){
        cout << "Memory freed" << endl;
        delete ptr;
    }

    T& operator*(){return *ptr;}
};

template<typename P>
class SharedPtr{
    private:
    P* ptr;
    int *count;

    public:
    explicit SharedPtr(P* p) : ptr(p){
        count = new int(1);
    }
    SharedPtr( const SharedPtr& other){
        ptr = other.ptr;
        count = other.count;
        (*count)++;
    }
    SharedPtr(P* rawPtr, int* c) : ptr(rawPtr), count(c){
        ptr = rawPtr;
        count = c;
        (*count)++;
    }

    
    ~SharedPtr(){
        (*count)--;
        cout << "Count: " << *count << endl;
        if(*count == 0){
            cout << "Memory freed" << endl;
            delete ptr; 
            delete count;
        }
        
    }
    explicit operator bool() const{
        return ptr != nullptr;
    }

    P& operator*(){return *ptr;}

    P* getPtr() const { return ptr; }
    int* getCount() const { return count; }
};

template<typename W>

class WeekPtr{

    private :
    W* ptr;
    int* strongCount;

    public:
    WeekPtr(const SharedPtr<W>& sharedPtr){
        ptr = sharedPtr.getPtr();
        strongCount = sharedPtr.getCount();
    }
    ~WeekPtr(){
        
            cout << "Weak pointer destroyed" << endl;
        
    }
    SharedPtr<W> lock(){
        if(strongCount != nullptr && *strongCount > 0){
            cout << "you can access the object" << endl;
            return SharedPtr<W>(ptr,strongCount);
        }
        else{
            cout << "Object is already deleted" << endl;
            return SharedPtr<W>(nullptr);
        }
    }
    bool expired(){
        return (strongCount == nullptr || *strongCount == 0);
    }
    

};

int main(){

    UniquePointer<int> p(new int(10));
    UniquePointer<int> p1(new int(20));

    SharedPtr<int> p2(new int(30));
    SharedPtr<int> p3 = p2;

    SharedPtr<int> sp(new int(40));
    WeekPtr<int> wp(sp);

    cout << "Is expired: " << wp.expired() << endl;
    SharedPtr<int> sp1 = wp.lock();
    if(sp1){
        cout << "Value:"<< *sp1 << endl;
    }
    else{
        cout << "Object is already deleted" << endl;
    }
    cout << *p << endl;
    cout << *p1 << endl;
    cout << *p2 << endl;
    cout << *p3 << endl;
    return 0;

}