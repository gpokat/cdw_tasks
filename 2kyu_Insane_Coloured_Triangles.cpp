#include <string>
using namespace std;

//This kata more about math instead of programming techniques.
//Anyway, the solution is sum of series which based on R,G,B=0,1,2.,
//and binominal coefficien (n,k).
//The main rule is look like a triangle  a b  => c=(2*(a+b))%3
//                                        c
//The full mathematics theory of the task here:
//https://stackoverflow.com/questions/53585022/three-colors-triangles
//Finially when (n-1)%3=0 => any triangle cutes to the small wich consist of
//first a ([0]), last b([n-1]) and result c.
//Other cases reduced used to nearest power of 3.

//powers of 3
const int pot[12] = {1, 3, 9, 27, 81, 243, 729, 2187, 6561, 19683, 59049, 177147};

//cast to ASCII then return custom int
int ctoi(char c)
{
    switch (static_cast<int>(c))
    {
    case 82: return 0;
    case 71: return 1;
    case 66: return 2;
    }
    return -1;
}

//return character based on custom int
char itoc(int i)
{
    switch (i)
    {
    case 0: return 'R';
    case 1: return 'G';
    case 2: return 'B';
    }
    return 'A';
}

char triangle(const string &row)
{
    //clock_t tStart = clock();

    string res = row;
    string line = row;

    while (line.size() > 1)
    {
        res.clear();
        int n = line.length() - 1;
        int np = 1;
        for (int p = 0; p < 11; ++p)
        {
            if (n >= pot[p]) np = pot[p];
            else break;
        }

        int sz = (n - np + 1);
        for (int pos = 0; pos < sz; ++pos)
            res.push_back(itoc((2 * (ctoi(line[pos]) + ctoi(line[n - pos]))) % 3));
        line = res;
    }

    //printf("Time taken total: %.2fms\n", (double)(clock() - tStart) / (CLOCKS_PER_SEC / 1000));

    return res[0];
}