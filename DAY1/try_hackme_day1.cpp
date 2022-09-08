
#include <iostream>
#include <string>
using namespace std;

int main()
{
	string input;
	//Hidden
	string password = "?????????";
	printf("Input the password :\n");

	getline(cin,input);
	if (input == password) {
		printf("Correct");
	}
	else {
		printf("Wrong");
	}
}
