#include "smart_pointer.h"

int main(){

    UniquePointer<int> p(new int(10));
    UniquePointer<int> p1(new int(20));
    
    p = move(p1);
    
    return 0;

}
