#include <iostream>
#include <complex>
#include <fftw3.h>
#include <cmath>

using namespace std;

#define re 0
#define im 1

int main(){
    int n = 5;
    fftw_complex x[n];
    fftw_complex y[n];
    for(int i = 0; i < n ; i++){
        x[i][re] = i + 1;
        x[i][im] = 0;
    }
    fftw_plan plan = fftw_plan_dft_1d(n, x, y, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(plan);
    fftw_destroy_plan(plan);
    fftw_cleanup();
    cout << "FFT =" << endl;
    for(int i =0;i<n;i++){
        if(y[i][im] < 0)
            cout << y[i][re] << " - j" << abs(y[i][im]) << endl;
        else
            cout << y[i][re] << " + j" << y[i][im] << endl;
    }
    return 0;
}


