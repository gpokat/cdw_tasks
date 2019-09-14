using namespace std;

int digital_root(int n)
{

  string line = std::to_string(n);
  int digit = 0;

  for (int i = 0; i < line.length(); i++)
  {
    digit += atoi(line.substr(i, 1).c_str());
  }

  if (digit >= 10)
    return digital_root(digit);
  else
  {
    cout << "digi: " << digit << endl;
    return digit;
  }
}
