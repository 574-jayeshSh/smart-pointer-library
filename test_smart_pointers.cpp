#include<iostream>
#include<string>
#include "smart_pointer.h"
using namespace std;

int passed = 0;
int failed = 0;

void test(string name, bool result){
    if(result){
        cout << "[PASS] " << name << endl;
        passed++;
    } else {
        cout << "[FAIL] " << name << endl;
        failed++;
    }
}

void testUniquePointer(){
    cout << "\n===== UniquePointer Tests =====" << endl;

    {
        UniquePointer<int> p(new int(42));
        test("UniquePtr: dereference", *p == 42);
    }

    {
        UniquePointer<int> p(new int(10));
        UniquePointer<int> p2(new int(20));
        p = move(p2);
        test("UniquePtr: move assignment value", *p == 20);
    }

    {
        UniquePointer<string> p(new string("hello"));
        test("UniquePtr: string type", *p == "hello");
    }

    {
        UniquePointer<int> p1(new int(100));
        UniquePointer<int> p2(move(p1));
        test("UniquePtr: move constructor", *p2 == 100);
    }
}

void testSharedPtr(){
    cout << "\n===== SharedPtr Tests =====" << endl;

    {
        SharedPtr<int> p1(new int(50));
        test("SharedPtr: create and dereference", *p1 == 50);
    }

    {
        SharedPtr<int> p1(new int(100));
        SharedPtr<int> p2 = p1;
        test("SharedPtr: copy shares data", *p1 == 100 && *p2 == 100);
    }

    {
        SharedPtr<int> p1(new int(200));
        SharedPtr<int> p2 = p1;
        SharedPtr<int> p3 = p2;
        test("SharedPtr: three copies share data", *p1 == 200 && *p2 == 200 && *p3 == 200);
    }

    {
        SharedPtr<int> p1(new int(300));
        SharedPtr<int> p2(new int(400));
        p2 = p1;
        test("SharedPtr: copy assignment", *p2 == 300);
    }

    {
        SharedPtr<int> p1;
        test("SharedPtr: default constructor is null", !p1);
    }

    {
        SharedPtr<int> result;
        {
            SharedPtr<int> p1(new int(500));
            SharedPtr<int> p2 = p1;
            result = p2;
        }
        test("SharedPtr: copy survives original scope", result && *result == 500);
    }

    {
        SharedPtr<int> p1(new int(600));
        SharedPtr<int> p2(new int(700));
        p1 = p2;
        p1 = p1;
        test("SharedPtr: self assignment safe", *p1 == 700);
    }
}

void testWeakPtr(){
    cout << "\n===== WeekPtr Tests =====" << endl;

    {
        SharedPtr<int> sp(new int(100));
        WeekPtr<int> wp(sp);
        test("WeakPtr: not expired when SharedPtr alive", !wp.expired());
    }

    {
        SharedPtr<int> sp(new int(200));
        WeekPtr<int> wp(sp);
        SharedPtr<int> locked = wp.lock();
        test("WeakPtr: lock succeeds while alive", locked && *locked == 200);
    }

    {
        SharedPtr<int> locked;
        {
            SharedPtr<int> sp(new int(300));
            WeekPtr<int> wp(sp);
            locked = wp.lock();
        }
        test("WeakPtr: lock valid after scope", locked && *locked == 300);
    }

    {
        WeekPtr<int> wp;
        SharedPtr<int> locked = wp.lock();
        test("WeakPtr: lock on null returns null", !locked);
    }

    {
        SharedPtr<int> locked;
        {
            SharedPtr<int> sp(new int(400));
            WeekPtr<int> wp(sp);
            locked = wp.lock();
        }
        test("WeakPtr: data survives after original SharedPtr destroyed", locked && *locked == 400);
    }

    {
        WeekPtr<int> wp;
        SharedPtr<int> locked;
        {
            SharedPtr<int> sp(new int(500));
            SharedPtr<int> sp2 = sp;
            wp = WeekPtr<int>(sp);
            locked = wp.lock();
        }
        test("WeakPtr: not expired with another SharedPtr alive", !wp.expired());
    }

    {
        WeekPtr<int> wp;
        {
            SharedPtr<int> sp(new int(600));
            SharedPtr<int> sp2 = sp;
            wp = WeekPtr<int>(sp);
        }
        test("WeakPtr: expired after all SharedPtrs destroyed", wp.expired());
    }
}

int main(){
    testUniquePointer();
    testSharedPtr();
    testWeakPtr();

    cout << "\n===== Results =====" << endl;
    cout << "Passed: " << passed << endl;
    cout << "Failed: " << failed << endl;

    return 0;
}
