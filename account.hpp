#pragma once
//#ifndef _ACCOUNT
//#define _ACCOUNT
#include <fstream>
#include <ios>
#include <string>
#include <stdexcept>


#define _CLIENTAC "clientAccount"
//charge service fee
#define _SVCFEE 0
//not charge service fee
#define _NO_SVCFEE 1

const bool Locked = true;
const bool Unlocked = false;
const double ServiceFeeRate = 0.01;
const int MinimumServiceFee = -2;
const int MaximumServiceFee = -100;


//accountNumber format: 5digits; 1st digit to recognize bank; 2nd to tell client and staff
//银行账号格式：6位数；第一位区分银行，第二位区分客户和职员
//the band built for simulation use 1 as 1st digit
//用于测试的银行的账号第一位是1
//staff account's 2nd digit is 0
//职员账号第二位是0
//client account's 2nd digit is non 0
//客户账号第二位非0
//PIN: 6 digits
//密码：6位数字

typedef struct //staff
{
	unsigned accountNumber;
	unsigned PIN;
} staffInfo;

//client
typedef struct{
	unsigned accountNumber;
	unsigned PIN;
	char name[20];
	double balance;
}clientInfo;

class clientAccount
{
public:
	//constructor
	clientAccount(): mutex(0) {};
	clientAccount(const clientInfo& _info, std::streampos p): info(_info), pos(p), mutex(0) {};
	//destructor
	virtual ~clientAccount() { SaveInFile(); }
	//visitor
	double getBalance();
	inline const std::string getAccount() const 
	{ 
		std::string ac = std::to_string(info.accountNumber);
		return ac;
	}
	inline const std::string getPIN() const 
	{ 
		std::string PIN = std::to_string(info.PIN);
		return PIN; 
	}
	inline const std::string getName() const { return std::string(info.name); }
	//int getfptr() const { return fptr; };
	bool verification(const std::string& AcNo, const std::string& PIN) const;

	//update balance
	//flag dictates whether it is charging service fee or not
	bool updateBalance(double amount, int flag);
	
	//operator overload
	bool operator==(const std::string& Ac) const
	{
		return getAccount() == Ac;
	}
	bool operator==(const clientAccount& oth)
	{
		return getAccount() == oth.getAccount();
	}
	//member only
	virtual void changePIN(const std::string&) {};

protected:
	//modify PIN
	void modifyPIN(const std::string&);
	//concurrency control
	void wait();
	void signal();
	//data member
	clientInfo info;
private:
	std::streampos pos;
	bool mutex;
	void SaveInFile() const;
};

class memberAccount: public clientAccount 
{
public:
	//constructor
	memberAccount(const clientInfo& _info, std::streampos f):
		clientAccount(_info, f)
	{ }
	//destructor
	~memberAccount() {}
	//update balance
	//bool updateBalance(double amount, int flag);
	//change PIN
	void changePIN(const std::string& newPIN) { modifyPIN(newPIN); }
};

inline double balanceAbsVal(double v) { return v >= 0 ? v : -v; }
//#endif // !ACCOUNT