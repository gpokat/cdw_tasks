#include <vector>
#include <algorithm>
#include <array>
#include <map>
#include <iostream>
#include <cmath>
#include <time.h>

using namespace std;

//Classic Knuth AlgorithmX for find a solution.
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
    int ID;

    //ctor (init field using nullptr and zeros)
    NodeXY() : count(0),
               ID(0),
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

vector<NodeXY *> covered_nodes;  //backtraced nodes
vector<NodeXY *> covered_nodes0; //backtraced nodes

vector<vector<NodeXY *>> DLXMatrix; //343 + row-headers
NodeXY *root = new NodeXY();        //parent node in DLXMatrix
bool first_lunch = true;            //begin flag
const int N = 9;                    //board size
const int MAX_ROW = N * N * N;
const int MAX_COL = N * N * 4;
int NN = N * N;

//The moving method which return next position in DLXMatrix
int Moving(int LRUP, int pos)
{
    switch (LRUP)
    {
    case 0: //Left
        if (pos - 1 < 0)
            return MAX_COL - 1;
        else
            return pos - 1;
    case 1: //Right
        if (pos + 1 > MAX_COL - 1)
            return 0;
        else
            return pos + 1;
    case 2: //Up
        if (pos - 1 < 0)
            return MAX_ROW;
        else
            return pos - 1;
    case 3: //Down
        if (pos + 1 > MAX_ROW)
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

    //fill SPM matrix with 0s && 1s (in this case it's {i,j,k})
    int first_column_cell = 0, second_column_row = 0, third_column_col = 0, fifth_column_block = 0;
    int column_shift = N * N, row_count = 1, block_num = 0;

    int DLXSPMatrix[MAX_ROW + 1][MAX_COL]; //+1 headers

    for (int i = 0; i < MAX_ROW + 1; ++i)
        for (int j = 0; j < MAX_COL; ++j)
            DLXSPMatrix[i][j] = 0;

    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            block_num = floor(i / 3.0);
            block_num += floor(j / 3.0);
            block_num += floor(i / 3.0) * 2;

            for (int k = 0; k < N; ++k)
            {
                second_column_row = (k + N * i) + column_shift;
                third_column_col = (k + N * j) + column_shift * 2;
                fifth_column_block = k + block_num * N + column_shift * 3;

                DLXSPMatrix[row_count][first_column_cell] = k + 1;
                DLXSPMatrix[row_count][second_column_row] = k + 1;
                DLXSPMatrix[row_count][third_column_col] = k + 1;
                DLXSPMatrix[row_count][fifth_column_block] = k + 1;

                //headers
                DLXSPMatrix[0][first_column_cell] = k + 1;
                DLXSPMatrix[0][second_column_row] = k + 1;
                DLXSPMatrix[0][third_column_col] = k + 1;
                DLXSPMatrix[0][fifth_column_block] = k + 1;

                row_count++;
            }                    //k
            first_column_cell++; //0-81
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
            } while (!DLXSPMatrix[a][b] && b != j);
            DLXMatrix[i][j]->left = DLXMatrix[i][b];

            a = i, b = j;
            do
            {
                b = Moving(1, b);
            } while (!DLXSPMatrix[a][b] && b != j);
            DLXMatrix[i][j]->right = DLXMatrix[i][b];

            a = i, b = j;
            do
            {
                a = Moving(2, a);
            } while (!DLXSPMatrix[a][b] && a != i);
            DLXMatrix[i][j]->up = DLXMatrix[a][j];

            a = i, b = j;
            do
            {
                a = Moving(3, a);
            } while (!DLXSPMatrix[a][b] && a != i);
            DLXMatrix[i][j]->down = DLXMatrix[a][j];
        }
    }

    root->right = DLXMatrix[0][0];
    root->left = DLXMatrix[0][MAX_COL - 1];

    DLXMatrix[0][0]->left = root;
    DLXMatrix[0][MAX_COL - 1]->right = root;
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

    cout <<k<< endl;


    //D. Knuth heuristic assumption --
    //cover column with smaller count of children at first,
    //to reduce branch factor.
    NodeXY *column = FindMinColumn();

    cover(column);
    for (auto row_node = column->down; row_node != column; row_node = row_node->down)
    {
        covered_nodes.emplace_back(row_node);

        for (auto right_node = row_node->right; right_node != row_node; right_node = right_node->right)
            cover(right_node);

        search(++k); //backtrack if any of full rows is not valid
     
         cout <<k<< endl;

     if (root->right == root)
            return;

        //if next search is not possible, backtrack
        //and try another value
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

bool validSolution(unsigned int board[9][9])
{
    covered_nodes.clear();
    covered_nodes0.clear();

    //so lazy for refactor and reengineering DLXs to constexpr
    if (first_lunch)
    {

        DLXMatrix.reserve(325);
        covered_nodes.reserve(81);
        covered_nodes0.reserve(81);

        FillDLXSPM();
        first_lunch = false;
    }
            cout<<"\n";

      for (int i = 0; i < N; ++i){
      cout<<"{";
        for (int j = 0; j < N; ++j){
        cout<<board[i][j]<<", ";
        }
        cout<<"},\n";
    }
            cout<<"\n";

    int block_num = 0;
    //cover column and row with existing values
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            if (board[i][j])
            {
                //covering existing values
                //row and column
                int first_column_cell = i * N + j;
                int second_column_row = ((board[i][j] - 1) + N * i) + N * N;
                int third_column_col = ((board[i][j] - 1) + N * j) + N * N * 2;

                block_num = floor(i / 3.0);
                block_num += floor(j / 3.0);
                block_num += floor(i / 3.0) * 2;

                int fifth_column_block = (board[i][j] - 1) + block_num * N + N * N * 3;

                int row_count = i * N * N + j * N + board[i][j];

                cover(DLXMatrix[row_count][first_column_cell]);
                cover(DLXMatrix[row_count][second_column_row]);
                cover(DLXMatrix[row_count][third_column_col]);
                cover(DLXMatrix[row_count][fifth_column_block]);

                covered_nodes0.emplace_back(DLXMatrix[row_count][first_column_cell]);
                covered_nodes0.emplace_back(DLXMatrix[row_count][second_column_row]);
                covered_nodes0.emplace_back(DLXMatrix[row_count][third_column_col]);
                covered_nodes0.emplace_back(DLXMatrix[row_count][fifth_column_block]);

                NN--;
            }

    cout << root->right<<" "<< root << endl;

    bool res=false;

    search(0);
    
     if (root->right == root) res=true;


    //recovering to initial state of DLX
    for (auto covered = covered_nodes.rbegin(); covered != covered_nodes.rend(); ++covered)
    {
        auto column = (*covered)->header;
        auto row_node = (*covered);

        for (auto left_node = row_node->left; left_node != row_node; left_node = left_node->left)
            uncover(left_node);

        uncover(column);
    }

    for (auto covered = covered_nodes0.rbegin(); covered != covered_nodes0.rend(); ++covered)
    {
        uncover((*covered));
    }

    return res;
}
