// Open follow-up: invalid new[] bounds should select bad_array_new_length.
// Both current compiler paths over this runtime instead reach bad_alloc.
// Kept outside the .pass.cpp suite until array-bound lowering is implemented.
#include <new>
#include <cassert>
int main() {
    volatile long size = -1;
    try { int* p = new int[size]; delete[] p; assert(false); }
    catch (const std::bad_array_new_length&) { return 0; }
    catch (const std::bad_alloc&) { assert(false); }
}
