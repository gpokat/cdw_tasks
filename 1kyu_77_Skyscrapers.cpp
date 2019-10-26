#include <vector>
#include <algorithm>
#include <array>
#include <map>
#include <iostream>

#include <time.h>

using namespace std;

//This kata about exact cover problem -
//one of the NP-complete problems in computer science.
//Attempt to use Knuth AlgorithmX for find a solution.
//https://arxiv.org/pdf/cs/0011047.pdf

struct NodeXY
{ //node
    //right-left pointers to nodes
    NodeXY *right;
    NodeXY *left;

    //up-down pointers to nodes
    NodeXY *up;
    NodeXY *down;

    //extra pointer to "header" node
    NodeXY *header;

    //column additional information
    int count;
    array<int, 3> ID;

    //ctor (init field using nullptr and zeros)
    NodeXY() : count(0),
               ID({0, 0, 0}),
               right(nullptr),
               left(nullptr),
               up(nullptr),
               down(nullptr),
               header(nullptr)
    {
    }

    //def dtor
    ~NodeXY()
    {
    }
};

//target matrix
vector<vector<int>> target_matrix =
    {{0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0}};

vector<int> CLUES; //copy of clues

vector<NodeXY *> covered_nodes;                     //backtraced nodes
vector<NodeXY *> sub_covered_nodes;                 //decomposited nodes
vector<NodeXY *> covered_nodes_dcp;                 //covered nodes due to tabu
vector<vector<NodeXY *>> DLXMatrix;                 //343 + row-headers
bool tabu_list[7][7][7] = {};                       //tabu list
multimap<pair<int, int>, vector<int>> permutations; //lookup table
NodeXY *root = new NodeXY();                        //parent node in DLXMatrix
bool first_lunch = true;                            //begin flag

void SetClues(const vector<int> &clues)
{
    CLUES = clues;
}

//transposition matrix
void TransposeMatrix(vector<vector<int>> &target_matrix)
{
    //in-place matrix transposition for square matrix
    for (register int i = 0; i < 6; ++i)
        for (register int j = i + 1; j < 7; ++j)
            swap(target_matrix[i][j], target_matrix[j][i]);
}

void PrintMatrix()
{
    for (int i = 0; i < 7; ++i)
    {
        cout << "\n";
        for (int j = 0; j < 7; ++j)
            cout << " " << target_matrix[i][j];
    }
    cout << "\n";
    return;
}

//set target matrix to zero
void Reset()
{
    for (register int i = 0; i < 7; ++i)
    {
        target_matrix[i][0] = 0;
        target_matrix[i][1] = 0;
        target_matrix[i][2] = 0;
        target_matrix[i][3] = 0;
        target_matrix[i][4] = 0;
        target_matrix[i][5] = 0;
        target_matrix[i][6] = 0;
    }
}

//method takes clue and expand it to possible numbers
void DecompositeClues(int k, int extra_iter = 1)
{
    int value = (8 - k); // for k=1..7 value=7..1

    //expand clue
    for (int iter = 0; iter < 2; ++iter)
    {
        for (int i = 0; i < 7; ++i)
        {
            int forward_clue, backward_clue;

            switch (iter)
            {
            case 0:
                forward_clue = CLUES[27 - i] - k;
                backward_clue = CLUES[i + 7] - k;
                break;
            case 1:
                forward_clue = CLUES[i] - k;
                backward_clue = CLUES[20 - i] - k;
                break;
            } //switch

            if (forward_clue > 0)
                for (int j = 0; j < forward_clue; ++j)
                    if (target_matrix[i][j] > 7 || target_matrix[i][j] < 1)
                        target_matrix[i][j] = 9;

            if (backward_clue > 0)
                for (int j = 6; j > (6 - backward_clue); --j)
                    if (target_matrix[i][j] > 7 || target_matrix[i][j] < 1)
                        target_matrix[i][j] = 9;

        } //for i
        TransposeMatrix(target_matrix);
    } //for iter

    //set possible values
    for (int iter = 0; iter < 2 * extra_iter; ++iter)
    {
        int check_sum, vacant_pos;
        for (int i = 0; i < 7; ++i)
        {
            check_sum = 0;
            vacant_pos = -1;
            for (int j = 0; j < 7; ++j)
            {
                if (target_matrix[i][j] > 0)
                    ++check_sum;
                else if (target_matrix[i][j] == 0)
                    vacant_pos = j;
            }

            if (check_sum == 6 && vacant_pos > -1)
            {
                target_matrix[i][vacant_pos] = value;

                //update possible positions
                for (int ik = 0; ik < 7; ++ik)
                    for (int jk = 0; jk < 7; ++jk)
                        if (target_matrix[ik][jk] == value)
                        {
                            for (int jj = 0; jj < 7; ++jj)
                                if (target_matrix[ik][jj] > 7 || target_matrix[ik][jj] < 1)
                                    target_matrix[ik][jj] = 9; //row

                            for (int ii = 0; ii < 7; ++ii)
                                if (target_matrix[ii][jk] > 7 || target_matrix[ii][jk] < 1)
                                    target_matrix[ii][jk] = 9; //col
                        }                                      //checksum

            } //rows
        }
        TransposeMatrix(target_matrix);

    } //iter

    //PrintMatrix();
    for (int i = 0; i < 7; ++i)
        for (int j = 0; j < 7; ++j)
            if (target_matrix[i][j] == 9)
            {
                tabu_list[i][j][value - 1] = true;
                target_matrix[i][j] = 0;
            }
}

//calculate right and left clue for row/col
pair<int, int> GetClues(vector<int> &state)
{
    int forward = 0, backward = 0;
    int MaxFloorF = 0, MaxFloorB = 0;
    for (register int i = 0; i < 7; ++i)
    {
        if (state[i] > MaxFloorF)
        {
            MaxFloorF = state[i];
            ++forward;
        }
        else if (!state[i] && MaxFloorF != 7)
        {
            ++forward;
        }
        if (state[7 - (i + 1)] > MaxFloorB)
        {
            MaxFloorB = state[7 - (i + 1)];
            ++backward;
        }
        else if (!state[7 - (i + 1)] && MaxFloorB != 7)
        {
            ++backward;
        }
    }
    return make_pair(forward, backward);
}

//verificate specific row/col according to the clues
bool ValidState(int row, int col)
{
    bool res = true;
    int forward_clue, backward_clue;
    bool is_full_row;
    int i;
    for (register int iter = 0; iter < 2; ++iter)
    {
        if (!iter)
        {
            i = row;
            forward_clue = CLUES[27 - i];
            backward_clue = CLUES[i + 7];
        }
        else
        {
            i = col;
            forward_clue = CLUES[i];
            backward_clue = CLUES[20 - i];
            TransposeMatrix(target_matrix);
        }
        if (!forward_clue && !backward_clue)
            continue;

        is_full_row = true;
        for (register int j = 0; j < 7; ++j)
        {
            if (!target_matrix[i][j])
            {
                is_full_row = false;
                break;
            }
        }

        auto clues = GetClues(target_matrix[i]);

        if (is_full_row)
        {
            if (forward_clue && clues.first != forward_clue)
            {
                if (iter)
                    TransposeMatrix(target_matrix);
                return false;
            }
            else if (backward_clue && clues.second != backward_clue)
            {
                if (iter)
                    TransposeMatrix(target_matrix);
                return false;
            }
        }
        else
        {
            if (forward_clue && clues.first < forward_clue)
            {
                if (iter)
                    TransposeMatrix(target_matrix);
                return false;
            }
            else if (backward_clue && clues.second < backward_clue)
            {
                if (iter)
                    TransposeMatrix(target_matrix);
                return false;
            }
        }
    }
    TransposeMatrix(target_matrix);

    return res;
}

//verificate whole target matrix according to the clues
bool ValidState()
{
    bool res = true;
    for (register int iter = 0; iter < 2; ++iter)
    {
        for (register int i = 0; i < 7; ++i)
        {
            int forward_clue, backward_clue;
            switch (iter)
            {
            case 0:
                forward_clue = CLUES[27 - i];
                backward_clue = CLUES[i + 7];
                break;
            case 1:
                forward_clue = CLUES[i];
                backward_clue = CLUES[20 - i];
                break;
            } //switch

            if (forward_clue == 0 && backward_clue == 0)
                continue;

            auto clues = GetClues(target_matrix[i]);
            if ((forward_clue > 0 && clues.first != forward_clue) || (backward_clue > 0 && clues.second != backward_clue))
            {
                if (iter)
                    TransposeMatrix(target_matrix);
                return false;
            }
        }
        TransposeMatrix(target_matrix);
    }
    return res;
}

//calculate possible permutations and relevant clues (LUT)
void PossiblePermute()
{
    vector<int> one_seven = {1, 2, 3, 4, 5, 6, 7};

    do
    {
        auto clues = GetClues(one_seven);
        permutations.insert(make_pair(clues, one_seven));
    } while (next_permutation(one_seven.begin(), one_seven.end()));
}

void UpdateTabuList()
{
    bool pst[7][7][7] = {};
    bool used[7][7] = {};

    for (int row = 0; row < 7; ++row)
    {
        for (int iter = 0; iter < 2; ++iter)
        {
            int forward_clue, backward_clue;
            switch (iter)
            {
            case 0:
                forward_clue = CLUES[27 - row];
                backward_clue = CLUES[7 + row];

                break;
            case 1:
                forward_clue = CLUES[row];
                backward_clue = CLUES[20 - row];

                break;
            } //switch

            //heuristic assumption - take into account only completed clues
            if (forward_clue > 0 && backward_clue > 0)
            {
                for (auto it = permutations.equal_range(make_pair(forward_clue, backward_clue)).first; it != permutations.equal_range(make_pair(forward_clue, backward_clue)).second; ++it)
                {
                    if (equal(target_matrix[row].begin(), target_matrix[row].end(), it->second.begin(),
                              [](const int &a1, const int &a2) {if(a1>0) return a1==a2; else return true; }))
                    {
                        for (int j = 0; j < 7; ++j)
                        {
                            if (!iter)
                            {
                                if (!tabu_list[row][j][it->second[j] - 1])
                                {
                                    pst[row][j][it->second[j] - 1] = true;
                                    used[row][j] = true;
                                }
                            }
                            else if (!tabu_list[j][row][it->second[j] - 1])
                            {
                                pst[j][row][it->second[j] - 1] = true;
                                used[j][row] = true;
                            }
                        }
                    }
                }
            }
            TransposeMatrix(target_matrix);
        } //iter
    }     //row

    //update tabu list
    for (int i = 0; i < 7; ++i)
        for (int j = 0; j < 7; ++j)
            for (int k = 0; k < 7; ++k)
            {
                if (!pst[i][j][k] && used[i][j])
                    tabu_list[i][j][k] = true;
            }
}

//The moving method which return next position in DLXMatrix
int Moving(int LRUP, int pos)
{
    switch (LRUP)
    {
    case 0: //Left
        if (pos - 1 < 0)
            return 146;
        else
            return pos - 1;
    case 1: //Right
        if (pos + 1 > 146)
            return 0;
        else
            return pos + 1;
    case 2: //Up
        if (pos - 1 < 0)
            return (342 + 1);
        else
            return pos - 1;
    case 3: //Down
        if (pos + 1 > (342 + 1))
            return 0;
        else
            return pos + 1;
    default:
        return 0;
    } //switch
}

//fill "dancing links" sparse matrix
void FillDLXSPM()
{
    int MAX_ROW = 7 * 7 * 7;
    int MAX_COL = 7 * 7 * 3;

    //fill SPM matrix with 0s && 1s (in this case it's {i,j,k})
    int first_column_cell = 0, second_column_row = 0, third_column_col = 0;
    int column_shift = 7 * 7, row_count = 1;

    array<int, 3> DLXSPMatrix[MAX_ROW + 1][MAX_COL]; //+1 headers

    for (int i = 0; i < MAX_ROW + 1; ++i)
        for (int j = 0; j < MAX_COL; ++j)
            DLXSPMatrix[i][j] = {0, 0, 0};

    for (int i = 0; i < 7; ++i)
    {
        for (int j = 0; j < 7; ++j)
        {
            for (int k = 0; k < 7; ++k)
            {
                second_column_row = (k + 7 * i) + column_shift;
                third_column_col = (k + 7 * j) + column_shift * 2;

                //if(target_matrix[i][j]==0){
                DLXSPMatrix[row_count][first_column_cell] = {i, j, k + 1};
                DLXSPMatrix[row_count][second_column_row] = {i, j, k + 1};
                DLXSPMatrix[row_count][third_column_col] = {i, j, k + 1};

                //headers
                DLXSPMatrix[0][first_column_cell] = {0, 0, k + 1};
                DLXSPMatrix[0][second_column_row] = {0, 0, k + 1};
                DLXSPMatrix[0][third_column_col] = {0, 0, k + 1};
                //}
                row_count++;
            }                    //k
            first_column_cell++; //0-48
        }                        //j
    }                            //i

    for (register int i = 0; i < MAX_ROW + 1; ++i)
    {
        vector<NodeXY *> row;
        row.reserve(MAX_COL);
        for (register int j = 0; j < MAX_COL; ++j)
            row.emplace_back(new NodeXY());

        DLXMatrix.emplace_back(row);
    }

    for (int i = 0; i < MAX_ROW + 1; ++i)
    { //+1 header row
        for (int j = 0; j < MAX_COL; ++j)
        {
            if (i)
            {
                DLXMatrix[0][j]->count++;                //increment count for each nodes
                DLXMatrix[i][j]->ID = DLXSPMatrix[i][j]; //set ID
            }

            DLXMatrix[i][j]->header = DLXMatrix[0][j]; //link header

            //the best way to make circular list
            int a = i, b = j;
            do
            {
                b = Moving(0, b);
            } while (!DLXSPMatrix[a][b][2] && b != j);
            DLXMatrix[i][j]->left = DLXMatrix[i][b];

            a = i, b = j;
            do
            {
                b = Moving(1, b);
            } while (!DLXSPMatrix[a][b][2] && b != j);
            DLXMatrix[i][j]->right = DLXMatrix[i][b];

            a = i, b = j;
            do
            {
                a = Moving(2, a);
            } while (!DLXSPMatrix[a][b][2] && a != i);
            DLXMatrix[i][j]->up = DLXMatrix[a][j];

            a = i, b = j;
            do
            {
                a = Moving(3, a);
            } while (!DLXSPMatrix[a][b][2] && a != i);
            DLXMatrix[i][j]->down = DLXMatrix[a][j];
        }
    }

    root->right = DLXMatrix[0][0];
    root->left = DLXMatrix[0][MAX_COL - 1];

    DLXMatrix[0][0]->left = root;
    DLXMatrix[0][MAX_COL - 1]->right = root;
}

//cover only rows
//should be applied when the SP-matrix is symmectric
void SubCover(NodeXY *target_node)
{
    //unlink up-down links
    for (auto right_node = target_node->right; right_node != target_node; right_node = right_node->right)
    {
        right_node->up->down = right_node->down;
        right_node->down->up = right_node->up;
        //decrement header count of elements
        right_node->header->count--;
    }
}

//uncover whole column and row
void SubUncover(NodeXY *target_node)
{
    //link back, unlinked node
    for (auto left_node = target_node->left; left_node != target_node; left_node = left_node->left)
    {
        left_node->up->down = left_node;
        left_node->down->up = left_node;

        //increment header count of elements
        left_node->header->count++;
    }
}

//cover whole column and row
void cover(NodeXY *target_node)
{
    NodeXY *header = target_node->header;
    //unlink column header
    header->left->right = header->right;
    header->right->left = header->left;
    //unlink up-down links
    for (auto row = header->down; row != header; row = row->down)
    {
        for (auto right_node = row->right; right_node != row; right_node = right_node->right)
        {
            right_node->up->down = right_node->down;
            right_node->down->up = right_node->up;
            //decrement header count of elements
            right_node->header->count--;
        }
    }
}

//uncover whole column and row
void uncover(NodeXY *target_node)
{
    NodeXY *header = target_node->header;
    //link back, unlinked node
    for (auto row_node = header->up; row_node != header; row_node = row_node->up)
    {
        for (auto left_node = row_node->left; left_node != row_node; left_node = left_node->left)
        {
            left_node->up->down = left_node;
            left_node->down->up = left_node;

            //increment header count of elements
            left_node->header->count++;
        }
    }
    //link column header
    header->left->right = header;
    header->right->left = header;
}

//find minimum column in th SP-matrix
NodeXY *FindMinColumn()
{
    NodeXY *min_candidate = root->right;
    for (auto header = root->right; header != root; header = header->right)
    {
        if (header->count < min_candidate->count)
            min_candidate = header;
    }
    return min_candidate;
}

//Recursive depth-first search with backtrack
void search(int k)
{
    //exact cover has been found
    //if matrix is empty (cover all nodes)
    if (root->right == root)
        return;

    //D. Knuth heuristic assumption --
    //cover column with smaller count of children at first,
    //to reduce branch factor.
    NodeXY *column = FindMinColumn();

    cover(column);
    for (auto row_node = column->down; row_node != column; row_node = row_node->down)
    {
        target_matrix[row_node->ID[0]][row_node->ID[1]] = row_node->ID[2];
        //pruning SP-matrix
        if (!ValidState(row_node->ID[0], row_node->ID[1]))
        {
            target_matrix[row_node->ID[0]][row_node->ID[1]] = 0;
            continue;
        }

        covered_nodes.emplace_back(row_node);

        for (auto right_node = row_node->right; right_node != row_node; right_node = right_node->right)
            cover(right_node);

        search(++k); //backtrack if any of full rows is not valid
        if (root->right == root)
            return;

        //if next search is not possible, backtrack
        //and try another value
        target_matrix[row_node->ID[0]][row_node->ID[1]] = 0;
        covered_nodes.pop_back();

        //backtrack is recursive and local relative to current column
        //it's mean if current row in column can't be used, then the search is backtrack to previous row.
        //If all rows in current column can't be used, than the search is backtrack to previous recursion state
        // - previous column (the recursion stack look like as cabbages)
        //and try to used next rows(i.e. in SP-matrix).
        column = row_node->header;

        for (auto left_node = row_node->left; left_node != row_node; left_node = left_node->left)
            uncover(left_node);
    }

    uncover(column);
}

std::vector<std::vector<int>> SolvePuzzle(const std::vector<int> &clues)
{
    //  clock_t tStart2 = clock();

    Reset();
    SetClues(clues);

    for (int i = 1; i <= 6; ++i)
        DecompositeClues(i, 2);

    //so lazy for refactor and reengineering DLXs to constexpr
    if (first_lunch)
    {

        DLXMatrix.reserve(344);
        covered_nodes.reserve(49);
        covered_nodes_dcp.reserve(49 * 3);
        sub_covered_nodes.reserve(49 * 3);
        CLUES.reserve(28);

        FillDLXSPM();
        PossiblePermute();
        first_lunch = false;
    }

    UpdateTabuList();

    //covering (subcovering) prohibited positions
    //just excluding only rows (not a column)
    for (int i = 0; i < 7; ++i)
        for (int j = 0; j < 7; ++j)
            for (int k = 0; k < 7; ++k)
                if (tabu_list[i][j][k])
                {
                    int first_column_cell = i * 7 + j;
                    int second_column_row = (k + 7 * i) + 49;
                    int third_column_col = (k + 7 * j) + 49 * 2;
                    int row_count = i * 49 + j * 7 + (k + 1);

                    SubCover(DLXMatrix[row_count][first_column_cell]);
                    SubCover(DLXMatrix[row_count][second_column_row]);
                    SubCover(DLXMatrix[row_count][third_column_col]);

                    sub_covered_nodes.emplace_back(DLXMatrix[row_count][first_column_cell]);
                    sub_covered_nodes.emplace_back(DLXMatrix[row_count][second_column_row]);
                    sub_covered_nodes.emplace_back(DLXMatrix[row_count][third_column_col]);
                }

    //cover column and row with existing values
    for (int i = 0; i < 7; ++i)
        for (int j = 0; j < 7; ++j)
            if (target_matrix[i][j])
            {
                //covering existing values
                //row and column
                int first_column_cell = i * 7 + j;
                int second_column_row = ((target_matrix[i][j] - 1) + 7 * i) + 49;
                int third_column_col = ((target_matrix[i][j] - 1) + 7 * j) + 49 * 2;
                int row_count = i * 49 + j * 7 + target_matrix[i][j];

                cover(DLXMatrix[row_count][first_column_cell]);
                cover(DLXMatrix[row_count][second_column_row]);
                cover(DLXMatrix[row_count][third_column_col]);

                covered_nodes_dcp.emplace_back(DLXMatrix[row_count][first_column_cell]);
                covered_nodes_dcp.emplace_back(DLXMatrix[row_count][second_column_row]);
                covered_nodes_dcp.emplace_back(DLXMatrix[row_count][third_column_col]);
            }

    search(0);
    //PrintMatrix();

    for (register int i = 0; i < 7; ++i)
        for (register int j = 0; j < 7; ++j)
            for (register int k = 0; k < 7; ++k)
                tabu_list[i][j][k] = false;

    //recovering to initial state of DLX
    for (auto covered = covered_nodes.rbegin(); covered != covered_nodes.rend(); ++covered)
    {
        auto column = (*covered)->header;
        auto row_node = (*covered);

        for (auto left_node = row_node->left; left_node != row_node; left_node = left_node->left)
            uncover(left_node);

        uncover(column);
    }

    for (auto covered = covered_nodes_dcp.rbegin(); covered != covered_nodes_dcp.rend(); ++covered)
    {
        uncover((*covered));
    }

    for (auto covered = sub_covered_nodes.rbegin(); covered != sub_covered_nodes.rend(); ++covered)
    {
        SubUncover((*covered));
    }

    covered_nodes.clear();
    covered_nodes_dcp.clear();
    sub_covered_nodes.clear();
    CLUES.clear();

  //  PrintMatrix();
  //  cout << ValidState() << endl;

    //  printf("Time taken total: %.2fms\n", (double)(clock() - tStart2) / (CLOCKS_PER_SEC / 1000)); //+ 2.7);

    return target_matrix;
}
//Code optimization
//lvl 1 -- replace push_back to emplace_back                     <--- DONE
//lvl 2 -- replace slow transpose method to fast in-place method <--- DONE
//lvl 3 -- add prun method to verifying row/col                  <--- DONE
//lvl 4 -- add uncovering to initial state of DLX                <--- DONE
//lvl 5 -- add constexpr to DLX                                  <--- UNDONE
//lvl 6 -- change defaul compiller flags to -03                  <--- DONE (at local machine.)

int main()
{
    clock_t tStart = clock();

    vector<vector<int>> clues = {{7, 0, 0, 0, 2, 2, 3, 0, 0, 3, 0, 0, 0, 0, 3, 0, 3, 0, 0, 5, 0, 0, 0, 0, 0, 5, 0, 4},
                                 {7, 0, 0, 0, 2, 2, 3, 0, 0, 3, 0, 0, 0, 0, 3, 0, 3, 0, 0, 5, 0, 0, 0, 0, 0, 5, 0, 4},
                                 {6, 4, 0, 2, 0, 0, 3, 0, 3, 3, 3, 0, 0, 4, 0, 5, 0, 5, 0, 2, 0, 0, 0, 0, 4, 0, 0, 3},
                                 {0, 0, 0, 5, 0, 0, 3, 0, 6, 3, 4, 0, 0, 0, 3, 0, 0, 0, 2, 4, 0, 2, 6, 2, 2, 2, 0, 0},
                                 {0, 0, 5, 0, 0, 0, 6, 4, 0, 0, 2, 0, 2, 0, 0, 5, 2, 0, 0, 0, 5, 0, 3, 0, 5, 0, 0, 3},
                                 {0, 0, 5, 3, 0, 2, 0, 0, 0, 0, 4, 5, 0, 0, 0, 0, 0, 3, 2, 5, 4, 2, 2, 0, 0, 0, 0, 5},
                                 {0, 2, 3, 0, 2, 0, 0, 5, 0, 4, 5, 0, 4, 0, 0, 4, 2, 0, 0, 0, 6, 5, 2, 2, 2, 2, 4, 1},
                                 {0, 2, 3, 0, 2, 0, 0, 5, 0, 4, 5, 0, 4, 0, 0, 4, 2, 0, 0, 0, 6, 5, 0, 0, 0, 0, 0, 0},
                                 {3, 0, 3, 0, 0, 5, 0, 0, 0, 0, 0, 5, 0, 4, 7, 0, 0, 0, 2, 2, 3, 0, 0, 3, 0, 0, 0, 0},
                                 {0, 0, 0, 4, 0, 0, 3, 6, 4, 0, 2, 0, 0, 3, 0, 3, 3, 3, 0, 0, 4, 0, 5, 0, 5, 0, 2, 0},
                                 {0, 6, 3, 4, 0, 0, 0, 3, 0, 0, 0, 2, 4, 0, 2, 6, 2, 2, 2, 0, 0, 0, 0, 0, 5, 0, 0, 3},
                                 {0, 3, 0, 5, 0, 0, 3, 0, 0, 5, 0, 0, 0, 6, 4, 0, 0, 2, 0, 2, 0, 0, 5, 2, 0, 0, 0, 5},
                                 {2, 2, 0, 0, 0, 0, 5, 0, 0, 5, 3, 0, 2, 0, 0, 0, 0, 4, 5, 0, 0, 0, 0, 0, 3, 2, 5, 4},
                                 {0, 4, 2, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 0, 2, 0, 0, 5, 0, 4, 5, 0, 4, 0},
                                 {0, 0, 3, 0, 0, 0, 0, 3, 0, 3, 0, 0, 5, 0, 0, 0, 0, 0, 5, 0, 4, 7, 0, 0, 0, 2, 2, 3},
                                 {0, 3, 3, 3, 0, 0, 4, 0, 5, 0, 5, 0, 2, 0, 0, 0, 0, 4, 0, 0, 3, 6, 4, 0, 2, 0, 0, 3},
                                 {2, 6, 2, 2, 2, 0, 0, 0, 0, 0, 5, 0, 0, 3, 0, 6, 3, 4, 0, 0, 0, 3, 0, 0, 0, 2, 4, 0},
                                 {4, 0, 0, 2, 0, 2, 0, 0, 5, 2, 0, 0, 0, 5, 0, 3, 0, 5, 0, 0, 3, 0, 0, 5, 0, 0, 0, 6},
                                 {0, 0, 0, 4, 5, 0, 0, 0, 0, 0, 3, 2, 5, 4, 2, 2, 0, 0, 0, 0, 5, 0, 0, 5, 3, 0, 2, 0},
                                 {0, 0, 3, 0, 0, 0, 0, 3, 0, 3, 0, 0, 5, 0, 0, 0, 0, 0, 5, 0, 4, 7, 0, 0, 0, 2, 2, 3},
                                 {0, 3, 3, 3, 0, 0, 4, 0, 5, 0, 5, 0, 2, 0, 0, 0, 0, 4, 0, 0, 3, 6, 4, 0, 2, 0, 0, 3},
                                 {2, 6, 2, 2, 2, 0, 0, 0, 0, 0, 5, 0, 0, 3, 0, 6, 3, 4, 0, 0, 0, 3, 0, 0, 0, 2, 4, 0},
                                 {4, 0, 0, 2, 0, 2, 0, 0, 5, 2, 0, 0, 0, 5, 0, 3, 0, 5, 0, 0, 3, 0, 0, 5, 0, 0, 0, 6},
                                 {0, 0, 0, 4, 5, 0, 0, 0, 0, 0, 3, 2, 5, 4, 2, 2, 0, 0, 0, 0, 5, 0, 0, 5, 3, 0, 2, 0}};

    for (int i = 0; i < clues.size(); ++i)
    {
        SolvePuzzle(clues[i]);

        cout << "variant is: " << i << endl;
    }
    //  SolvePuzzle(clues[4]);
    //    SolvePuzzle(clues[22]);
    /*    vector<vector<int>> clues ={
    {7, 0, 0, 0, 2, 2, 3, 0, 0, 3, 0, 0, 0, 0, 3, 0, 3, 0, 0, 5, 0, 0, 0, 0, 0, 5, 0, 4},
    {0, 0, 5, 3, 0, 2, 0, 0, 0, 0, 4, 5, 0, 0, 0, 0, 0, 3, 2, 5, 4, 2, 2, 0, 0, 0, 0, 5},
{0,0,0,4,5,0,0,0,0,0,3,2,5,4,2,2,0,0,0,0,5,0,0,5,3,0,2,0},
{0,0,0,4,5,0,0,0,0,0,3,2,5,4,2,2,0,0,0,0,5,0,0,5,3,0,2,0}
};

for (int i = 1; i <clues.size(); ++i)
        SolvePuzzle(clues[i]);
        */

    printf("Time taken total: %.2fms\n", (double)(clock() - tStart) / (CLOCKS_PER_SEC / 1000)); //+ 2.7);
}