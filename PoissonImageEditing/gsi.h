#ifndef GSI_H
#define GSI_H


#include <algorithm>
using namespace std;


//	pair of unknown number id and its coefficient
typedef pair<int,double> GSIUnit;

//	maximum unknown number in an equation
const int MAX_GSIUNIT_IN_EQUATION = 5;

//	Gauss-Seidel iteration method class
class GSIMethod {
public:

	//	matrix of equations left
	GSIUnit **a;

	//	vector of equations right
	double *b;

	//	vector of unknown number answer
	double *ans;

	//	unknown number size
	int size;

	//	now equation id
	int nowEquation;

	//	now GSIUnit number of now equation
	int nowGSIUnitNum;

	//	constructor and destructor
	GSIMethod(int);
	~GSIMethod();

	//	add equation id
	void addEquation(int);
	
	//	add GSIUnit of now equation
	void addGSIUnit(int, double);

	//	add equation right
	void addB(double);

	//	get initial value
	double getInitialValue();

	//	solve equations using iteration
	void solve(int, int);
	void solve(double, int);
	void solve(int, double, int);
};


#endif