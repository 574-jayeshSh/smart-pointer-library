# Smart Pointers Implementation (C++)

A custom implementation of `UniquePointer`, `SharedPtr`, and `WeekPtr` (Weak Pointer) in C++ with reference counting.

## Files

| File | Description |
|------|-------------|
| `smart_pointer.h` | Header with all class definitions |
| `smart_pointer.cpp` | Driver file with `main()` |
| `test_smart_pointers.cpp` | Test suite with 18 test cases |

## How to Compile and Run

```bash
# Run main program
g++ -std=c++14 -o main smart_pointer.cpp
./main

# Run tests
g++ -std=c++14 -o tests test_smart_pointers.cpp
./tests
```

## Test Output

```
===== UniquePointer Tests =====
[PASS] UniquePtr: dereference
Memory freed
[PASS] UniquePtr: move assignment value
Memory freed
Memory freed
[PASS] UniquePtr: string type
Memory freed
[PASS] UniquePtr: move constructor
Memory freed
Memory freed

===== SharedPtr Tests =====
[PASS] SharedPtr: create and dereference
Count: 0
Memory freed
[PASS] SharedPtr: copy shares data
Count: 1
Count: 0
Memory freed
[PASS] SharedPtr: three copies share data
Count: 2
Count: 1
Count: 0
Memory freed
[PASS] SharedPtr: copy assignment
Count: 1
Count: 0
Memory freed
[PASS] SharedPtr: default constructor is null
Count: 2
Count: 1
[PASS] SharedPtr: copy survives original scope
Count: 0
Memory freed
[PASS] SharedPtr: self assignment safe
Count: 1
Count: 0
Memory freed

===== WeekPtr Tests =====
[PASS] WeakPtr: not expired when SharedPtr alive
Weak pointer destroyed
Count: 0
Memory freed
you can access the object
[PASS] WeakPtr: lock succeeds while alive
Count: 1
Weak pointer destroyed
Count: 0
Memory freed
you can access the object
Count: 2
Weak pointer destroyed
Count: 1
[PASS] WeakPtr: lock valid after scope
Count: 0
Memory freed
Object is already deleted
[PASS] WeakPtr: lock on null returns null
Weak pointer destroyed
you can access the object
Count: 2
Weak pointer destroyed
Count: 1
[PASS] WeakPtr: data survives after original SharedPtr destroyed
Count: 0
Memory freed
Weak pointer destroyed
you can access the object
Count: 3
Count: 2
Count: 1
[PASS] WeakPtr: not expired with another SharedPtr alive
Count: 0
Memory freed
Weak pointer destroyed
Weak pointer destroyed
Count: 1
Count: 0
Memory freed
[PASS] WeakPtr: expired after all SharedPtrs destroyed
Weak pointer destroyed

===== Results =====
Passed: 18
Failed: 0
```

---

## Bugs Found and Fixed

### Bug 1: Typo `strongount` (WeekPtr destructor)

**Wrong:**
```cpp
if(*wp->strongount == 0 && *wp->weakCount == 0){
```

**Fixed:**
```cpp
if(*wp->strongCount == 0 && *wp->weakCount == 0){
```

**Why it matters:** Compiler error — the program won't build.

---

### Bug 2: Wrong operator precedence in `lock()`

**Wrong:**
```cpp
if(*wp->strongCount != nullptr && *wp->strongCount > 0){
```

**Fixed:**
```cpp
if(wp->strongCount != nullptr && *wp->strongCount > 0){
```

**Why it matters:** `->` binds before `*`, so `*wp->strongCount` dereferences the `int*` and compares an `int` to `nullptr`. This is always wrong — you want to check if the pointer itself is valid, not dereference it.

---

### Bug 3: Invalid return syntax in `lock()`

**Wrong:**
```cpp
return SharedPtr<W>* wp;
```

**Fixed:**
```cpp
*wp->strongCount += 1;
return SharedPtr<W>(wp);
```

**Why it matters:** `SharedPtr<W>* wp` is parsed as a type declaration, not a return expression. Additionally, `lock()` must increment `strongCount` before returning, otherwise the returned SharedPtr holds a reference that was never counted — causing use-after-free when other SharedPtrs see count = 0 and delete the object while this one still uses it.

---

### Bug 4: `SharedPtr<W>(nullptr)` with no matching constructor

**Wrong:**
```cpp
return SharedPtr<W>(nullptr);
```

`SharedPtr` only had `explicit SharedPtr(ControlBlock<P>*)` — no constructor accepting raw pointer or `nullptr`.

**Fixed:** Added constructors:
```cpp
SharedPtr(P* p) : sp(new ControlBlock<P>(p)){}
SharedPtr() : sp(nullptr){}
```

And the return becomes:
```cpp
return SharedPtr<W>();
```

**Why it matters:** Compiler error. Even if it compiled, a null `sp` would crash the destructor.

---

### Bug 5: `lock()` didn't increment `strongCount`

**Why it matters:** This is the most dangerous bug. Without incrementing the count, the returned SharedPtr holds a reference that isn't tracked. When the last "real" SharedPtr dies, it sets `strongCount = 0` and frees the memory — while the locked SharedPtr still uses it. This is a **use-after-free** vulnerability.

---

### Bug 6: No null guard in `~SharedPtr()`

**Wrong:**
```cpp
~SharedPtr(){
    (*sp->strongCount)--;
```

**Fixed:**
```cpp
~SharedPtr(){
    if(sp == nullptr) return;
    (*sp->strongCount)--;
```

**Why it matters:** A default-constructed SharedPtr has `sp = nullptr`. Destroying it crashes with a null pointer dereference.

---

### Bug 7: No copy assignment operator for SharedPtr

Only the copy constructor was defined. Using `a = b` called the compiler-generated shallow copy, which overwrites `a.sp` without decrementing the old reference count.

**Why it matters:** The old object that `a` was pointing to is leaked (count never decremented), and `a` shares the control block with `b` without incrementing the count. When both are destroyed, the count goes below zero and the object is freed at the wrong time — **double-free / memory leak**.

---

### Bug 8: UniquePointer missing deleted copy operations

Move constructor and move assignment were defined, but copy was implicitly generated by the compiler.

**Fixed:**
```cpp
UniquePointer(const UniquePointer&) = delete;
UniquePointer& operator=(const UniquePointer&) = delete;
```

**Why it matters:** If someone accidentally copies a UniquePointer (e.g. passing by value), both copies delete the same memory — **double-free**.

---

### Bug 9: `operator bool()` crash on null `sp`

**Wrong:**
```cpp
explicit operator bool() const{
    return sp->ptr != nullptr;
}
```

**Fixed:**
```cpp
explicit operator bool() const{
    return sp != nullptr && sp->ptr != nullptr;
}
```

**Why it matters:** Null pointer dereference when checking a default-constructed SharedPtr.

---

### Bug 10: `WeekPtr` constructor didn't increment `weakCount`

**Wrong:**
```cpp
WeekPtr(const SharedPtr<W>& sharedPtr){
    wp = sharedPtr.getBlock();
}
```

**Fixed:**
```cpp
WeekPtr(const SharedPtr<W>& sharedPtr){
    wp = sharedPtr.getBlock();
    if(wp != nullptr){
        *wp->weakCount += 1;
    }
}
```

**Why it matters:** The control block tracks both strong and weak references. If `weakCount` isn't incremented, the destructor logic (`if strongCount == 0 && weakCount == 0 → delete control block`) triggers incorrectly — either deleting the control block while WeekPtrs still exist (**use-after-free**) or never cleaning it up (**memory leak**).
