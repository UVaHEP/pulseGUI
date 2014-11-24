#ifndef PATOOLS_H
#define PATOOLS_H

#include "TMath.h"

// f(x; 0=alpha, 1=n, 2=xbar, 3=sigma, 4=Scale)
double XTLBall(double *xx, double* p){
  static const double SQRTPIo2=TMath::Sqrt(3.14159265358979323846/2);
  double x=xx[0];
  double alpha=p[0];
  double n=p[1];
  double xbar=p[2];
  double sigma=p[3];
  double dx=x-xbar;
  double fn;

  double C = n/TMath::Abs(alpha)/(n-1)*TMath::Exp(-alpha*alpha/2);
  double D = SQRTPIo2 * ( 1 + TMath::Erf ( TMath::Abs(alpha)/TMath::Sqrt2() ) );
  double N = 1.0/sigma/(C+D);
  
  if  ( dx/sigma > -alpha )
    fn = TMath::Exp( -dx*dx/(2*sigma*sigma) );
  else {
    double A = TMath::Power( n/TMath::Abs(alpha) , n ) * TMath::Exp( -alpha*alpha/2 );
    double B = n/TMath::Abs(alpha) - TMath::Abs(alpha);
    fn = A * TMath::Power(( B - dx/sigma ),-n);
  }
  return p[4]*N*fn;
}

#endif
