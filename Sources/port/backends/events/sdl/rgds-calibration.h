#ifndef RGDS_CALIBRATION_H
#define RGDS_CALIBRATION_H
#include <cmath>
struct RgdsCalibration {
    double a=1,b=0,c=0,d=0,e=1,f=0;
    void map(double u,double v,double &x,double &y) const {
        x=a*u+b*v+c; y=d*u+e*v+f;
    }
    bool valid() const {
        return std::isfinite(a)&&std::isfinite(b)&&std::isfinite(c)&&std::isfinite(d)&&std::isfinite(e)&&std::isfinite(f)
            && std::fabs(a)<4&&std::fabs(b)<4&&std::fabs(c)<4&&std::fabs(d)<4&&std::fabs(e)<4&&std::fabs(f)<4
            && std::fabs(a*e-b*d)>0.05;
    }
};
struct RgdsCalibrationPoint { double u,v,x,y; };
inline bool rgdsFitCalibration(const RgdsCalibrationPoint *p,int count,RgdsCalibration &out) {
    if (count<3) return false;
    double m[3][5]={};
    for(int k=0;k<count;++k) {
        double q[3]={p[k].u,p[k].v,1};
        for(int i=0;i<3;++i) {
            for(int j=0;j<3;++j)m[i][j]+=q[i]*q[j];
            m[i][3]+=q[i]*p[k].x;m[i][4]+=q[i]*p[k].y;
        }
    }
    for(int i=0;i<3;++i) {
        int pivot=i;for(int r=i+1;r<3;++r)if(std::fabs(m[r][i])>std::fabs(m[pivot][i]))pivot=r;
        if(std::fabs(m[pivot][i])<1e-10)return false;
        for(int j=0;j<5;++j){double t=m[i][j];m[i][j]=m[pivot][j];m[pivot][j]=t;}
        double div=m[i][i];for(int j=i;j<5;++j)m[i][j]/=div;
        for(int r=0;r<3;++r)if(r!=i){double factor=m[r][i];for(int j=i;j<5;++j)m[r][j]-=factor*m[i][j];}
    }
    out.a=m[0][3];out.b=m[1][3];out.c=m[2][3];out.d=m[0][4];out.e=m[1][4];out.f=m[2][4];
    return out.valid();
}
#endif
