# Smart Pointer Library (C++)

Week 1 of a 48-week systems engineering roadmap. A from-scratch implementation of `UniquePointer`, `SharedPtr`, and `WeakPtr` in modern C++ — built to understand what `std::unique_ptr`, `std::shared_ptr`, and `std::weak_ptr` actually do under the hood.

## What's implemented

- **UniquePointer** — RAII wrapper with exclusive ownership. Destructor frees memory automatically.
- **SharedPtr** — reference-counted shared ownership.
  - Copy constructor shares the existing counter and increments it.
  - A second constructor `SharedPtr(P* rawPtr, int* c)` shares an *existing* counter — used internally by `WeakPtr::lock()`.
  - `operator bool()`, `operator*()`, `getPtr()`, `getCount()`.
- **WeakPtr** — non-owning observer of a `SharedPtr`.
  - Holds the same raw pointer and the same counter address, but never increments it.
  - `expired()` — checks if the counter has hit 0.
  - `lock()` — returns a valid `SharedPtr` if the object is still alive, or a null `SharedPtr` if it's expired.

## The bug I found and fixed

My first working version of `WeakPtr::lock()` looked like this:

```cpp
return SharedPtr<W>(ptr);
```

This compiled and ran without crashing, which made it look correct — but it was silently building a **double free**.

`SharedPtr(P* p)` (the raw-pointer constructor) always allocates a *brand new* counter with `count = new int(1)`. So the `SharedPtr` returned by `lock()` had its own independent counter, completely disconnected from the original `SharedPtr`'s counter, even though both pointed at the *same* heap object.

Result: when the `lock()`-returned `SharedPtr` went out of scope, its counter hit 0 and it called `delete ptr`. Later, when the *original* `SharedPtr` went out of scope, its counter *also* hit 0, and it called `delete ptr` again — on an address already freed. That's undefined behavior. It happened not to crash on this run, which is exactly why the bug was dangerous: it looked fine until it wasn't.

**The fix:** add a constructor that shares an *existing* counter instead of allocating a new one:

```cpp
SharedPtr(P* rawPtr, int* c) : ptr(rawPtr), count(c) {
    (*count)++;
}
```

And change `lock()` to use it:

```cpp
return SharedPtr<W>(ptr, strongCount);
```

Now both `SharedPtr`s share one counter. Only the *last* owner to go out of scope brings the count to 0 and actually deletes — exactly once.

### Before vs after (terminal output)

**Before (buggy):**
```
Count: 0
Memory freed        <- sp1's independent counter hits 0, deletes
...
Count: 0
Memory freed        <- sp's independent counter hits 0, deletes AGAIN (double free)
```

**After (fixed):**
```
Count: 1             <- sp1 destructs, shared count 2 -> 1, no delete
Weak pointer destroyed
Count: 0             <- sp destructs, shared count 1 -> 0, delete happens
Memory freed          (exactly once)
```

## Key concepts learned

- RAII — tying cleanup to object lifetime, not manual calls
- Why the reference counter must live on the heap, not inside the object itself, so every owner can share the same one
- Ownership vs. observation: `WeakPtr` never increments the counter because it isn't an owner and has no power to keep the object alive
- Why "it ran without crashing" isn't proof of correctness — undefined behavior can silently pass on one run and fail on another
- `expired()` / `lock()` pattern for safe temporary access to a possibly-deleted object

## Build & run

```bash
g++ -std=c++17 -Wall smart_pointer.cpp -o smart_ptr
./smart_ptr
```

## Next up

Week 2 — STL Vector from scratch (dynamic arrays, amortized growth, benchmarking against `std::vector`).