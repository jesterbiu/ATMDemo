//#include <iostream>
//#include <list>
//#include <fstream>
#include "bank.hpp"
#include "atm.hpp"
#include "account.hpp"

int main()
{
	//create atm data
	unsigned cash = 100000;
	unsigned printingconsumes = 1000;
	std::ofstream ofs_atm(_ATM, std::ios::in 
		| std::ios::binary 
		| std::ios::trunc);
	ofs_atm.write((char*)&cash, sizeof(unsigned));
	ofs_atm.write((char*)&printingconsumes, sizeof(unsigned));
	ofs_atm.close();

	//create staff account
	staffInfo si;
	si.accountNumber = 100001;
	si.PIN = 666666;
	std::ofstream ofs_staff(_STAFFAC, std::ios::in
		| std::ios::binary
		| std::ios::trunc);
	ofs_staff.write((char*)&si, sizeof(staffInfo));
	ofs_staff.close();
	
	

	//create member account
	clientInfo ci;
	ci.accountNumber = 110001;
	ci.PIN = 666666;
	char name[] = "kbjxz";
	for (size_t i = 0; i <= strlen(name); i++)
		ci.name[i] = name[i];
	ci.balance = 1000;

	//create an non-member account
	clientInfo ci1;
	ci1.accountNumber = 210002;
	ci1.PIN = 888888;
	char name1[] = "jester";
	for (size_t i = 0; i <= strlen(name1); i++)
		ci1.name[i] = name1[i];
	ci1.balance = 2000;

	//create a client data file
	std::ofstream ofs0(_CLIENTAC,
		std::ios::out | std::ios::binary | std::ios::trunc);
	if (!ofs0) {
		std::cout << "ofs error" << std::endl;
		return 0;
	}	
	//write in file
	ofs0.write((char*)&ci, sizeof(clientInfo));
	ofs0.write((char*)&ci1, sizeof(clientInfo));
	//close ofs
	ofs0.close();

	std::ifstream ifs(_CLIENTAC, std::ios::in | std::ios::binary);
	//read
	ifs.seekg(0, std::ios::end);
	auto end = ifs.tellg();
	ifs.seekg(0, std::ios::beg);	
	ifs.close();

	
	//initialize and run
	BankDatabase bdt_1;//bank has only a synthesized constructor
	ATM atm;//atm has only a defined default constructor
	atm.start(bdt_1);
	 
	system("pause");
	return 0;
}
