// Original regression coverage for vector relocation and rollback.
#include <vector>
#include <cassert>

struct Element {
    static int alive;
    static int copies_before_throw;
    int value;
    explicit Element(int v = 0) : value(v) { ++alive; }
    Element(const Element& other) : value(other.value) {
        if (copies_before_throw == 0) throw 7;
        if (copies_before_throw > 0) --copies_before_throw;
        ++alive;
    }
    Element(Element&& other) noexcept(false) : value(other.value) {
        other.value = -1;
        ++alive;
    }
    Element& operator=(const Element& other) { value = other.value; return *this; }
    Element& operator=(Element&& other) { value = other.value; other.value = -1; return *this; }
    ~Element() { --alive; }
};
int Element::alive = 0;
int Element::copies_before_throw = -1;

int main() {
    {
        std::vector<Element> values;
        values.reserve(3);
        values.emplace_back(1);
        values.emplace_back(2);
        values.emplace_back(3);
        Element* original = values.data();
        for (int count = 0; count != 3; ++count) {
            Element::copies_before_throw = count;
            bool caught = false;
            try { values.emplace(values.begin() + 1, 9); }
            catch (int e) { assert(e == 7); caught = true; }
            assert(caught);
            assert(values.data() == original);
            assert(values.size() == 3);
            assert(Element::alive == 3);
            for (int i = 0; i != 3; ++i) assert(values[i].value == i + 1);
        }
        Element::copies_before_throw = -1;
        values.push_back(values[0]);
        assert(values.back().value == 1);
        values.reserve(10);
        values.emplace(values.begin() + 1, values.back());
        assert(values[1].value == 1);
        values.shrink_to_fit();
        assert(values.capacity() == values.size());
    }
    assert(Element::alive == 0);
    {
        std::vector<Element> values(3);
        Element incoming[] = {Element(7), Element(8)};
        for (int count = 0; count != 5; ++count) {
            Element::copies_before_throw = count;
            bool caught = false;
            try { values.insert(values.begin() + 1, incoming, incoming + 2); }
            catch (int) { caught = true; }
            assert(caught);
            assert(values.size() == 3);
            assert(Element::alive == 5);
            for (int i = 0; i != 3; ++i) assert(values[i].value == 0);
        }
        Element::copies_before_throw = -1;
    }
    assert(Element::alive == 0);
}
