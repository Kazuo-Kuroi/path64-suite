// Original regressions for scope destruction on break and continue.
#include <cassert>
static int live, log_ids[128], used;
struct Guard {
    int id;
    Guard(int i): id(i) { ++live; }
    ~Guard() { --live; log_ids[used++] = id; }
};
int main() {
    {
        Guard outer(99);
        for (int i = 0; i < 3; ++i) {
            Guard a(10+i);
            { Guard b(20+i); if (i != 1) continue; }
            assert(live == 2);
        }
        assert(live == 1 && used == 6);
        for (int i = 0; i < 3; ++i) {
            assert(log_ids[2*i] == 20+i);
            assert(log_ids[2*i+1] == 10+i);
        }
        used = 0;
        for (int i = 0; i < 3; ++i) {
            Guard a(1);
            { Guard b(2); break; }
        }
        assert(live == 1 && used == 2 && log_ids[0] == 2 && log_ids[1] == 1);
        used = 0;
        for (int i = 0; i < 2; ++i) {
            Guard loop(3);
            switch (i) {
            case 0: { Guard branch(4); continue; }
            default: { Guard branch(5); break; }
            }
            assert(live == 2);
        }
        assert(live == 1 && used == 4);
        assert(log_ids[0] == 4 && log_ids[1] == 3);
        assert(log_ids[2] == 5 && log_ids[3] == 3);
        used = 0;
        int count = 0;
        while (++count < 4) {
            Guard a(6);
            if (count < 3) continue;
            break;
        }
        assert(live == 1 && used == 3);
        count = 0;
        do {
            Guard a(7);
            ++count;
            if (count < 3) continue;
            break;
        } while (count < 5);
        assert(live == 1 && used == 6);
#if __cplusplus >= 201103L
        int range[] = {0,1,2};
        for (int i: range) {
            Guard a(8);
            if (i < 2) continue;
            break;
        }
        assert(live == 1 && used == 9);
#endif
        // A loop inside an outer body must retain that body's live objects.
        for (int i = 0; i < 2; ++i) {
            Guard a(9);
            for (int j = 0; j < 2; ++j) { Guard b(10); break; }
            assert(live == 2);
        }
        assert(live == 1);
    }
    assert(live == 0);
}
