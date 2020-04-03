#pragma once
//#ifndef  _BANK
//#define _BANK
#define _STAFFAC "staffAccount"
#define _TRANSACTION "transaction"

#include <ctime>
#include <list>
#include <vector>
#include <memory>
#include <map>
#include <iostream>
#include "account.hpp"
#include "Uncopyable.hpp"
class transaction
{
public:
	//constructor
	transaction(): amount(0) {}

	//mention:
	//a) payer == "cash": deposit
	//b) payee == "cash": withdraw
	transaction(const time_t& t, double a, const std::string& payer, const std::string& payee);

	//check if the transaction record is valid
	bool validRecord() const { return amount == 0 ? false : true; }

	//visitor
	const time_t* getTime() const;
	double getAmount() const;
	const std::string getPayer() const;
	const std::string getPayee() const;

private:
	//attributes
    time_t time;
	double amount;
	unsigned payerAccount;
	unsigned payeeAccount;
};

class BankDatabase: private Uncopyable
{
    public:		
		//return how many times a client account Ac has been tried logging in with wrong passwords
		int checkLoginFailedTimes(const std::string Ac) const;

		//login verification
		bool staffLogin(const std::string& AcNo, const std::string& PIN);
        std::shared_ptr<clientAccount> clientLogin(const std::string& AcNo, const std::string& PIN);
		
		//transfer
		bool transfer(std::string& msg, 
			std::shared_ptr<clientAccount> pyer,
			const std::string& pyee,
			double amount,
			transaction& t);

		//open statement file and return found statement
		void orderStatement(const std::string& AcNo, std::vector<transaction>& stm);

		//add a new transaction record
		bool recordTransaction(const transaction& t);

		//save changes when all users of an account quit 
        void accountQuit(const std::string& Ac);
    private:
        std::list<std::shared_ptr<clientAccount> > activeAccount;
		std::map<std::string, int> LoginFailedTimes;
		//return pointer to transfer payee; return nullptr if not found
		std::shared_ptr<clientAccount> transferPayee(const std::string& AcNo);
};

inline bool isMember(const std::string& ac) { return ac[0] == '1'; }
//#endif // ! BANK