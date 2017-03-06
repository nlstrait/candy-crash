#include <iostream>

using namespace std;

string myTest() {
  char* something = (char*)"heyyoo";
  return string(something);
}

int main(int argc, char **argv) {
  string helloMsg = "{\"action\": \"hello\"}";
  cout << helloMsg << endl;
  string myString = myTest();
  cout << myString << endl;
}
