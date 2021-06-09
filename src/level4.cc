#include <cassert>
#include <iostream>

bool has_double1(std::string const& s) {
    unsigned char last = s[0];
    for (int i = 1; i < s.length(); ++i) {
        if(last == s[i]) {
            return true;
        }
        last = s[i];
    }
    return false;
}

bool has_double2(std::string const& s) {
    unsigned cnt = 1;
    unsigned char last = s[0];
    for (int i = 1; i < s.length(); ++i) {
        if(last == s[i]) {
            ++cnt;
        } else if (cnt == 2) {
            return true;
        } else {
            last = s[i];
            cnt = 1;
        }
    }
    return cnt == 2;
}

std::string fix_order(std::string s) {
    unsigned char last = s[0];
    for (int i = 1; i < s.length(); ++i) {
        if(last > s[i]) {
            s[i] = last;
        } else {
            last = s[i];
        }
    }
    return s;
}

std::string increment(std::string const& s) {
    return std::to_string(std::stoi(s) + 1);
}

int main()
{
    assert(has_double1("372037") == false);
    assert(has_double1("372027") == false);
    assert(has_double1("372237") == true);
    assert(has_double1("111111") == true);

    assert(fix_order("111111") == "111111");
    assert(fix_order("111122") == "111122");
    assert(fix_order("121111") == "122222");
    assert(fix_order("123456") == "123456");

    assert(increment("111111") == "111112");
    assert(increment("111119") == "111120");

    assert(has_double2("112233") == true);
    assert(has_double2("111122") == true);
    assert(has_double2("123444") == false);

    std::string from = "372037";
    std::string to = "905157";

    from = fix_order(from);

    unsigned cnt1 = 0, cnt2 = 0;
    for (; from < to; from = fix_order(increment(from))) {
        if (has_double1(from)) {
            ++cnt1;
        }
        if (has_double2(from)) {
            ++cnt2;
        }
    }

    std::cout << "1: " << cnt1 << "\n"
              << "2: " << cnt2 << "\n";

    return 0;
}
