#include "atm.hpp"

ATM::ATM()
{
	std::ifstream ifs(_ATM, std::ios::in | std::ios::binary);
	ifs.read((char*)&Cash, sizeof(unsigned));
	ifs.read((char*)&PrintingConsumes, sizeof(unsigned));
}

ATM::~ATM()
{
	std::ofstream ofs(_ATM, 
		std::ios::out | std::ios::binary | std::ios::trunc);
	ofs.write((char*)&Cash, sizeof(int));
	ofs.write((char*)&PrintingConsumes, sizeof(int));
}

void ATM::start(BankDatabase& bdt)//welcome screen
{
	while (true)
	{
		//login
		int result = UI_Login(bdt);
		if (result == 0)//fail
		{			
			std::cout << "Wrong account or password!" << std::endl;
			system("pause");
			continue;
		}
		else if (result == 1)//client
		{//sevice
			UI_Service(bdt);
		}
		else if (result == 2)//staff
		{//maintenance
			UI_Maintenance(bdt);
		}
		else if (result == -1)
		{
			std::cout << "This account has been suspended due to inputting wrong passwords for multiple times!"
				<< std::endl;
			system("pause");
			continue;
		}
		else
		{		}
	}	
}

int ATM::UI_Login(BankDatabase& bdt)//return true if successfully login
{
	//enter ac and pw
	std::string ac, pw;
	while (true)
	{
		//welcome
		clearScreen();
		printWelcome_service(std::string("LOGIN"));

		std::cout << "Account: ";
		if (getInput(ac))
			std::cout << "PIN: ";
		else
			continue;

		if (getPassword(pw))
			//verify: staff or holder or member
			return Login(ac, pw, bdt);
		else
			continue;

		
	}
	
}

int ATM::Login(const std::string& Ac, const std::string& PIN, BankDatabase& bdt)
{		
	if (bdt.checkLoginFailedTimes(Ac) == 3)
	{
		return -1;
	}

	//client login
	if (isClient(Ac))
	{
		//accept shared_ptr return by the bank
		cptr = bdt.clientLogin(Ac, PIN);
		if (cptr != nullptr)
			return 1;
		
			
	}
	else
	{
		if (bdt.staffLogin(Ac, PIN))
			return 2;
	}		
	
	return 0;
}

void ATM::UI_Service(BankDatabase& bdt)//client service
{
	while (true)
	{
		//print welcome and instructions
		clearScreen();
		printWelcome_user(cptr->getName());
		std::cout << "1) Balance Inquiry\n";
		std::cout << "2) Deposit\n";
		std::cout << "3) Withdraw\n";
		std::cout << "4) Transfer\n";
		std::cout << "5) Payment\n";
		std::cout << "6) Order Statement\n";
		std::cout << "7) Change PIN\n";
		std::cout << "8) Quit" << std::endl;
		std::cout << "Choose: ";

		//input and exception handle
		char choice;
		//int exec;
		try
		{
			//input choice
			std::cin >> choice;
			//check input
			if (!isdigit(choice))
			{
				throw std::exception("Input is not a digit! ");
			}
		}
		//handle exception
		catch (std::exception not_digit)
		{
			badInputHandler(std::cin, not_digit.what());
		}
		
		//run the chosen service
		switch (choice)
		{
		case '1':
		{
			UI_Inquiry();
			break;
		}
		case '2':
		{
			while (UI_Deposit(bdt))
			{ 
				//continue
			}
			break;
		}
		case '3':
		{
			while (UI_Withdraw(bdt))
			{ 
				//continue
			}
			break;
		}
		case '4':
		{
			while (UI_Transfer(bdt))
			{
				//continue
			}
			break;
		}
		case '5':
		{
			while (UI_Payment(bdt))
			{
				//continue
			}
			break;
		}
		case '6':
		{
			UI_Statement(bdt);
			break;
		}
		case '7':
		{
			UI_ChangePIN();
			break;
		}
		case '8':
		{
			if (UI_Quit(bdt))
				return;
			else
				break;
		}
		/*
		
		*/
		default:
		{	
			//print instruction
			std::cout << "Please choose a service (1 ~ (8!"
				<< std::endl;
			//press any key to return
			system("pause");
			continue;  }
		}//end of switch

	}//end of while
}

void ATM::UI_Inquiry()
{
	//print welcome and instruction
	clearScreen();
	printWelcome_service("INQUIRY");

	//error message
	std::string msg;

	//inquiry
	double b = Inquiry(msg);

	//output
	//successfully read balance
	if (b >= 0)
	{
		std::cout << "Your current balance: "
			<< b 
			<< std::endl;
	}
	//failed
	else
	{
		std::cout << msg
			<< std::endl;
	}

	//press any key to return
	system("pause");
}

double ATM::Inquiry(std::string& msg)
{
	if (cptr != nullptr)
	{
		return cptr->getBalance();
	}
	else
	{
		msg = "Inquiry: failed!";
		return -1;
	}		
}

bool ATM::UI_Deposit(BankDatabase& bdt)//return true if user wants to continue
{
	//print welcome and instruction
	clearScreen();
	printWelcome_service("DEPOSIT");		
	printTransactionFee(cptr->getAccount());	

	std::cout << "Put cash (Input an amount) into drawer;\n"
		<< "Accept the 100 bill only; N: return. ";
	
	//get user input
	std::string msg;
	double amount;
	transaction t;
	//get input:
	//the user input amount
	if (getInput(&amount, 'N'))
	{
		char choice;
		//if the amount is not a multiple of 100
		if ((int)amount % 100 == 0)
		{
			Deposit(msg, (double)amount, bdt, t);		
		}
		else
		{
			msg = "ATM accepts only 100 bill!";
		}
	
		// output instruction or msg
		if (msg.empty())
		{
			double b = Inquiry(msg);
			std::cout << "Deposit success! Your current balance:";
			if (b >= 0)
			{
				std::cout << b << std::endl;
			}
			else
			{
				std::cout << "Inquiry: failed!" << std::endl;
			}
			printReceipt(t);
			std::cout << "Y: continue depositing, N: return: ";
		}
		else
		{
			std::cout << msg
				<< "Please retrieve your money from the drawer.\n"
				<< "Y: try again, N: return: " 
				<< std::endl;
		}		
		//ask for choice
		//to return or continue(try again)
		
		if (getInput(&choice))
			//return
			return true;
		else		
			return false;
	}
	//user choose to return
	else
		return false;	
}

bool ATM::Deposit(std::string& msg, int amount, BankDatabase& bdt, transaction& t)
{
	if (cptr == nullptr)
		return false;

	double _amount = amount;
	if (cptr->updateBalance(_amount, _SVCFEE))
	{
		//update the bank database
		time_t tm = time(NULL);
		std::string payer;
		std::string payee(cptr->getAccount());
		//add service fee
		if (!isMember(cptr->getAccount()))
		{
			int serviceFee = -amount * ServiceFeeRate;
			if (serviceFee > MinimumServiceFee)
				serviceFee = MinimumServiceFee;
			else if (serviceFee < MaximumServiceFee)
				serviceFee = MaximumServiceFee;
			else
			{ }
			amount = amount + serviceFee;
		}
		transaction tst(tm, amount, payer, payee);
		t = tst;
		bdt.recordTransaction(tst);

		//update atm
		Cash += amount;
				
		return true;
	}
	else
	{
		msg = "Deposit: failed!";
		return false;
	}
}

bool ATM::UI_Withdraw(BankDatabase& bdt)
{
	//print welcome and instruction
	clearScreen();
	printWelcome_service("WITHDRAW");
	printTransactionFee(cptr->getAccount());
	std::cout << "Offers 100 bill only\n"
		<< "Input an amount(Input N to return): ";
	
	//get user input
	std::string msg;
	double amount;
	transaction t;
	//get input:
	//the user input amount
	if (getInput(&amount, 'N'))
	{
		//validate the amount
		if ((int)amount % 100 == 0)
		{
			Withdraw(msg, (int)amount, bdt, t);
		}
		else
		{
			msg = "ATM offers only 100 bill!";
		}

		// output instruction or msg
		if (msg.empty())
		{
			double b = Inquiry(msg);
			std::cout << "Withdraw success!\nYour current balance:";
			if (b >= 0)
			{
				std::cout << b << std::endl;
			}
			else
			{
				std::cout << "Inquiry: failed!" << std::endl;
			}
			printReceipt(t);
			std::cout << "Please seize your money from the drawer.\n" 
				<< "Input Y to continue withdrawing, N to return: ";
		}
		else
		{
			std::cout << msg				
				<< "Input Y to try again, N to return: "
				<< std::endl;
		}
		//ask for choice
		//to return or continue(try again)
		char choice;
		if (getInput(&choice))
			//return
			return true;
		else
			return false;
	}
	//user choose to return
	else
		return false;
}

bool ATM::Withdraw(std::string& msg, int amount, BankDatabase& bdt, transaction& t)
{
	if (cptr == nullptr)
		return false;
	
	
	if (Cash - amount < 0)
	{
		msg = "Withdraw: ATM ran out of cash!";
		return false;
	}

	double _amount = 0 - amount;
	if (cptr->updateBalance(_amount, _SVCFEE))
	{
		//update the bank database
		time_t tm = time(NULL);
		std::string payer(cptr->getAccount());
		std::string payee;
		//add service fee
		if (!isMember(cptr->getAccount()))
		{
			int serviceFee = amount * ServiceFeeRate;
			if (-serviceFee > MinimumServiceFee)
				serviceFee = -MinimumServiceFee;
			else if (-serviceFee < MaximumServiceFee)
				serviceFee = -MaximumServiceFee;
			else
			{
			}
			amount = amount + serviceFee;
		}
		transaction tst(tm, amount, payer, payee);
		bdt.recordTransaction(tst);
		t = tst;
		//update atm
		Cash -= amount;

		return true;
	}
	else
	{
		msg = "Withdraw: insufficient balance!";
		return false;
	}
}

void ATM::UI_Statement(BankDatabase& bdt)
{
	//print welcome and instruction
	clearScreen();
	printWelcome_service("STATEMENT");

	//if the client is not a bank member
	if (!isMember(cptr->getAccount()))
	{
		std::cout << "Sorry, this service is only available to our bank member!"
			<< std::endl;
		system("pause");
		return;
	}

	//obtain transaction records
	std::vector<transaction> stm;

	Statement(stm, bdt);
	if (!stm.empty())
	{
	//for
		for (size_t i = 0; i != stm.size(); i++)
		{
			std::cout << "Transaction "
				<< i + 1
				<< std::endl;
				printTransaction(stm[i]);		
				std::cout << std::endl;
		}//end of for
	}
	else
	{
		std::cout << "No transaction record!";
	}
		
	//press any key to return
	system("pause");
}

void ATM::Statement(std::vector<transaction>& stm, BankDatabase& bdt)
{
	bdt.orderStatement(cptr->getAccount(), stm);
}

void ATM::UI_ChangePIN()//bank member only. input current PIN then input new one twice
{
	//print welcome and instruction
	clearScreen();
	printWelcome_service("CHANGE PIN");

	//if the client is not a bank member
	if (!isMember(cptr->getAccount()))
	{
		std::cout << "Sorry, this service is only available to our bank member!"
			<< std::endl;
		system("pause");
		return;
	}

	//get user input
	std::string currPIN;
	//get input:
	do
	{
		//print welcome and instruction
		clearScreen();
		printWelcome_service("CHANGE PIN");
		std::cout << "N: return;" << std::endl;
		std::cout << "Input your current PIN: ";
		if (getPassword(currPIN))
		{
			std::string nPIN, _nPIN;
			std::cout << "Input your new PIN: ";
			if (getPassword(nPIN))
			{
				std::cout << "Input your new PIN again: ";
				if (getPassword(_nPIN))
				{
					std::string msg;
					if (currPIN != cptr->getPIN())
					{
						msg = "Current PIN incorrect!";
					}
					else if (nPIN != _nPIN)
					{
						msg = "Different new PINs input!";
					}
					else if (nPIN.size() != 6)
					{
						msg = "PIN must be 6-digits!";
					}
					else //all correct
					{
						cptr->changePIN(nPIN);
					}
					
					//print result
					std::cout << "Change PIN: ";
					//success
					if (msg.empty())
					{
						std::cout << "success!" << std::endl;
						system("pause");
						return;
					}
					else
					{ 
						std::cout << msg
							<< " Input Y to try again, N to return:";
						char choice;
						if (getInput(&choice, 'Y', 'N'))
						{
							continue;
						}
						else
						{
							return;
						}
					}
				}// _nPIN
			}// nPIN
			else
			{
				return;
			}
		}//currPIN
		else
		{
			return;
		}
	} while (true);

}

void ATM::ChangePIN(std::string& msg, std::string& newPIN)
{
	cptr->changePIN(newPIN);
}

bool ATM::UI_Quit(BankDatabase& bdt)
{
	//show instruction
	clearScreen();
	printWelcome();
	std::cout << "Do you want to quit the session?\n"
		<< "Y: yes, N: return:";

	//input to confirm quitting
	char choice;
	if (getInput(&choice, 'Y', 'N'))
	{
		//is it client or a staff?
		if (cptr != nullptr)
			Quit(bdt);
		return true;
	}
	else
		return false;
}

void ATM::Quit(BankDatabase& bdt)
{
	//record ac
	std::string ac = cptr->getAccount();

	//release the pointer to the account
	cptr.reset();
	
	//inform bankdatabase of the quitting
	bdt.accountQuit(ac);
}

bool ATM::UI_Transfer(BankDatabase& bdt)
{
	//print welcome and instruction
	clearScreen();
	printWelcome_service("TRANSFER");
	printTransactionFee();
	std::cout << "N: cancel at anytime\n";
	
	//input account
	std::cout << "Input account:";
	std::string othAc;
	transaction t;
	if (getInput(othAc, 'N'))
	{
		// input amount
		std::string msg;
		double amount;		
		std::cout << "Input amount:";
		if (getInput(&amount, 'N'))
		{
			if (Transfer(msg, amount, othAc, bdt, t))
			{
				//success!
				//
				std::cout << "Transfer success!\n"
					<< "Your current balance: ";
				double b = Inquiry(msg);
				if (b >= 0)
				{
					std::cout << b << std::endl;
				}
				else
				{
					std::cout << "Inquiry: failed!" << std::endl;
				}
				printReceipt(t);
				std::cout << "Y: continue transferring, N: return to main menu: ";

			}
			//output error
			else
			{
				
				std::cout << msg
					<< "\nY: to try again, N: return to main menu:";
			}
		}
	}
	//does not input an account
	else
	{
		std::cout << "Y: try again, N: return to main menu:";
	}

	//try again or return
	char choice;
	if (getInput(&choice))
		return true;
	else
		return false;
}

bool ATM::Transfer(std::string& msg, double amount, const std::string& othAc, BankDatabase& bdt, transaction& t)
{
	//self transfer
	if (othAc == cptr->getAccount())
	{
		msg = "Transfer: self-transfer is not allowed!";
		return false;
	}
	else
	{
		return bdt.transfer(msg, cptr, othAc, amount, t);
	}
	
}

bool ATM::UI_Payment(BankDatabase& bdt)
{
	//select service
	char choice;
	std::string payee;
	while (true)
	{
		//print welcome and instructions
		clearScreen();
		printWelcome_service("PAYMENT");
		std::cout << "1) Water\n";
		std::cout << "2) Electricity\n";
		std::cout << "3) Phone\n";
		std::cout << "4) Social Security\n";
		std::cout << "5) Quit" << std::endl;
		std::cout << "Choose: ";

		//input and exception handle
		
		//int exec;
		try
		{
			//input choice
			std::cin >> choice;
			//check input
			if (!isdigit(choice))
			{
				throw std::exception("Input is not a digit! ");
			}
		}
		//handle exception
		catch (std::exception not_digit)
		{
			badInputHandler(std::cin, not_digit.what());
		}

		//run the chosen service
		clearScreen();
		switch (choice)
		{
		case '1':
		{
			printWelcome_service("PAYMENT: WATER");
			payee = _WATER_PAYMENT;
			break;
		}
		case '2':
		{
			printWelcome_service("PAYMENT: ELECTRICITY");
			payee = _ELECTRICITY_PAYMENT;
			break;
		}
		case '3':
		{
			printWelcome_service("PAYMENT: PHONE");
			payee = _PHONE_PAYMENT;
			break;
		}
		case '4':
		{
			printWelcome_service("PAYMENT: SOCIAL SECURITY");
			payee = _SOCIALSECURITY_PAYMENT;
			break;
		}
		case '5':
		{
			//print instruction
			std::cout << "Do you want to return?\n"
				<< "Y: to return, N: continue: ";
			char ch;
			if (getInput(&ch, 'Y', 'N'))
			{
				//i don't want to continue payment
				return false;
			}
			else
			{
				//i want to continue payment
				continue;
			}
		}
		default:
		{
			//print instruction
			std::cout << "Please choose a service (1 ~ (5!"
				<< std::endl;
			//press any key to return
			system("pause");
			continue;
		}
		}//end of switch
		break;
	}//end of while
	
	//input an amount
	printTransactionFee(cptr->getAccount());
	std::cout << "The payment will be charged using your account balance;\n"
		<< "Input an amount, N to return: ";
	std::string msg;
	double amount;
	transaction t;
	if (getInput(&amount, 'N'))//i would like to finish the payment
	{
		//pay
		if (Payment(bdt, (int)amount, payee, msg, t))
		{
			//print success
			switch (choice)
			{
			case '1':
			{
				std::cout << "Water ";
				break;
			}
			case '2':
			{
				std::cout << "Electricity ";
				break;
			}
			case '3':
			{
				std::cout << "Phone ";
				break;
			}
			case '4':
			{
				std::cout << "Social security ";
				break;
			}
			default:
			{ std::cout << "Default "; }
			}

			//print balance
			std::cout << " bill paid successfully!\n"
				<< "Your current balance: ";
			std::string msg_inquiry;
			double b = Inquiry(msg_inquiry);
			if (b >= 0)
			{
				std::cout << b << std::endl;
			}
			else
			{
				std::cout << "Inquiry: failed!" << std::endl;
			}
			printReceipt(t);
		}
		else
		{
			//print msg
			std::cout << msg
				<< std::endl;
		}

		//try again or quit
		std::cout << "Y: ";
		if (msg.empty())
		{
			std::cout << "continue, ";
		}
		else
		{
			std::cout << "try again, ";
		}
		std::cout << "N: return:";
		if (getInput(&choice, 'Y', 'N'))
			return true;
		else
			return false;
	}
	else// i dont wanna proceed the payment!
	{
		return true;
	}
}

bool ATM::Payment(BankDatabase& bdt, int amount, const std::string& pyee, std::string& msg, transaction& t)
{
	//check balance
	if (cptr == nullptr)
		return false;

	double _amount = 0 - amount;
	if (cptr->updateBalance(_amount, _SVCFEE))
	{
		//update the bank database
		time_t tm = time(NULL);
		std::string payer(cptr->getAccount());
		std::string payee(pyee);
		//add service fee
		if (!isMember(cptr->getAccount()))
		{
			int serviceFee = amount * ServiceFeeRate;
			if (-serviceFee > MinimumServiceFee)
				serviceFee = -MinimumServiceFee;
			else if (-serviceFee < MaximumServiceFee)
				serviceFee = -MaximumServiceFee;
			else
			{
			}
			amount = amount + serviceFee;
		}

		transaction tst(tm, amount, payer, payee);

		t = tst;
		bdt.recordTransaction(tst);
		return true;
	}
	else
	{
		msg = "Payment: insufficient balance!";
		return false;
	}
}

void ATM::UI_Maintenance(BankDatabase& bdt)
{
	while (true)
	{
		//print welcome and instructions
		clearScreen();
		printWelcome();
		std::cout << "1) Cash Inquiry\n";
		std::cout << "2) Printing Consumes Inquiry\n";
		std::cout << "3) Restock Cash\n";
		std::cout << "4) Restock Printing Consumes\n";
		std::cout << "5) Quit" << std::endl;
		//print alert
		if (Cash <= 500)
		{
			std::cout << "Alert: Low in cash, please restock!"
				<< std::endl;
		}
		if (PrintingConsumes <= 50)
		{
			std::cout << "Alert: Low in printing consumes, please restock!"
				<< std::endl;
		}
		std::cout << "Choose: ";		

		//input and exception handle
		char choice;
		//int exec;
		try
		{
			//input choice
			std::cin >> choice;
			//check input
			if (!isdigit(choice))
			{
				throw std::exception("Input is not a digit! ");
			}
		}
		//handle exception
		catch (std::exception not_digit)
		{
			badInputHandler(std::cin, not_digit.what());
		}

		//run the chosen service
		switch (choice)
		{
		case '1':
		{
			UI_inquireCash();
			break;
		}
		case '2':
		{
			UI_inquirePrintingConsumes();
			break;
		}
		case '3':
		{
			UI_RestockCash();
			break;
		}
		case '4':
		{
			UI_RestockPrintingConsumes();
			break;
		}
		case '5':
		{
			if (UI_Quit(bdt))
				return;
			else
				break;
		}
		default:
		{
			break;
		}
		}//end of switch
	}//end of while
}//end of UI_Maintenance

void ATM::UI_inquireCash() const
{
	//print welcome and instruction
	clearScreen();
	printWelcome_service("INQUIRY CASH");
	std::cout << "Current amount of cash: "
		<< ATM::Cash
		<< std::endl;

	//press any key to return
	system("pause");
}

void ATM::UI_inquirePrintingConsumes() const
{
	//print welcome and instruction
	clearScreen();
	printWelcome_service("INQUIRY PRINTING CONSUMES");
	std::cout << "Current amount of printing consumes: "
		<< ATM::PrintingConsumes
		<< std::endl;

	//press any key to return
	system("pause");
}

void ATM::UI_RestockCash()
{
	//print welcome and instruction
	clearScreen();
	printWelcome_service("RESTOCK CASH");

	//restock
	RestockCash();
	std::cout << "Success!\nCurrent amount of cash: "
		<< ATM::Cash
		<< std::endl;

	//press any key to return
	system("pause");
}

void ATM::UI_RestockPrintingConsumes()
{
	//print welcome and instruction
	clearScreen();
	printWelcome_service("RESTOCK PRINTING CONSUMES");

	//restock
	RestockPrintingConsumes();

	//print
	std::cout << "Success!\nCurrent amount of printing consumes: "
		<< ATM::PrintingConsumes
		<< std::endl;

	//press any key to return
	system("pause");
}

void printWelcome_user(const std::string& username)
{
	printWelcome();
	std::cout << "Have a good day, "
		<< username
		<< "!" << std::endl;
}

void printWelcome_service(const std::string& servicename)
{
	printWelcome();
	std::cout << servicename << std::endl;
}

void printTransaction(const transaction& t)
{
	//print time
	std::cout << std::ctime(t.getTime());
	//print type
	std::string pyer = t.getPayer();
	std::string pyee = t.getPayee();
	if (pyer == "0")
	{
		std::cout << "Deposit\n"
			<< ": +";
	}
	else if (pyee == "0")
	{
		std::cout << "Withdraw\n"
			<< ": -";
	}
	else if (isPayment(pyee))
	{
		std::cout << "Payment: ";
		switch (pyee[0])
		{
		case '1':
		{
			std::cout << "Water payment";
			break;
		}
		case '2':
		{
			std::cout << "Electricity payment";
			break;
		}
		case '3':
		{
			std::cout << "Phone payment";
			break;
		}
		case '4':
		{
			std::cout << "Social security payment";
			break;
		}
		default:
		{
			break;
		}
		}//end of switch
		std::cout << "\nAmount: -";
	}
	else
	{
		std::cout << "Transfer\n"
			<< "Payer:  "
			<< pyer
			<< "\nPayee: "
			<< pyee
			<< "\n: ";
	}

	//print amount
	std::cout << t.getAmount()
		<< std::endl;
}

void ATM::printReceipt(const transaction& t)
{
	if (PrintingConsumes == 0)
	{
		std::cout << "Sorry, ATM ran out of receipt tissue!"
			<< std::endl;
		return;
	}
	PrintingConsumes--;
	std::cout << "\nPrinting receipt...\n";
	std::cout << "\t  Receipt\n"
		<< "--------Bank of CN!--------" 
		<< std::endl;
	printTransaction(t);
	std::cout << std::endl;
}

void printTransactionFee(const std::string& ac)
{
	if (!isMember(ac))
	{
		for (int i = 0; i != 38; i++)
		{
			std::cout << "-";
		}
		std::cout << std::endl;
		std::cout << "This trasaction charges service fee!\n"
			<< "The charging rate is %" << (int)(ServiceFeeRate * 100) << ";\n"
			<< "Minimum: " << -MinimumServiceFee << ", "
			<< "Maximum: " << -MaximumServiceFee << ".\n";
		for (int i = 0; i != 38; i++)
		{
			std::cout << "-";
		}
		std::cout << std::endl;
	}
}

void printTransactionFee()
{
	for (int i = 0; i != 38; i++)
	{
		std::cout << "-";
	}
	std::cout << std::endl;
	std::cout << "This trasaction may charge service fee!\n"
		<< "The charging rate is %" << (int)(ServiceFeeRate * 100) << ";\n"
		<< "Minimum: " << -MinimumServiceFee << ", "
		<< "Maximum: " << -MaximumServiceFee << ".\n";
	for (int i = 0; i != 38; i++)
	{
		std::cout << "-";
	}
	std::cout << std::endl;
}

void badInputHandler(std::istream& is, const char *excpt_str)
{
	//print an instruction
	std::cout << excpt_str
		<< " Try again!" << std::endl;

	//clear istream error and ignore all previous input
	is.clear();
	is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

bool getInput(double *amount, char retword)
{
	while (true)
	{
		//input
		std::string input;
		std::cin >> input;

		//process
		//if return word found
		retword = toupper(retword);
		if (isalpha(input[0]) 
			&& toupper(input[0]) == retword)
			return false;
		else
		{
			//try to extract digits
			try
			{
				for (char c : input)
				{
					if (!isdigit(c))
						throw std::exception("Not digit!");
				}
			}
			//handle exception
			catch (std::exception not_digit)
			{
				badInputHandler(std::cin, not_digit.what());
				continue;
			}

			//successfully extracted
			*amount = std::stod(input);
			std::cin.get();
			return true;
		}
	}//end of input while
}

bool getInput(char *choice, char contword, char retword)
{
	while (true)
	{
		//input
		char input;
		std::cin >> input;
		std::cin.get();
		input = toupper(input);
		try
		{
			if (isalpha(input))
			{
				if (input == toupper(contword))
					return true;
				if (input == toupper(retword))
					return false;
			}
			throw std::exception("Bad input!");
		}
		//handle exception
		catch (std::exception bad_input)
		{
			badInputHandler(std::cin, bad_input.what());
			continue;
		}
	}//end of input while
}

bool getInput(std::string& ac, char retword)
{
	while (true)
	{
		//input
		std::string input;
		std::cin >> input;
		
		//process
		//if return word found
		retword = tolower(retword);
		if (isalpha(input[0])
			&& tolower(input[0]) == retword)
			return false;
		else
		{
			//try to extract digits
			try
			{
				for (char c : input)
				{
					if (!isdigit(c))
						throw std::exception("Not digit!");
				}
			}
			//handle exception
			catch (std::exception not_digit)
			{
				badInputHandler(std::cin, not_digit.what());
				continue;
			}

			//successfully extracted
			ac = input;
			std::cin.get();
			return true;
		}
	}//end of while
}

bool getPassword(std::string& str, char retword)
{
	std::string input;
	retword = tolower(retword);
	char ch;
input:
	while (true)
	{
		ch = _getch();

		//if return word found		
		if (isalpha(ch)
			&& tolower(ch) == retword)
			return false;

		//only printable characters will be adopted
		if (32 <= ch && ch <= 126) {
			input.push_back(ch);
			printf("*");
		}
		//8: BackSpace
		else if (ch == 8) {
			//only backspace when i >= 2 (there is at least 1 chracter input
			if (input.size() > 0)
			{
				input.pop_back();
				printf("%c", '\b');//shift typing cursor left; balance against the "backspace" input
				printf("%c", 127);//delete the "*"
				printf("%c", '\b');//shift typing cursor left; balance against the printf "del" 
			}
		}
		//13: CarriageChange
		else if (ch == 13) {
			std::cout << std::endl;
			break;
		}
		//get function key
		else if (ch <= 0)
		{
			_getch();
		}
		else
		{
			//do nothing
		}
	}//end of while

	//try to extract digits
	try
	{
		for (char c : input)
		{
			if (!isdigit(c))
				throw std::exception("Not digit!");
		}
	}
	//handle exception
	catch (std::exception not_digit)
	{
		std::cout << not_digit.what()
			<< " Try again!" << std::endl;
		input.clear();
		goto input;
	}

	//successfully extracted
	str = input;
	return true;
}