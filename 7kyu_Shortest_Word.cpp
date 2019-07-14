#include <vector>
#include <string>
#include <algorithm>

using namespace std;
int find_short(std::string str)
{
cout<<str<<endl;
string delimiter = " "; //space is delimeter

vector<int> length_w; //buffer of words
size_t pos=0;
if(str.find(delimiter)!=string::npos){ // the simplest data validation
while ((pos = str.find(delimiter)) != string::npos) {
    
    string word = str.substr(0, str.find(delimiter)); // scan
    str.erase(0, pos + delimiter.length());
    
    length_w.push_back(word.length());
    }
    } else return str.length();
    
sort(length_w.begin(),length_w.end()); //get minimum

  return length_w.at(0);
}
