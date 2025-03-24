#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <stdio.h>


int main() {
	FILE* client1 = fopen("client1.txt", "r");
	double inp, out;
	while (!feof(client1)) {
		fscanf(client1, "%lf %lf", &inp, &out);
		std::cout << inp << " " << out << "\n";
		std::cout << sin(inp) << " " << out << "\n";
		if (abs(out  - sin(inp)) > 0.001) { std::cout << "-100000 Socilal credit!!!\n"; return -1; }
	}
	std::cout << "client1 sexfull!!!\n";

	FILE* client2 = fopen("client2.txt", "r");
	while (!feof(client2)) {
		fscanf(client2, "%lf %lf", &inp, &out);
		std::cout << inp << " " << out << "\n";
		std::cout << sqrt(inp) << " " << out << "\n";
		if (abs(out - sqrt(inp)) > 0.001) { std::cout << "-100000 Socilal credit!!!\n"; return -1; }
	}
	std::cout << "client2 sexfull!!!\n";

	FILE* client3 = fopen("client3.txt", "r");
	double num1, num2, res;
	while (!feof(client3)) {
		fscanf(client3, "%lf %lf %lf", &num1, &num2,&res);
		std::cout << num1 << " " << num2 << " " <<  res << "\n";
		if (abs(res - pow(num1,num2)) > 0.001) { std::cout << abs(res - pow(num1, num2)) << " -100000 Socilal credit!!!\n"; return -1; }
	}
	std::cout << "client3 sexfull!!!\n";
}