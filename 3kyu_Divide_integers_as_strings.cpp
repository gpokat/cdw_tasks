#include <iostream>
#include <vector>
#include <string>

using namespace std;

//This kata about dividing huge integers problem storing as string.
//Using school mathematics to solve the problem i.e. long substruction,
//lond division and long multiplying.

bool BigLess(const string &a, const string &b)
{

    int n1 = a.length(), n2 = b.length();

    if (n1 < n2)
        return true;
    else if (n2 > n1 || n1 > n2)
        return false;

    for (int i = 0; i < n1; i++)
    {
        if (a[i] < b[i])
            return true;
        else if (a[i] > b[i])
            return false;
    }
    return false;
}

bool BigLessEqual(const string &a, const string &b)
{

    int n1 = a.length(), n2 = b.length();

    if (n1 < n2)
        return true;
    else if (n2 > n1 || n1 > n2)
        return false;

    for (int i = 0; i < n1; i++)
    {
        if (a[i] < b[i])
            return true;
        else if (a[i] > b[i])
            return false;
    }
    return true;
}

string BigMinus(const string &a, const string &b)
{

    string res;

    int n1 = a.length(), n2 = b.length();
    int dist = n1 - n2;

    int reminder = 0;

    for (int i = n2 - 1; i >= 0; i--)
    {
        // Do school mathematics
        int sub = (a[i + dist] - '0') - (b[i] - '0') - reminder;
        if (sub < 0)
        {
            sub += 10;
            reminder = 1;
        }
        else
            reminder = 0;

        res.push_back(sub + '0');
    }

    for (int i = dist - 1; i >= 0; i--)
    {
        if (a[i] == '0' && reminder)
        {
            res.push_back('9');
            continue;
        }
        int sub = ((a[i] - '0') - reminder);
        if (i > 0 || sub > 0)
            res.push_back(sub + '0');
        reminder = 0;
    }

    while (!(res.back() - '0') && res.length() > 1)
        res.pop_back();

    reverse(res.begin(), res.end());

    return res;
}

pair<int, string> FindNearestMultiplier(const string &a, string b)
{

    if (a.length() == 1 && (a[0] - '0') == 0)
        return make_pair(0, "0");
    else if (BigLess(a, b))
        return make_pair(0, a);

    string prev_res = b;
    reverse(b.begin(), b.end());
    for (int i = 2; i <= 10; ++i)
    {
        string res;
        int reminder = 0;
        int indx = 0;

        while (b[indx])
        {
            int mult = (b[indx++] - '0') * i + reminder;
            res.push_back(mult % 10 + '0');
            reminder = mult / 10;
        }

        while (reminder)
        {
            res.push_back(reminder % 10 + '0');
            reminder /= 10;
        }

        reverse(res.begin(), res.end());

        if (BigLess(a, res))
            return make_pair(i - 1, BigMinus(a, prev_res));
        else if (BigLessEqual(a, res))
            return make_pair(i, BigMinus(a, res));

        prev_res = res;
    } //for

    return pair<int, string>();
}

vector<string> divide_strings(string a, string b)
{

    if ((a[0] - '0') == 0)
        return vector<string>{"0", "0"};
    else if (BigLess(a, b))
        return vector<string>{"0", a};

    // if (a.length() <= 18) return vector<string> {to_string(stoll(a, nullptr, 10) / stoll(b, nullptr, 10)),
    //  to_string(stoll(a, nullptr, 10) % stoll(b, nullptr, 10))}; //def way with digit limit

    string res;
    string remainder;

    int indx = 0;
    string temp;
    temp.push_back(a[indx]);

    while (BigLess(temp, b))
        temp.push_back(a[++indx]);

    while (a.size() > indx)
    {
        auto div = FindNearestMultiplier(temp, b);
        res += div.first + '0';

        temp = div.second;
        temp.push_back(a[++indx]);

        reverse(temp.begin(), temp.end());

        while (!(temp.back() - '0') && temp.length() > 1)
            temp.pop_back();

        reverse(temp.begin(), temp.end());

        if (a.size() == indx)
            remainder = div.second;
    }
    return vector<string>{res, remainder};
}