#include "backends/events/sdl/rgds-calibration.h"
#include <cassert>
#include <cstdio>
#include <limits>
int main(){
    const double matrices[][6]={
        {1,0,0,0,1,0},{-1,0,1,0,-1,1},{0,1,0,-1,0,1},
        {0.91,0.035,0.04,-0.02,1.08,-0.03}
    };
    for(const auto &coeff:matrices){
        RgdsCalibration expected;
        expected.a=coeff[0];expected.b=coeff[1];expected.c=coeff[2];expected.d=coeff[3];expected.e=coeff[4];expected.f=coeff[5];
        RgdsCalibrationPoint p[9];int k=0;
        for(int row=0;row<3;++row)for(int col=0;col<3;++col){
            double u=.1+.4*col,v=.1+.4*row,x,y;expected.map(u,v,x,y);p[k++]={u,v,x,y};
        }
        RgdsCalibration fit;assert(rgdsFitCalibration(p,9,fit));
        for(const auto &sample:p){double x,y;fit.map(sample.u,sample.v,x,y);assert(std::fabs(x-sample.x)<1e-9&&std::fabs(y-sample.y)<1e-9);}
        double x,y,ex,ey;fit.map(.37,.63,x,y);expected.map(.37,.63,ex,ey);assert(std::fabs(x-ex)<1e-9&&std::fabs(y-ey)<1e-9);
    }
    RgdsCalibrationPoint duplicate[9];for(auto &p:duplicate)p={.5,.5,.5,.5};
    RgdsCalibration fit;assert(!rgdsFitCalibration(duplicate,9,fit));
    fit.a=std::numeric_limits<double>::quiet_NaN();assert(!fit.valid());
    puts("ARM calibration tests passed: identity, flipped axes, rotation, scale/offset/skew, unseen point and invalid data.");
}
