#include <iostream>

int main() {
	char a[10] = { 0,1,2,3,4,5,6,7,8,9 };
	int b = 30;

	char* pa = a;

	std::cout << (int)pa << std::endl;

	return 0;
}