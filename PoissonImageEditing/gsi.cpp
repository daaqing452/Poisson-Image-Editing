#include <ctime>
#include "gsi.h"



GSIMethod::GSIMethod(int _size) {
	srand(time(0));
	size = _size;
	a = new GSIUnit*[size];
	for (int i = 0; i < size; ++i) {
		a[i] = new GSIUnit[MAX_GSIUNIT_IN_EQUATION];
		for (int j = 0; j < MAX_GSIUNIT_IN_EQUATION; ++j) a[i][j].second = 0;
	}
	b = new double[size];
	ans = new double[size];
}

GSIMethod::~GSIMethod() {
	for (int i = 0; i < size; ++i) delete[] a[i];
	delete[] a;
}

void GSIMethod::addEquation(int _nowEquation) {
	nowEquation = _nowEquation;
	nowGSIUnitNum = 0;
}

void GSIMethod::addGSIUnit(int id, double value) {
	a[nowEquation][nowGSIUnitNum++] = make_pair(id, value);
}

void GSIMethod::addB(double value) {
	b[nowEquation] = value;
}

double GSIMethod::getInitialValue() {
	return rand() % 10000 / 10000.0 * 256;
}

void GSIMethod::solve(int maxIterationTimes, int mode) {
	solve(maxIterationTimes, -1, mode);
}

void GSIMethod::solve(double maxDifference, int mode) {
	solve(-1, maxDifference, mode);
}

void GSIMethod::solve(int maxIterationTimes, double maxDifference, int mode) {

	//	initialize
	for (int i = 0; i < size; ++i) ans[i] = getInitialValue();
	
	//	backup ans
	double *d = new double[size];

	//	iterate at most maxIterationTimes times
	for (int times = 0; times != maxIterationTimes; ++times) {

		//	if all new answers is the same as the before
		bool ifSame = true;

		//	calculate new answers
		for (int i = 0; i < size; ++i) {
			d[i] = b[i];
			for (int j = 1; j < MAX_GSIUNIT_IN_EQUATION; ++j)
				d[i] -= a[i][j].second * ans[a[i][j].first];
			d[i] /= a[i][0].second;
			
			//	mode == 1	renew new answers immediately
			if (mode == 1) {
				ifSame &= (ans[i] - d[i]) < maxDifference;
				ans[i] = d[i];
			}
		}
		
		//	mode == 0	renew new answers after one iteration
		if (mode == 0)
			for (int i = 0; i < size; ++i) {
				ifSame &= (ans[i] - d[i]) < maxDifference;
				ans[i] = d[i];
			}
		
		//	exit if all difference between every new answers and old answers is less than maxDifference
		if (ifSame) break;
	}
}