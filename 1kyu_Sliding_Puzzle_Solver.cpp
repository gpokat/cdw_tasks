#include <vector>
#include <algorithm>
#include <array>
#include <map>
#include <iostream>
#include <unordered_map>
#include <random>

//This kata about classic combinatorial puzzles like 15-puzzles,
//but with hard-time constraints and large-scale extension (10x10).
//With this constraints classics algorithms like BFA, IDA* or A* doesn't work properly in general.
//Finally it's should pass 400 random test + 12 fixed for 12 second.
//The approach wich was applied is the algorithm athoured by GuiPing Wang & Ren Li
//http://dx.doi.org/10.1080/0952813X.2016.1259270

using namespace std;

std::random_device rd;                        // only used once to initialise (seed) engine
std::mt19937 rng(rd());                       // random-number engine used (Mersenne-Twister in this case)
std::uniform_int_distribution<int> uni(1, 4); // guaranteed unbiased

//look-up table for factorials from 0 to 10
const array<size_t, 11> factorials = {1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880, 3628800};

//State Transition Table
struct STTNode
{
    STTNode *pseq;              //link to previous node
    vector<vector<int>> puzzle; //corresponding state

    //ctor
    STTNode(vector<vector<int>> puzzle_, STTNode *pseq_ = nullptr) : puzzle(puzzle_),
                                                                     pseq(pseq_)
    {
    }

    //dtor
    ~STTNode()
    {
    }

    //overload compare operator
    bool operator==(const STTNode &node_) const noexcept
    {
        return (puzzle == node_.puzzle);
    }
};

//flatten input 2D vector
vector<int> ExpandPuzzle(const vector<vector<int>> &puzzle)
{
    vector<int> flatten_puzzle;

    for (auto i : puzzle)
        for (auto tile : i)
            flatten_puzzle.emplace_back(tile);

    return flatten_puzzle;
}

//calculate inversion count of each tile
vector<int> GetInversions(const vector<vector<int>> &puzzle, bool incz = false)
{
    vector<int> flatten_puzzle = ExpandPuzzle(puzzle);
    int sz = flatten_puzzle.size();

    vector<int> invs(sz, 0);

    for (int i = 0; i < sz - 1; i++)
        for (int j = i + 1; j < sz; j++)
        {
            if (!incz)
            {
                if (flatten_puzzle[i] > 0 && flatten_puzzle[j] > 0 && flatten_puzzle[i] > flatten_puzzle[j])
                    invs[i]++;
            }
            else
            {
                if (flatten_puzzle[i] > flatten_puzzle[j])
                    invs[i]++;
            }
        }
    return invs;
}

//specialization of hash function
namespace std
{
template <>
struct hash<STTNode>
{
    size_t operator()(const STTNode &node) const noexcept
    {
        size_t hash_value = 1;

        vector<int> invs = GetInversions(node.puzzle, true);

        vector<int> flatten_puzzle = ExpandPuzzle(node.puzzle);
        int sz = flatten_puzzle.size();

        for (int i = sz - 1; i >= 0; --i)
            hash_value += (factorials[i] * invs[sz - i - 1]);

        return hash_value;
    }
};
} // namespace std

//hash maps for STT
unordered_map<STTNode, STTNode *> STT3x2;
unordered_map<STTNode, STTNode *> STT2x3;

//solvability judgment
bool IsSolvable(const vector<vector<int>> &puzzle)
{
    int m = puzzle.size();    //rows
    int n = puzzle[0].size(); //cols

    vector<int> invs = GetInversions(puzzle);

    int invs_sum = accumulate(invs.begin(), invs.end(), 0); //total inversions of puzzle
    int bi = 0;

    for (auto vrow : puzzle)
    {
        if (find(vrow.begin(), vrow.end(), 0) != vrow.end())
            break;
        ++bi;
    }

    if (n % 2 && !(invs_sum % 2))
        return true;
    else if (!(n % 2) && ((bool)((m - bi) % 2) == !(invs_sum % 2)))
        return true;
    else
        return false;

    return true;
}

//moving method
bool Moving(int &blank_i, int &blank_j, int direction, int size_m, int size_n)
{

    switch (direction)
    {
    case 1: //upward
        blank_i--;
        if (blank_i < 0)
        {
            blank_i++;
            return false;
        }
        break;

    case 2: //leftward
        blank_j--;
        if (blank_j < 0)
        {
            blank_j++;
            return false;
        }
        break;

    case 3: //downward
        blank_i++;
        if (blank_i > size_m - 1)
        {
            blank_i--;
            return false;
        }
        break;

    case 4: //rightward
        blank_j++;
        if (blank_j > size_n - 1)
        {
            blank_j--;
            return false;
        }
        break;
    }
    return true;
}

STTNode *root3x2;
STTNode *root2x3;
//construct nodes of hash maps and filling State Transition Table
void ConstructSTT(unordered_map<STTNode, STTNode *> &STT, bool STT3x2 = true)
{
    vector<vector<int>> init_state;
    int blank_i, blank_j;
    int i, j;
    int szm, szn;

    if (STT3x2)
    {
        init_state = {{1, 2}, {3, 4}, {5, 0}};
        root3x2 = new STTNode(init_state, root3x2);
        STT.insert(make_pair(*root3x2, root3x2));
        i = blank_i = 2;
        j = blank_j = 1;
        szm = 3;
        szn = 2;
    }
    else
    {
        init_state = {{1, 2, 3}, {4, 5, 0}};
        root2x3 = new STTNode(init_state, root2x3);
        STT.insert(make_pair(*root2x3, root2x3));
        i = blank_i = 1;
        j = blank_j = 2;
        szm = 2;
        szn = 3;
    }

    do
    {
        if (Moving(i, j, uni(rng), szm, szn)) //stochastic moving of blank tile (it's simple and ugly =) )
        {
            //actually, the stochastic filling is much slower than determininig, but more simplify.
            //In this case it's about 200 msec. without chance to compilling as constexpr.
            //Btw, the code could be re-engineering for filling as constexpr and it'll boost this realization up to 0.5 msec.
            auto prev_state_key = STTNode(init_state);
            swap(init_state[blank_i][blank_j], init_state[i][j]);

            blank_i = i;
            blank_j = j;

            if (!STT.count(STTNode(init_state)) && IsSolvable(init_state))
            {
                auto prev_state_link = (*(STT.find(prev_state_key))).second;
                STT.insert(make_pair(STTNode(init_state), new STTNode(init_state, prev_state_link)));
            }
        }
    } while (STT.size() != 360);
}

//find position of tile in the puzzle
pair<int, int> FindTile(vector<vector<int>> &puzzle, int tile)
{
    int i, j;

    for (int i = 0; i < puzzle.size(); ++i)
        for (int j = 0; j < puzzle[0].size(); ++j)
            if (puzzle[i][j] == tile)
                return make_pair(i + 1, j + 1);

    return pair<int, int>();
}

vector<pair<int, int>> forbidden_pos;      //already in-place tile
vector<pair<int, int>> steps;              //already made steps
vector<pair<float, pair<int, int>>> seeds; //seeds for the next step
vector<int> MovingBlankHistory;            //history of blank tile mooving
//moving tile along the shortest path to it target position
void SolveTile(vector<vector<int>> &puzzle, int m, int n, int tile, int M, int N)
{
    //until tile not in-place
    while (puzzle[m - 1][n - 1] != tile)
    {

        auto blank_pos = FindTile(puzzle, 0);
        auto tile_pos = FindTile(puzzle, tile);

        //tile-target distance
        int ttr = tile_pos.first - m;  //row
        int ttc = tile_pos.second - n; //col

        //new blank position
        int bpr = -999, bpc = -999;

        if (ttc != 0)
        {
            bpr = tile_pos.first;
            if (ttc > 0)
            { //b-p
                bpc = tile_pos.second - 1;
            }
            else if (ttc < 0)
            { //p-b
                bpc = tile_pos.second + 1;
            }
        }
        else
        {
            bpc = tile_pos.second;
            if (ttr > 0) //vertical b-p
            {
                bpr = tile_pos.first - 1;
            }
            else if (ttr < 0)
            { //vertical p-b
                bpr = tile_pos.first + 1;
            }
        }

        //target position of blank tile
        bpr--;
        bpc--;

        //greedy strategy with minimize path
        while (pair<int, int>(bpr + 1, bpc + 1) != blank_pos) //until reached blank target pos
        {
            int ib_init = blank_pos.first - 1, jb_init = blank_pos.second - 1;
            steps.emplace_back(ib_init, jb_init); //blank pos

            for (int i = 4; i >= 1; --i) //heuristic --- prefer right/down move.
            {
                int ib = blank_pos.first - 1, jb = blank_pos.second - 1;

                if (Moving(ib, jb, i, M, N) &&                                                                  //if move can be done
                    find(steps.begin(), steps.end(), make_pair(ib, jb)) == steps.end() &&                       //if it step didn't use before
                    (pair<int, int>(ib + 1, jb + 1) != tile_pos) &&                                             //move mustn't change targte tile
                    find(forbidden_pos.begin(), forbidden_pos.end(), make_pair(ib, jb)) == forbidden_pos.end()) //move mustn't change already in-place tiles
                {
                    float dist = sqrt((bpr - ib) * (bpr - ib) + (bpc - jb) * (bpc - jb)); //estimate distance
                    seeds.emplace_back(dist, make_pair(ib, jb));
                }
            }

            //get step with min distance prefer down/right move
            pair<int, int> next_pos = make_pair(-999, -999);
            int min_dist = 999;
            for (auto v : seeds)
            {
                if (v.first < min_dist)
                {
                    next_pos = v.second;
                    min_dist = v.first;
                }
            }

            steps.emplace_back(next_pos.first, next_pos.second);
            MovingBlankHistory.emplace_back(puzzle[next_pos.first][next_pos.second]); //store blank tile move
            swap(puzzle[ib_init][jb_init], puzzle[next_pos.first][next_pos.second]);  //move blank tile

            seeds.clear();
            blank_pos = FindTile(puzzle, 0);
        }
        //exchange blank and target tile
        MovingBlankHistory.emplace_back(puzzle[tile_pos.first - 1][tile_pos.second - 1]); //store blank tile move
        swap(puzzle[bpr][bpc], puzzle[tile_pos.first - 1][tile_pos.second - 1]);

        steps.clear();
    }

    if (m == M)
        n--;
    if (n == N)
        m--;

    //exit(0);
    forbidden_pos.emplace_back(m - 1, n - 1);
}

//set blank tile to the right-down position of the current rectangle
void SetBlankTile(vector<vector<int>> &puzzle, int m, int n, vector<pair<int, int>> &mapped_pos, bool corner = true)
{
    const int M = puzzle.size();
    const int N = puzzle[0].size();
    auto blank_pos = FindTile(puzzle, 0);
    //until tile not in-place
    if (corner)
    {
        if ((blank_pos.second - 1) - (n - 1) == 0)
            return;
        else if ((blank_pos.second - 1) - n == 0)
            return;
    }
    else
    {
        if ((blank_pos.second - 1) - (n - 1) == 0)
            return;
        else if ((blank_pos.second - 1) - n == 0)
            return;
        else if ((blank_pos.second - 1) - (n - 2) == 0)
            return;
    }
    while (puzzle[m][n] != 0)
    {
        // auto blank_pos = FindTile(puzzle, 0);
        int ib_init = blank_pos.first - 1, jb_init = blank_pos.second - 1;
        steps.emplace_back(ib_init, jb_init); //blank pos
        for (int i = 4; i >= 1; --i)          //heuristic --- prefer right/down move.
        {
            int ib = blank_pos.first - 1, jb = blank_pos.second - 1;

            if (Moving(ib, jb, i, M, N) &&                                                                    //if move can be done
                find(steps.begin(), steps.end(), make_pair(ib, jb)) == steps.end() &&                         //if it step didn't use before
                find(forbidden_pos.begin(), forbidden_pos.end(), make_pair(ib, jb)) == forbidden_pos.end() && //move mustn't change already in-place tiles
                find(mapped_pos.begin(), mapped_pos.end(), make_pair(ib, jb)) == mapped_pos.end())            //move mustn't change already mapped tiles
            {
                float dist = sqrt((m - ib) * (m - ib) + (n - jb) * (n - jb)); //estimate distance
                seeds.emplace_back(dist, make_pair(ib, jb));
            }
        }

        //get step with min distance prefer down/right move
        pair<int, int> next_pos = make_pair(-999, -999);
        int min_dist = 999;
        for (auto v : seeds)
        {
            if (v.first < min_dist)
            {
                next_pos = v.second;
                min_dist = v.first;
            }
        }

        steps.emplace_back(next_pos.first, next_pos.second);
        MovingBlankHistory.emplace_back(puzzle[next_pos.first][next_pos.second]); //store blank tile move
        swap(puzzle[ib_init][jb_init], puzzle[next_pos.first][next_pos.second]);  //move blank tile

        seeds.clear();
        blank_pos = FindTile(puzzle, 0);
    }

    steps.clear();
}

map<int, int> mapping;
//forming sub-puzzle and mapping for it
vector<vector<int>> ApplyMapping(vector<vector<int>> &puzzle, int beg_m, int beg_n, bool corner, bool dir = false)
{

    vector<vector<int>> sub_puzzle;
    vector<int> residual;

    int size_m = 2, size_n = 3;

    // top-right corner 3x2 mapping
    if (corner)
    {
        sub_puzzle = {{0, 0}, {0, 0}, {0, 0}};
        sub_puzzle[0][0] = 1;
        sub_puzzle[1][1] = 2;
        residual = {3, 4, 5};
        swap(size_m, size_n);
    }
    else // last two rows 2x3 mapping
    {
        sub_puzzle = {{0, 0, 0}, {0, 0, 0}};
        sub_puzzle[0][0] = 1;
        sub_puzzle[1][1] = 4;
        residual = {2, 3, 5};
    }

    if (!dir) //mapping for residual tile (not direct)
    {
        do
        {
            next_permutation(residual.begin(), residual.end());
            int i_res = 0;
            for (int i = 0; i < size_m; ++i)
            {
                for (int j = 0; j < size_n; ++j)
                {
                    int m = i + beg_m;
                    int n = j + beg_n;
                    int tile = puzzle[m][n];

                    if (tile == 0 || (i == 0 && j == 0) || (i == 1 && j == 1)) //skip already mapped tiles
                        continue;

                    sub_puzzle[i][j] = residual[i_res++];

                    //cout<<tile<< m<<", "<<n<<", "<<beg_n <<endl;
                }
            }
        } while (!IsSolvable(sub_puzzle));

        for (int i = 0; i < size_m; ++i)
        {
            for (int j = 0; j < size_n; ++j)
            {
                int m = i + beg_m;
                int n = j + beg_n;
                int tile = puzzle[m][n];

                mapping.insert(make_pair(sub_puzzle[i][j], tile));
            }
        }
    }
    else //direct mapping (for last 2x3 puzzle)
    {
        vector<pair<int, pair<int, int>>> prep;
        prep.reserve(6);

        const int M = puzzle.size();

        for (int i = 0; i < size_m; ++i)
        {
            for (int j = 0; j < size_n; ++j)
            {
                int tile = puzzle[i + beg_m][j + beg_n];
                prep.emplace_back(make_pair(tile, FindTile(puzzle, tile)));
            }
        }

        sort(prep.begin(), prep.end(), [](const pair<int, pair<int, int>> &v1, const pair<int, pair<int, int>> &v2) { return v1.first < v2.first; });

        do
        {
            int order = 0;
            for (auto t : prep)
            {
                mapping.insert(make_pair(order, t.first));
                sub_puzzle[t.second.first - (M - 2) - 1][t.second.second - beg_n - 1] = order++;
            }

            swap(prep[3], prep[5]); //heuristic -- only one swap enough to change the solvability

        } while (!IsSolvable(sub_puzzle));
    }
    return sub_puzzle;
}

void FindSolution(vector<vector<int>> &puzzle)
{

    const int M = puzzle.size();    //rows
    const int N = puzzle[0].size(); //cols

    forbidden_pos.reserve(M * N);
    steps.reserve(M * N);
    seeds.reserve(4);
    
    MovingBlankHistory.clear();
    MovingBlankHistory.reserve(M * N * 10);

    vector<pair<int, int>> MpdPos_;

    vector<vector<int>> sub_puzzle;

    for (int m = 1; m <= M - 2; ++m)
    {
        for (int n = 1; n <= N; ++n) //include the last tile in the row
        {

            int tile = n + N * (m - 1);
            if (n != N)
                SolveTile(puzzle, m, n, tile, M, N); // for k
            else
                SolveTile(puzzle, m + 1, n, tile, M, N); //for k+1
        }                                                //n

        MpdPos_ = {pair<int, int>(m - 1, N - 2), pair<int, int>(m, N - 1)};
        SetBlankTile(puzzle, m + 1, N - 1, MpdPos_);

        sub_puzzle = ApplyMapping(puzzle, m - 1, N - 2, true);

        //solve last and second-last tiles in the row using by STT3x2
        auto begin = STT3x2[STTNode(sub_puzzle)];
        int target_1 = N - 1 + N * (m - 1); //second-last target tile => k
        int target_2 = N + N * (m - 1);     //last target tile => k+1

        while (mapping[begin->puzzle[0][0]] != target_1 || mapping[begin->puzzle[0][1]] != target_2)
        {
            auto blank_tile_pos = FindTile(begin->puzzle, 0);
            begin = begin->pseq;
            MovingBlankHistory.emplace_back(mapping[begin->puzzle[blank_tile_pos.first - 1][blank_tile_pos.second - 1]]); //store blank tile move
        }

        for (int i = 0; i < begin->puzzle.size(); ++i)
        {
            for (int j = 0; j < begin->puzzle[0].size(); ++j)
                puzzle[i + (m - 1)][j + (N - 2)] = mapping[begin->puzzle[i][j]];
        }
        mapping.clear();
    } //m

    //for the last two rows
    for (int n = 1; n <= N - 3; ++n)
    {
        for (int m = M - 1; m <= M; ++m)
        {
            int tile = n + N * (m - 1);

            if (m != M)
                SolveTile(puzzle, m, n, tile, M, N); //for s
            else
                SolveTile(puzzle, m, n + 1, tile, M, N); //for s+n tile
        }                                                //m

        MpdPos_ = {pair<int, int>(M - 2, n - 1), pair<int, int>(M - 1, n)};
        SetBlankTile(puzzle, M - 1, n + 1, MpdPos_, false);

        sub_puzzle = ApplyMapping(puzzle, M - 2, n - 1, false);

        //solve last and second-last tiles in the row using by STT3x2
        auto begin = STT2x3[STTNode(sub_puzzle)];
        int target_1 = n + N * (M - 2); //first target tile => s
        int target_2 = n + N * (M - 1); //4th target tile => s+n

        while (mapping[begin->puzzle[0][0]] != target_1 || mapping[begin->puzzle[1][0]] != target_2)
        {
            auto blank_tile_pos = FindTile(begin->puzzle, 0);
            begin = begin->pseq;
            MovingBlankHistory.emplace_back(mapping[begin->puzzle[blank_tile_pos.first - 1][blank_tile_pos.second - 1]]); //store blank tile move
        }
        for (int i = 0; i < begin->puzzle.size(); ++i)
        {
            for (int j = 0; j < begin->puzzle[0].size(); ++j)
                puzzle[i + (M - 1) - 1][j + (n - 1)] = mapping[begin->puzzle[i][j]];
        }
        mapping.clear();
    } //n

    //solve last-down 2x3 puzzle
    sub_puzzle = ApplyMapping(puzzle, M - 2, N - 3, false, true);

    auto begin = STT2x3[STTNode(sub_puzzle)];
    while (begin != root2x3)
    {
        auto blank_tile_pos = FindTile(begin->puzzle, 0);
        begin = begin->pseq;
        MovingBlankHistory.emplace_back(mapping[begin->puzzle[blank_tile_pos.first - 1][blank_tile_pos.second - 1]]); //store blank tile move
    }

    for (int i = 0; i < begin->puzzle.size(); ++i)
    {
        for (int j = 0; j < begin->puzzle[0].size(); ++j)
            puzzle[i + (M - 1) - 1][j + (N - 2) - 1] = mapping[begin->puzzle[i][j]];
    }

    
    mapping.clear();
    sub_puzzle.clear();
    forbidden_pos.clear();
}

bool is_first_lunch = true;
std::vector<int> slide_puzzle(const std::vector<std::vector<int>> &arr)
{  
    if (is_first_lunch)
    {
        ConstructSTT(STT3x2, true);
        ConstructSTT(STT2x3, false);
        is_first_lunch = false;
    }
    
    vector<vector<int>> puzzle = arr;
        
    if (!IsSolvable(puzzle)){
    mapping.clear();
    forbidden_pos.clear();
    MovingBlankHistory.clear();
    return {0};
    }

FindSolution(puzzle);
    return MovingBlankHistory;
}