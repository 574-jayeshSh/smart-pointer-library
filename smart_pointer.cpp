#include<iostream>
using namespace std;

template<typename T>


class UniquePointer{

    private:
    T* ptr;
    public:
    explicit UniquePointer(T* p) : ptr(p){}

    UniquePointer(UniquePointer&& other){
        ptr = other.ptr;
        other.ptr = nullptr;
    }
    
    ~UniquePointer(){
        cout << "Memory freed" << endl;
        delete ptr;
    }

    T& operator*(){return *ptr;}
    UniquePointer& operator=(UniquePointer&& other){
        if(this != &other){
            delete ptr;
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }
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
    

    p =move(p1);
    
    return 0;

}