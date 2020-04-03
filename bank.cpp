#pragma once
#include "bank.hpp"

//CLASS TRANSACTION
transaction::transaction(const time_t& t, double a, 
	const std::string& payer, const std::string& payee)
	:
	time(t), amount(a)
{
	//if .empty(), payer/payee is cash
	if (!payer.empty()) {
		payerAccount = std::stoul(payer);
	}
	else {
		
		payerAccount = 0;
	}
	if(!payee.empty()) {
		payeeAccount = std::stoul(payee);
	}
	else {
	payeeAccount = 0;
	}

}

//visitor
const time_t* transaction::getTime() const
{
	if (validRecord())
		return &(transaction::time);
	else
		return nullptr;
}
double transaction::getAmount() const
{
	if (validRecord())
		return amount;
	else
		return 0;
}
const std::string transaction::getPayer() const
{
	std::string str;
	if (validRecord())
	{
		str = std::to_string(payerAccount);
		return str;
	}		
	else
		return str;
}
const std::string transaction::getPayee() const
{
	std::string str;
	if (validRecord())
	{
		str = std::to_string(payeeAccount);
		return str;
	}
	else
		return str;
}

//--------------------------------------------------------------------

int BankDatabase::checkLoginFailedTimes(const std::string Ac) const
{
	auto failTimes = LoginFailedTimes.find(Ac);
	if (failTimes != LoginFailedTimes.end())
		return failTimes->second;
	else
		return 0;
}

bool BankDatabase::staffLogin(const std::string& AcNo, const std::string& PIN)
{
	//openfile
	std::ifstream ifs(_STAFFAC, std::ios::in | std::ios::binary);
	if (ifs.bad())
		return false;

	//read
	ifs.seekg(0, std::ios::end);
	auto end = ifs.tellg();
	ifs.seekg(0, std::ios::beg);
	staffInfo staff;
	while (ifs.tellg() != end)
	{
		ifs.read((char*)&staff, sizeof(staffInfo));

		//if match
		unsigned staffAc = std::stoul(AcNo);
		unsigned staffPIN = std::stoul(PIN);
		if (staffAc == staff.accountNumber && staffPIN == staff.PIN)
			return true;
		//else, continue
	}
	ifs.close();
	return false;
}

std::shared_ptr<clientAccount> BankDatabase::clientLogin(const std::string& AcNo, const std::string& PIN)
{
	//if there is other client already have been visiting the account
	auto beg = activeAccount.begin();
	auto end = activeAccount.end();
	while (beg != end)
	{
		if (*(*beg) == AcNo)
		{
			if ((*beg)->verification(AcNo, PIN))
				return *beg;
			else
				return std::shared_ptr<clientAccount>(nullptr);
		}
		beg++;
	}//end of while

	//else, open from file
	std::ifstream ifs(_CLIENTAC, std::ios::in | std::ios::binary);
	if (ifs.bad())
		return std::shared_ptr<clientAccount>(nullptr);

	//get the length of the file
	ifs.seekg(0, std::ios::end);
	auto End = ifs.tellg();
	ifs.seekg(0, std::ios::beg);
	
	//read
	clientInfo pinfo;
	std::shared_ptr<clientAccount> cAc(nullptr);
	std::streampos pos = ifs.tellg();
	while (ifs.tellg() != End)
	{
		ifs.read((char*)&pinfo, sizeof(clientInfo));

		//if match
		unsigned clientAc = std::stoul(AcNo);
		unsigned clientPIN = std::stoul(PIN);
		if (pinfo.accountNumber == clientAc && pinfo.PIN == clientPIN)
		{
			//member account
			if (AcNo[0] == '1')
				cAc = std::make_shared<memberAccount>(pinfo, pos);
			//other banks' account
			else
				cAc = std::make_shared<clientAccount>(pinfo, pos);
			//add to active accountlist
			activeAccount.push_back(cAc);
			break;
		}
		else
		{
			pos = ifs.tellg();
		}
	}

	//failed to match account and password
	ifs.close();
	LoginFailedTimes[AcNo]++;
	return cAc;
}

bool BankDatabase::transfer(std::string& msg, 
	std::shared_ptr<clientAccount> pyer,
	const std::string& pyee,
	double amount,
	transaction& t)
{
	//verify and open payee account
	auto ptr_payee = transferPayee(pyee);
	if (ptr_payee == nullptr)
	{
		msg = "Transfer: invalid account!";
		return false;
	}

	//transfer
	bool transfer_success;
	double serviceFee = 0;
	if (isMember(pyer->getAccount())
		&& isMember(ptr_payee->getAccount()))
	{
		transfer_success = pyer->updateBalance(-amount, _NO_SVCFEE);
	}
	else
	{
		transfer_success = pyer->updateBalance(-amount, _SVCFEE);

		//calculate service fee
		serviceFee = amount * ServiceFeeRate;
		if (-serviceFee > MinimumServiceFee)
			serviceFee = -MinimumServiceFee;
		else if (-serviceFee < MaximumServiceFee)
			serviceFee = -MaximumServiceFee;
		else
		{
		}
		
	}
		
	if (transfer_success)
	{
		try
		{
			//PAYEE DOES NOT NEED TO PAY SERVICE FEE!
			if (!ptr_payee->updateBalance(amount, _NO_SVCFEE))
				throw std::exception("ptr_payee error!");
		}
		catch (std::exception payee_err)
		{
			msg = payee_err.what();
		}
		
		ptr_payee.reset();
		BankDatabase::accountQuit(pyee);

		//make record
		time_t tm = time(NULL);
		std::string payer(pyer->getAccount());
		std::string payee(pyee);
		amount = amount + serviceFee;
		transaction tst(tm, amount, payer, payee);
		t = tst;
		recordTransaction(tst);

		return true;
	}
	else
	{
		msg = "Transfer: insufficient balance!";
		return false;
	}
}

std::shared_ptr<clientAccount> BankDatabase::transferPayee(const std::string& AcNo)
{
	//if there is other client already have been visiting the account
	auto beg = activeAccount.begin();
	auto end = activeAccount.end();
	while (beg != end)
	{
		if (*(*beg) == AcNo)
		{
			if ((*beg)->getAccount() == AcNo)
				return *beg;
			else
				return std::shared_ptr<clientAccount>(nullptr);
		}
		beg++;
	}//end of while

	//else, open from file
	std::ifstream ifs(_CLIENTAC, std::ios::in | std::ios::binary);
	if (ifs.bad())
		return std::shared_ptr<clientAccount>(nullptr);

	//get the length of the file
	ifs.seekg(0, std::ios::end);
	auto End = ifs.tellg();
	ifs.seekg(0, std::ios::beg);

	//read
	clientInfo pinfo;
	std::shared_ptr<clientAccount> cAc(nullptr);
	std::streampos pos = ifs.tellg();
	while (ifs.tellg() != End)
	{
		ifs.read((char*)&pinfo, sizeof(clientInfo));

		//if match
		unsigned clientAc = std::stoul(AcNo);
		if (pinfo.accountNumber == clientAc)
		{
			//member account
			if (AcNo[0] == '1')
				cAc = std::make_shared<memberAccount>(pinfo, pos);
			//other banks' account
			else
				cAc = std::make_shared<clientAccount>(pinfo, pos);
			//add to active accountlist
			activeAccount.push_back(cAc);
			break;
		}
		else
		{
			pos = ifs.tellg();
		}
	}
	ifs.close();
	return cAc;
}

void BankDatabase::orderStatement(const std::string& AcNo, std::vector<transaction>& stm)
{
	//openfile
	std::ifstream ifs(_TRANSACTION, std::ios::in | std::ios::binary);
	if (ifs.bad())
		return;

	//get the length of the file
	ifs.seekg(0, std::ios::end);
	auto End = ifs.tellg();
	ifs.seekg(0, std::ios::beg);
	
	//read
	transaction t;
	while (ifs.tellg() != End)
	{
		ifs.read((char*)&t, sizeof(transaction));
		//if match
		if (t.getPayer() == AcNo || t.getPayee() == AcNo)
			stm.push_back(t);
		//else, continue
	}
	ifs.close();
}

bool BankDatabase::recordTransaction(const transaction& t)
{
	std::ofstream ofs(_TRANSACTION, 
		std::ios::out | std::ios::binary | std::ios::app);
	if (ofs.bad())
		return false;
	//write
	ofs.write((char*)&t, sizeof(transaction));
	ofs.close();
	return true;
}

void BankDatabase::accountQuit(const std::string& Ac)
{
	auto beg = activeAccount.begin();
	auto end = activeAccount.end();
	while (beg != end)
	{
		if (*(*beg) == Ac)
		{
			//save in file
			//(*beg)->SaveInFile();			
			//if the other users except the bank had quit
			if (1 == beg->use_count())
				activeAccount.erase(beg);
			return;
		}
		beg++;
	}//end of while
}
