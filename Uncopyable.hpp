#pragma once
class Uncopyable
{
protected:
	Uncopyable() {};
	~Uncopyable() {};
private:
	Uncopyable(const Uncopyable& oth);
	Uncopyable& operator=(const Uncopyable& rhs);
};