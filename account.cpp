#include "account.hpp"

double clientAccount::getBalance()
{
	wait();
	double b = info.balance;
	signal();
	return b;
}

bool clientAccount::verification(const std::string& AcNo, const std::string& PIN) const
{
	unsigned clientAc = std::stoul(AcNo);
	unsigned clientPIN = std::stoul(PIN);
	if(clientAc == info.accountNumber && clientPIN == info.PIN)
		return true;
	else
		return false;
}

void clientAccount::wait()
{
	while (mutex == Locked)
	{ /*wait*/ }
	mutex = Locked;		
}

void clientAccount::signal()
{
	mutex = Unlocked;
}

bool clientAccount::updateBalance(double amount, int flag)
{
	//account must have at least 100rmb in the account to proceed any transaction
	if (info.balance < 100)
		return false;

	double svcFee = 0;
	//do nothing if amount == 0 or _NO_SVCFEE
	if (flag == _SVCFEE)
	{
		svcFee = amount * ServiceFeeRate;
		if (amount > 0)
			svcFee = -svcFee;
		if (svcFee > MinimumServiceFee)
		{
			svcFee = MinimumServiceFee;
		}
		else if (svcFee < MaximumServiceFee)
		{
			svcFee = MaximumServiceFee;
		}
		else
		{ }
	}//end of svcFee calculating
	
	//acquire mutex
	wait();
	//modify balance
	if (info.balance + svcFee + amount < 0)
		return false;
	else
		info.balance += svcFee + amount;
	//release mutex
	signal();

	return true;
}

void clientAccount::SaveInFile() const
{
	std::ofstream ofs(_CLIENTAC, std::ios::out | std::ios::binary | std::ios::in);
	//write
	ofs.seekp(pos, std::ios::beg);
	ofs.write((char*)&info, sizeof(clientInfo));
	ofs.close();
}

void clientAccount::modifyPIN(const std::string& newPIN)
{
	wait();
	unsigned _newPIN = std::stoul(newPIN);
	info.PIN = _newPIN;
	signal();
}

