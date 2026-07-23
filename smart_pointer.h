#ifndef SMART_POINTER_H
#define SMART_POINTER_H

#include<iostream>
using namespace std;

template<typename T>
class UniquePointer{

    private:
    T* ptr;
    public:
    explicit UniquePointer(T* p) : ptr(p){}
    UniquePointer(const UniquePointer&) = delete;
    UniquePointer& operator=(const UniquePointer&) = delete;

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
struct ControlBlock{
    P* ptr;
    int* strongCount;
    int* weakCount;

    ControlBlock(P* p): ptr(p), strongCount(new int(1)), weakCount(new int(0)) {}

    ~ControlBlock(){
        delete strongCount;
        delete weakCount;
    }
};

template<typename P>
class SharedPtr{
    private:
    ControlBlock<P>* sp;

    public:
    explicit SharedPtr(ControlBlock<P>* p) : sp(p){}
    SharedPtr(P* p) : sp(new ControlBlock<P>(p)){}
    SharedPtr() : sp(nullptr){}
    SharedPtr(const SharedPtr& other){
        sp = other.sp;
        *sp->strongCount += 1;
    }
    SharedPtr& operator=(const SharedPtr& other){
        if(this != &other){
            if(sp != nullptr){
                (*sp->strongCount)--;
                if(*sp->strongCount == 0){
                    delete sp->ptr;
                }
                if(*sp->strongCount == 0 && *sp->weakCount == 0){
                    delete sp;
                }
            }
            sp = other.sp;
            if(sp != nullptr){
                *sp->strongCount += 1;
            }
        }
        return *this;
    }

    ~SharedPtr(){
        if(sp == nullptr) return;
        (*sp->strongCount)--;
        cout << "Count: " << *sp->strongCount << endl;
        if(*sp->strongCount == 0){
            cout << "Memory freed" << endl;
            delete sp->ptr; 
        }
        if(*sp->strongCount == 0 && *sp->weakCount == 0){
            delete sp; 
        }
    }
    explicit operator bool() const{
        return sp != nullptr && sp->ptr != nullptr;
    }

    P& operator*(){return *sp->ptr;}
    P* operator->(){return sp->ptr;}
    P* get() const {return sp ? sp->ptr : nullptr;}

    ControlBlock<P>* getBlock() const {return sp;}
};

template<typename W>
class WeekPtr{

    private:
    ControlBlock<W>* wp;

    public:
    WeekPtr(const SharedPtr<W>& sharedPtr){
        wp = sharedPtr.getBlock();
        if(wp != nullptr){
            *wp->weakCount += 1;
        }
    }
    WeekPtr() : wp(nullptr){}
    WeekPtr(const WeekPtr& other){
        wp = other.wp;
        if(wp != nullptr){
            *wp->weakCount += 1;
        }
    }
    WeekPtr& operator=(const WeekPtr& other){
        if(this != &other){
            if(wp != nullptr){
                (*wp->weakCount)--;
                if(*wp->strongCount == 0 && *wp->weakCount == 0){
                    delete wp;
                }
            }
            wp = other.wp;
            if(wp != nullptr){
                *wp->weakCount += 1;
            }
        }
        return *this;
    }
    ~WeekPtr(){
            if(wp != nullptr){
                (*wp->weakCount)--;
                if(*wp->strongCount == 0 && *wp->weakCount == 0){
                    delete wp;
                }
            }
            cout << "Weak pointer destroyed" << endl;
    }
    SharedPtr<W> lock(){
        if(wp != nullptr && wp->strongCount != nullptr && *wp->strongCount > 0){
            cout << "you can access the object" << endl;
            *wp->strongCount += 1;
            return SharedPtr<W>(wp);
        }
        else{
            cout << "Object is already deleted" << endl;
            return SharedPtr<W>();
        }
    }
    bool expired(){
        return (wp == nullptr || wp->strongCount == nullptr || *wp->strongCount == 0);
    }   
};

#endif
