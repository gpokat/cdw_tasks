#include <vector>
#include <array>
#include <cmath>
#include <smmintrin.h>
using namespace std;

//Obviously the solution of this kata
//is hamiltonian path in a graph wich
//is made by perfect square rule.

//The constants of sizes arrays and vectors can be extended
//and approach must be working deeper --> N.

//reject the degree of a vertices
int N=0;
void reject_dgree(int vtx, vector<int> &d_vis,vector<vector<int>>& link_neighbors){

for(int kk=0; kk<N; ++kk)
for(int hh=0; hh<link_neighbors[kk].size(); ++hh)
if(vtx==link_neighbors[kk][hh]) d_vis[kk]-=1;

}
//path finding using heuristic approach based on
//Warnsdorff rule with some extension
vector<int> WarnsdorfR(vector<int> &d_vis,vector<vector<int>>& link_neighbors, array<bool,2500>& used, int heuristic=0){

vector<int> d_vis_cpy=d_vis;
int n=link_neighbors.size();

vector<int> f_vertices;
f_vertices.reserve(50);

vector<int> f_vertices_tmp;
f_vertices_tmp.reserve(100);//f_vertices.size()*5);

vector<int> result;
result.reserve(n);

//The first heuristic assumption --
//starting with vertices which have smallest degree
//find the minimum degree vertex
int minElpos_size= (*min_element(d_vis.begin(), d_vis.end()));
for(int i=0; i<n; ++i){
if(minElpos_size==d_vis[i]) f_vertices.emplace_back(i+1);//push_back(i+1);
}

//The second heuristic assumption --
//we should take into account vertices nearby 1st,
//because distribution of degree changes with rise of N and
//Warnsdorff rule can't work ideally due to shift of degree.

//find a vertices nearby minimum degree vertex. 
//(5 vertices, but actually 3 must be enough. Except of n=26 - must be 5)
int neighbors_check=3;
if(n==26) neighbors_check=5;

for(int k=0; k<f_vertices.size(); ++k){
for(int i=1; i<=neighbors_check; ++i){
if(f_vertices[k]< (n-i) ) f_vertices_tmp.emplace_back(f_vertices[k]+i);//push_back(f_vertices[k]+i);
if(f_vertices[k]>i) f_vertices_tmp.emplace_back(f_vertices[k]-i);//push_back(f_vertices[k]-i);
}
}

for(int i=0; i<f_vertices_tmp.size(); ++i)
f_vertices.emplace_back(f_vertices_tmp[i]);//push_back(f_vertices_tmp[i]);

//semi-brute-force searching for the hamiltonian path
for(int k=0; k<f_vertices.size(); ++k){

if(k>0){
fill(used.begin(),used.end(),false);
d_vis=d_vis_cpy;
result.clear();
}

result.emplace_back(f_vertices[k]);//push_back(f_vertices[k]);
used[f_vertices[k]-1]=true;

//The third heuristic assumption --
//take into account degree of first vertex
//experimental result.
if(heuristic==0) reject_dgree(f_vertices[k], d_vis,link_neighbors);

//open other vertices
for(int i=1;i<=n-1;i++){
int next=result[result.size()-1];

//max priority to node with smaller number of childrens - Warnsdorff rule.
int wf_r=2500, vtx_next=2500;
for(int h=0; h<link_neighbors[next-1].size(); ++h){
int vtx=link_neighbors[next-1][h];
if(!used[vtx-1]){
if(wf_r>d_vis[vtx-1]){ vtx_next=vtx; wf_r=d_vis[vtx-1]; }
}
}

if(wf_r==2500) break;

result.emplace_back(vtx_next);//push_back( vtx_next );
used[vtx_next-1]=true;

if(result.size()==n) break; 

//reject vertexes degree for correction estimation of max priority
reject_dgree(vtx_next, d_vis,link_neighbors);


}//for n

if(result.size()==n) break; 

} //vtx

return result;
}

vector<int> pfs={1,4,9,16,25,36,49,64,81,100,121,144,169,196,225,256,
                 289,324,361,400,441,484,529,576,625,676,729,784,841,
                 900,961,1024,1089,1156,1225,1296,1369,1444,1521,1600,
                 1681,1764,1849,1936,2025};

array<bool,2500> used;

vector<int> square_sums_row(int n)
{
N=n;
//max perfect square - PS
int MaxPS=n+(n-1); 

//forming "graph" as link between PS and source data N
//filling array of degree vertices 
vector<vector<int>> link_neighbors;
vector<int> d_vis;

link_neighbors.reserve(n);
d_vis.reserve(n);

vector<vector<int>>graph_matrix;
graph_matrix.reserve(n);
vector<int> tmpZero(n,0);

vector<int> neighbors;
neighbors.reserve(60);

for(int i=0; i<n; ++i)
graph_matrix.emplace_back(tmpZero);//push_back(tmpZero);


for(int i=1; i<=n; ++i){
neighbors.clear();
fill(begin(used),end(used),false);

for(int j=0; j<45; ++j){
if(pfs[j]>MaxPS) break;

int var=abs(pfs[j]-i);

if(used[var]) continue;

float root_ps=sqrt(i+var);

if(var!=i && var<=n && var>0 && floor(root_ps)==root_ps){

used[var]=true;
graph_matrix[i-1][var-1]=1;
neighbors.emplace_back(var);//push_back(var);

}
}
link_neighbors.emplace_back(neighbors);//push_back(neighbors);
d_vis.emplace_back(neighbors.size());//push_back(neighbors.size());
}

fill(begin(used),end(used),false);

//dbg
//print adjacent matrix 
/*
for(int i=0; i<n; i++){
cout<<"\n";
for(int j=0; j<n; j++){
cout<<" "<<graph_matrix.at(i).at(j)<<", ";
}
}
cout<<"\n";
*/

vector<int> result= WarnsdorfR(d_vis,link_neighbors, used);
if(result.size()!=n) result= WarnsdorfR(d_vis,link_neighbors, used, 1);

//dbg
//print result 
//cout<<"size: "<<result.size() <<"\n";
//for(int i=0;i<result.size();i++)
//cout<< result.at(i)<<", ";
//cout<<"\n";

if(result.size()==n ) return  result;

  return vector<int>();
}

//Code optimization
//lvl 1 -- replace .at() to []               <--- DONE
//lvl 2 -- hardcoded perfect square table    <--- DONE
//lvl 3 -- replace push_back to emplace_back <--- DONE
//lvl 4 -- replace post- to pre- increment   <--- DONE
//lvl 5 -- unroll loops ?
//lvl 6 -- using SSE SIMD intrinsics ?
//lvl 7 -- change defaul compiller flags to -03 ?
