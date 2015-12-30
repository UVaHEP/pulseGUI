/* 
   Analyze forward IV curve for SIMP-like array
   Compensate for series resistance in circuit and estimate Rquench values
   Implicit equation for IV characteristic curve is solved using MINUIT2
 */


#include "Minuit2/FCNBase.h"
#include "TGraph.h"
#include "TFitterMinuit.h"
#include "TF1.h"
#include "TMath.h"
#include <iostream>

using std::cout;
using std::endl;


double Vfcn(double I, int nc, double rs, double *p){
  double lnIs=p[0];
  double beta=p[1];
  double rq=p[2];
  double Ic=I/nc;     // <I> per cell
  // last term below corrects for series resistance in measurement fixture
  return (1/beta*(TMath::Log(Ic)-lnIs) + Ic*rq) + I*rs;
}

double Vfcn(double I, int nc, double rs,
	    const std::vector<double> & p){
  double par[3]={p[0],p[1],p[2]};
  return Vfcn(I,nc,rs,par);
}


class MyFCN : public ROOT::Minuit2::FCNBase {
public: 

  MyFCN(TGraph *grin=0, int n=1, double r=0) : tg(grin), nc(n),
					       rs(r), xmin(-1e6),xmax(1e6) {}
  // objective fcn
  // model: I = Is(exp(B(V-IR))-1)~I = Is exp(B(V-IR))
  //        invert to fit V(I)
  double operator() (const std::vector<double> & p) const {    
    Double_t V, I;
    double val=0;
    for (int i=0; i<tg->GetN(); i++){
      tg->GetPoint(i,V,I);
      if (V<xmin || V>xmax) continue;
      double res= V - Vfcn(I,nc,rs,p);
      val+=res*res;  // minimze residuals squared
      //val+=res*res/V/V; // minimize fractional residuals squared
      //cout << V << ":" << I << ":" << rhs << " ";
    }
    //cout << endl;
    //cout << val << " " << lnIs << " " << beta << " " << rq << endl;
    return val;
  } 

  void SetGraph(TGraph *grin) {tg=grin;}
  void SetNcell(int n) {nc=n;}
  void SetRseries (double r) {rs=r;}
  void SetXmin(double x) {xmin=x;}
  void SetXmax(double x) {xmax=x;}
  
  double Up() const { return 1.; }
  
private: 
  TGraph *tg;
  int nc;     // number of cells in DUT
  double rs;  // resistance in series with DUT
  double xmin,xmax;
};


class ForwardIVfitter{
public:
  ForwardIVfitter(TGraph *grin, int n=1, double r=0) :
    tg(grin), nc(n), rs(r) {} 
  void Fit(Double_t *par);
  TGraph *GetFitGraph() {return gresult;}
  
private:
  TGraph *tg;
  TGraph *gresult;
  int nc;     // number of cell
  double rs;  // series resistance with DUT
};


void ForwardIVfitter::Fit(Double_t *par){

  TFitterMinuit * minuit = new TFitterMinuit();

  MyFCN fcn(tg,nc,rs);
    
  minuit->SetMinuitFCN(&fcn);

  // fit using a simple exponential, guess at PN I-V characteristics
  tg->Fit("expo","0","",1,2);
  cout << tg->GetFunction("expo")->GetParameter(0) << endl;
  double lnIs=exp(tg->GetFunction("expo")->GetParameter(0))/1000;
  double beta=tg->GetFunction("expo")->GetParameter(1);
  // get slope in high voltage region
  double xmin,ymin,xmax,ymax;
  tg->ComputeRange(xmin,ymin,xmax,ymax);
  double m=(ymax-tg->Eval(xmax*0.9))/(0.1*xmax);
  double rq=(1/m - rs)*nc;  // guesstimate of Rq

  cout << "rq estimate ****** "<< rq << endl;
  
  minuit->SetParameter(0,"lnIs",lnIs,0.1,0,0);
  minuit->SetParameter(1,"beta",beta,0.1,0,0);
  minuit->SetParameter(2,"Rq",rq,0.1,0,0);

  minuit->CreateMinimizer();
  minuit->FixParameter(2);
  fcn.SetXmin(4);
  fcn.SetXmax(5);
  int iret = minuit->Minimize();
  minuit->ReleaseParameter(2);
  iret = minuit->Minimize();
  for (int i=0; i<=2; i++) par[i]=minuit->GetParameter(i);

  // make graph of fit function
  gresult=new TGraph(*tg);
  double V,I;
  for (int i=0; i<tg->GetN(); i++){
    tg->GetPoint(i,V,I);
    double fitV=Vfcn(I,nc,rs,par);
    gresult->SetPoint(i,fitV,I);
    gresult->SetLineColor(2);
    //cout << V << "," << I << " : " << fitV << "  ::  ";
  }
  cout << endl;
}

