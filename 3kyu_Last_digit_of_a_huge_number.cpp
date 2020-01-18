#include <list>
#include <iostream>
#include <math.h>

//This kata about mathematic not a programming,but anyway.
//The stackoverflow magic is gave me a hit,
//and it used  by solved this kata.

using namespace std;

string PowerOperator(const unsigned long long int &base, const int &power)
{
    string res = to_string(base);
       
    reverse(res.begin(), res.end());

    for (int i = 2; i <= power; ++i)
    {
        int reminder = 0;
        string str_base = res;
        int indx = 0;
        res.clear();
        while (str_base[indx])
        {
            unsigned long long int mult = (str_base[indx++] - '0') * base + reminder;
            res.push_back(mult % 10 + '0');
            reminder = mult / 10;
        } //while 1

        while (reminder)
        {
            res.push_back(reminder % 10 + '0');
            reminder /= 10;
        }
    } //for
    reverse(res.begin(), res.end());
    return res;
}

int ModuloOperator(string base, int modb)
{
    int res = 0;
    for (int i = 0; i < base.length(); ++i)
        res = (res * 10 + (base[i] - '0')) % modb;

    return res;
}

int last_digit(list<int> array)
{

    if (array.empty()) return 1;
    else if (array.size() == 2 && !array.front() && !array.back()) return 1;
    else if (array.size() > 2 && !array.front()) return 0;

    reverse(array.begin(), array.end());
    string LD("1");

    for (auto v : array)
    {
        if (LD == "0") LD = "1";
        else if (LD == "1") LD = to_string(v);
        else
        {
            int modv = ModuloOperator(LD, 4) + 4;
            LD = PowerOperator(v, modv);
        }
    }

    return (LD.back() - '0');
}