#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cmath>
#include <time.h>
#include <array>
#include <algorithm>

using namespace std;
//This kata about fast calculation
//of Big Integer Square Root problem.
//Attempt to use binary form of pencil&paper method(digit-by-digit)
//with BigBinaryArithmetic which implement as vector of bool
//https://en.wikipedia.org/wiki/Methods_of_computing_square_roots

typedef vector<bool> vbin;
//look-up-table for powers of 2
array<string, 500> power_of_two;
//make power of 2 table i.e. multiply 2 n times byself
void MakePowerOf2Table()
{

    power_of_two[0] = "1";
    power_of_two[1] = "2";

    for (int i = 2; i <= 499; ++i)
    {
        string power;
        int reminder = 0;
        string prev = power_of_two[i - 1];
        int indx = 0;

        while (prev[indx])
        {
            int mult = (prev[indx++] - '0') * 2 + reminder;
            power.push_back(mult % 10 + '0');
            reminder = mult / 10;
        } //while 1

        while (reminder)
        {
            power.push_back(reminder % 10 + '0');
            reminder /= 10;
        }

        power_of_two[i] = power;
    } //for

    for (int i = 0; i < 500; i++)
    {
        reverse(power_of_two[i].begin(), power_of_two[i].end());
       // cout<<power_of_two[i]<<endl;
    }

} //void


//BigBinaryArithmetic
struct BBA
{
    vbin v;
    //def ctor
    BBA() : v({}) {}

    //ctor for vector
    BBA(vbin &v_) : v(v_)
    {
    }

    //ctor for string
    BBA(string& str_)
    {
        to_binary(str_);
    }

    //def dtor
    ~BBA()
    {
        v.clear();
    }

    //assignment
    BBA &operator=(const BBA &v_)
    {
        v = v_.v;
        return *this;
    }

    //comparison
    friend bool operator>(const BBA &lhs_, const BBA &rhs_)
    {
        size_t n1 = lhs_.v.size();
        size_t n2 = rhs_.v.size();

        if (n1 > n2)
            return true;
        else if (n1 < n2)
            return false;

        for (size_t i = 0; i < n1; ++i)
        {
            if (lhs_.v[i] > rhs_.v[i])
                return true;
            else if (lhs_.v[i] < rhs_.v[i])
                return false;
        }
        return false;
    }

    friend bool operator>=(const BBA &lhs_, const BBA &rhs_)
    {
        size_t n1 = lhs_.v.size();
        size_t n2 = rhs_.v.size();

        if (n1 > n2)
            return true;
        else if (n1 < n2)
            return false;

        for (size_t i = 0; i < n1; ++i)
        {
            if (lhs_.v[i] > rhs_.v[i])
                return true;
            else if (lhs_.v[i] < rhs_.v[i])
                return false;
        }

        return true;
    }

    friend bool operator<(const BBA &lhs_, const BBA &rhs_)
    {
        size_t n1 = lhs_.v.size();
        size_t n2 = rhs_.v.size();

        if (n1 < n2)
            return true;
        else if (n1 > n2)
            return false;

        for (size_t i = 0; i < n1; ++i)
        {
            if (lhs_.v[i] < rhs_.v[i])
                return true;
            else if (lhs_.v[i] > rhs_.v[i])
                return false;
        }
        return false;
    }

    //long addition
    friend BBA operator+(const BBA &lhs_, const BBA &rhs_)
    {
        vbin res;
        res.reserve(100);

        int sum = 0;
        int n1 = lhs_.v.size() - 1;
        int n2 = rhs_.v.size() - 1;
        while (n1 >= 0 || n2 >= 0 || sum)
        {
            sum += ((n1 >= 0) ? lhs_.v[n1] : 0);
            sum += ((n2 >= 0) ? rhs_.v[n2] : 0);

            res.emplace_back(sum % 2);

            sum /= 2;

            --n1;
            --n2;
        }

        size_t i = res.size() - 1;
        while (!res[i--])
            res.pop_back();

        reverse(res.begin(), res.end());
        return BBA(res);
    }

    //long substruction
    friend BBA operator-(const BBA &lhs_, const BBA &rhs_)
    {
        vbin res;
        res.reserve(100);
        BBA lhs = lhs_;

        int sub = 0;
        int n1 = lhs_.v.size() - 1;
        int n2 = rhs_.v.size() - 1;
        while (n1 >= 0)
        {
            int a = lhs.v[n1];
            int b = (n2 >= 0) ? rhs_.v[n2] : 0;

            if (!a && b && n1)
            {
                int borrow = 1;
                while (!lhs.v[n1 - borrow])
                {
                    lhs.v[n1 - borrow] = 1;
                    borrow++;
                }
                a = 2;
                lhs.v[n1 - borrow] = 0;
            }

            sub = a - b;

            res.emplace_back(sub);

            --n1;
            --n2;
        }

        size_t i = res.size() - 1;
        while (!res[i--] && res.size() > 1)
            res.pop_back();

        reverse(res.begin(), res.end());

        return BBA(res);
    }

    //right bitshift i.e. dividing by 2*s
    friend BBA operator>>(const BBA &a, const unsigned int &s)
    {
        vbin res = a.v;
        if (a.v.size() <= s)
            return BBA();
        else
        {
            for (int i = 0; i < s; i++)
                res.pop_back();
        }

        return BBA(res);
    }
    //left bitshift i.e. multiply by 2*s
    friend BBA operator<<(const BBA &a, const unsigned int &s)
    {
        vbin res = a.v;
        for (int i = 0; i < s; i++)
            res.emplace_back(0);

        return BBA(res);
    }

    void GetDecimalForm(string &decimal_isqrt)
    {
        to_decimal();
        decimal_isqrt = fdecimal_isqrt;
        
        return;
    }

private:
    //recursive convert decimal string value to binary format i.e. long division by 2
    void to_binary(string &num)
    {
        if (num.length() == 1 && (num[0] - '0') == 0)
        {
            reverse(v.begin(), v.end());
            return;
        }

        string bin;
        int indx = 0;
        int temp = num[indx] - '0';
        while (temp < 2 && num.size() > 1)
            temp = temp * 10 + (num[++indx] - '0');

        while (num.size() > indx)
        {
            bin += (temp / 2) + '0';
            temp = (temp % 2) * 10 + num[++indx] - '0';
        }

        v.emplace_back(num.back() % 2);
        to_binary(bin);

        return;
    }

    //convert binary string value to decimal format i.e. long sum of power of 2
    void to_decimal()
    {
      
       if (!power_of_two[0].length())
        MakePowerOf2Table();
                           
       string a;
        int n = v.size();
        fdecimal_isqrt = power_of_two[n - 1];
        reverse(fdecimal_isqrt.begin(), fdecimal_isqrt.end());

        for (int indx = 1; indx < n; ++indx)
        {
            if (v[indx])
                a = power_of_two[(n - 1) - indx];
            else
                continue;

            string str = "";
            str.reserve(100);

            // Calculate length of strings
            int n1 = a.length(), n2 = fdecimal_isqrt.length();

            //Reverse string
            reverse(a.begin(), a.end());

            int reminder = 0;
            for (int i = 0; i < n1; ++i)
            {
                //long sum
                int sum = ((a[i] - '0') + (fdecimal_isqrt[i] - '0') + reminder);
                str.push_back(sum % 10 + '0');

                // Calculate remainder for next step
                reminder = sum / 10;
            }

            // Add remaining digits of larger number
            for (int i = n1; i < n2; ++i)
            {
                int sum = ((fdecimal_isqrt[i] - '0') + reminder);
                str.push_back(sum % 10 + '0');
                reminder = sum / 10;
            }

            // Add remainder
            if (reminder)
                str.push_back(reminder + '0');

            fdecimal_isqrt = str;
        }
        // reverse result string
        reverse(fdecimal_isqrt.begin(), fdecimal_isqrt.end());

        return;
    }

    string fdecimal_isqrt;
};


string integer_square_root(string n)
{

    //default C++ way.
    if (n.length() <= 18)
        return to_string((unsigned long long)sqrt(stoll(n))); //digit limit
            
       //     clock_t tStart2 = clock();
  
  //2^304 as binary
    vbin vinit(341, 0);
    vinit[0] = 1;   

    BBA root;
    BBA num(n);  
    BBA bit(vinit); //initial guess. Should be a power of 2
    
    while (bit > num)
        bit = bit >> 2; //find a nearest and smaller power of 2 to num

    //binary pencil-paper method of integer square root calculation digit-by-digit
    while (bit.v.size() > 0 && bit.v[0] != 0)
    {
        BBA sum = root + bit;
        if (num >= sum)
        {
            num = num - sum;
            root = (root >> 1) + bit;
        }
        else
            root = root >> 1;

        bit = bit >> 2;
    }
    
    string res;
    root.GetDecimalForm(res);
   // cout << "res is: " << res << endl;

   //  printf("Time taken total: %.2fms\n", (double)(clock() - tStart2) / (CLOCKS_PER_SEC / 1000)); //+ 2.7);
 
 
    return res;
}

//Code optimization
//lvl 1 -- replace push_back to emplace_back                <--- DONE
//lvl 2 -- using base 2 instead of base 10                  <--- DONE
//lvl 3 -- look-up-table for powers of 2                    <--- DONE
//lvl 4 -- using default way of sqrt if can                 <--- DONE
//lvl 5 -- implement fast binary integer square root method <--- DONE
//lvl 6 -- make look-up-table as constexpr                  <--- UNDONE