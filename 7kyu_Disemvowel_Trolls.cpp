#include <string>
#include <vector>
#include <algorithm>

using namespace std;
std::string disemvowel(std::string str)
{
    vector<string> vowels={"a","A","e","E","i","I","o","O","u","U"}; //vowels pattern
    string str_decomp;
    
    for(int i=0; i<str.length(); i++){ //scan char by char input string
    
    string current_char=str.substr(i,1); 
    
    if(find(vowels.begin(), vowels.end(),current_char)==vowels.end()) //check pattern
    str_decomp.append(current_char);
    
    }
        
    
     return str_decomp;
}
