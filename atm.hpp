#pragma once
//#ifndef _ATM
//#define _ATM
#include <iostream>
#include <conio.h>
#include "bank.hpp"
#include "Uncopyable.hpp" 
#define _ATM "atm.dat"
#define _WATER_PAYMENT "1"
#define _ELECTRICITY_PAYMENT  "2"
#define _PHONE_PAYMENT "3"
#define _SOCIALSECURITY_PAYMENT "4"
const int fullCash = 100000;
const int fullPrintingConsumes = 2000;
extern const double ServiceFeeRate;
extern const int MinimumServiceFee;
extern const int MaximumServiceFee;
class ATM: private Uncopyable
{
public:
	ATM();
	~ATM();
	void start(BankDatabase& bdt);//welcome screen

private:
	//attributes
    std::shared_ptr<clientAccount> cptr;//.reset() when quit
	unsigned Cash;
	unsigned PrintingConsumes;
	
	
	//return 1 - client, 2 - staff, 0 - fail
	int UI_Login(BankDatabase& bdt);
	int Login(const std::string& Ac, const std::string& PIN, BankDatabase& bdt);

	void UI_Service(BankDatabase& bdt);//client service

	//print balance; return when finishes
	void UI_Inquiry();
	//return a non-negative number if success
	double Inquiry(std::string& msg);

	//return true if continue
	bool UI_Deposit(BankDatabase& bdt);
	//return true if success
	bool Deposit(std::string& msg, int amount, BankDatabase& bdt, transaction& t);
	
	//return true if continue
	bool UI_Withdraw(BankDatabase& bdt);
	//return true if success
	bool Withdraw(std::string& msg, int amount, BankDatabase& bdt, transaction& t);

	bool UI_Transfer(BankDatabase& bdt);
	bool Transfer(std::string& msg, double amount, const std::string& othAc, BankDatabase& bdt, transaction& t);

	bool UI_Payment(BankDatabase& bdt);//return true if continue
	bool Payment(BankDatabase& bdt, int amount, const std::string& pyee, std::string& msg, transaction& t);
	//return true if confirm quitting
	bool UI_Quit(BankDatabase& bdt);
	//return true if success
	void Quit(BankDatabase& bdt);

	void UI_Statement(BankDatabase& bdt);//bank member only. print
	void Statement(std::vector<transaction>& stm, BankDatabase& bdt);

	void UI_ChangePIN();//bank member only. input current PIN then input new one twice
	void ChangePIN(std::string& msg, std::string& newPIN);//bank member only
	
	//print receipt
	void printReceipt(const transaction&);

	//maintenance: staff only
	void UI_Maintenance(BankDatabase& bdt);//staff performance
	void UI_inquireCash() const;
	void UI_inquirePrintingConsumes() const;
	void UI_RestockCash();
	void UI_RestockPrintingConsumes();

//maintenance
	unsigned inquireCash() const { return Cash; }
	unsigned inquirePrintingConsumes() const { return PrintingConsumes; }
	void RestockCash() { Cash = fullCash; }
	void RestockPrintingConsumes() { PrintingConsumes = fullPrintingConsumes; }

};
//------------------------------------------------------------------------------------------------------------------

inline bool isClient(const std::string& ac) 
{
	return ac[1] == '0'? false : true; 
}
//inline bool isStaff(const std::string& ac) { return ac[1] == 0; }

//print welcome headline
void printWelcome_user(const std::string& username);
void printWelcome_service(const std::string& servicename);
inline void printWelcome() { std::cout << "--------Welcome to Bank of CN!--------" << std::endl; }
void printTransaction(const transaction&);
void printTransactionFee(const std::string&);
void printTransactionFee();

//clear the screen
inline void clearScreen() { system("cls"); }

//ouput exception and instruction
//clear istream error and ignore all previous inputs
void badInputHandler(std::istream& is, const char *excpt_str);

//return true if gets a correct input
//return false if gets returnword
bool getInput(double *amount, char retword);

//return true if input continue-word
//return false if input return-word
bool getInput(char *choice, char contword = 'Y', char retword = 'N');

//press any key to return;

//return true if gets an account
//return false if input return-word
bool getInput(std::string& ac, char retword = 'N');

//get password input without echoing input characters
bool getPassword(std::string& str, char retword = 'N');

inline bool isPayment(const std::string& pyee)
{
	if (pyee == _WATER_PAYMENT
		|| pyee == _ELECTRICITY_PAYMENT
		|| pyee == _PHONE_PAYMENT
		|| pyee == _SOCIALSECURITY_PAYMENT)
		return true;
	else
		return false;
}
//#endif