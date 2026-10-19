#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "base/rgds-calibration.h"
#include "common/system.h"
#include "common/events.h"
#include "graphics/surface.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "backends/events/sdl/rgds-touch-state.h"
#include "backends/events/sdl/rgds-calibration.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>

static void caption(Graphics::Surface &surface,const char *text,int y,int scale,uint32 color) {
    const Graphics::Font *font=FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
    if (!font) return;
    Graphics::Surface line;line.create(font->getStringWidth(text)+2,font->getFontHeight(),surface.format);
    line.fillRect(Common::Rect(0,0,line.w,line.h),0);
    font->drawString(&line,Common::String(text),0,0,line.w,color);
    int start=(surface.w-line.w*scale)/2;
    for(int ly=0;ly<line.h;++ly)for(int lx=0;lx<line.w;++lx)
        if(line.getPixel(lx,ly)==color) {
            int x=start+lx*scale,py=y+ly*scale;
            if(x>=0&&py>=0&&x+scale<=surface.w&&py+scale<=surface.h)
                surface.fillRect(Common::Rect(x,py,x+scale,py+scale),color);
        }
    line.free();
}
static void present(OSystem &system,Graphics::Surface &surface,int index,const char *status=nullptr) {
    uint32 bg=surface.format.RGBToColor(15,23,42),fg=surface.format.RGBToColor(235,244,255),blue=surface.format.RGBToColor(56,189,248);
    surface.fillRect(Common::Rect(0,0,surface.w,surface.h),bg);
    caption(surface,"CALIBRAR TOUCH",5,1,fg);
    if(status)caption(surface,status,surface.h/2-20,2,fg);
    else {
        char label[32];snprintf(label,sizeof(label),"ALVO %d DE 9",index+1);
        caption(surface,label,surface.h-45,1,fg);
        caption(surface,"TOQUE NO CENTRO E LEVANTE A CANETA",surface.h-22,1,fg);
        int x=int((.08+(index%3)*.42)*(surface.w-1)),y=int((.10+(index/3)*.39)*(surface.h-1));
        for(int dy=-32;dy<=32;++dy)for(int dx=-32;dx<=32;++dx){int q=dx*dx+dy*dy;
            if(q>=28*28&&q<=31*31)surface.setPixel(x+dx,y+dy,blue);
        }
        surface.fillRect(Common::Rect(x-18,y-1,x+19,y+2),fg);
        surface.fillRect(Common::Rect(x-1,y-18,x+2,y+19),fg);
    }
    system.copyRectToOverlay(surface.getPixels(),surface.pitch,0,0,surface.w,surface.h);
    system.updateScreen();
}
int runRgdsCalibrationIfRequested(OSystem &system) {
    const char *save=getenv("SCUMMVM_RGDS_CALIBRATE");
    if(!save||!*save)return -1;
    system.beginGFXTransaction();system.initSize(1024,768);
    system.setFeatureState(OSystem::kFeatureAspectRatioCorrection,false);system.endGFXTransaction();
    system.showOverlay();
    Graphics::Surface surface;surface.create(system.getOverlayWidth(),system.getOverlayHeight(),system.getOverlayFormat());
    fprintf(stderr,"Integrated calibration overlay=%dx%d\n",surface.w,surface.h);
    present(system,surface,0);
    int fd=open("/dev/input/by-path/platform-fe5e0000.i2c-event",O_RDONLY|O_NONBLOCK|O_CLOEXEC);
    if(fd<0)fd=open("/dev/input/event1",O_RDONLY|O_NONBLOCK|O_CLOEXEC);
    char name[128]={};input_absinfo ax={},ay={},tracking={};
    if(fd<0||ioctl(fd,EVIOCGNAME(sizeof(name)),name)<0||!strstr(name,"gt9xx")||
        ioctl(fd,EVIOCGABS(ABS_MT_POSITION_X),&ax)<0||ioctl(fd,EVIOCGABS(ABS_MT_POSITION_Y),&ay)<0||
        ax.maximum<=ax.minimum||ay.maximum<=ay.minimum){fprintf(stderr,"Calibration touch unavailable\n");if(fd>=0)close(fd);surface.free();return 1;}
    RgdsTouchState state(ioctl(fd,EVIOCGABS(ABS_MT_TRACKING_ID),&tracking)==0);
    int index=0,samples=0;double sumx=0,sumy=0;
    uint32 pressed=0,ready=system.getMillis()+500;bool cancel=false;
    RgdsCalibrationPoint points[9];
    while(index<9&&!cancel) {
        Common::Event ev;
        while(system.getEventManager()->pollEvent(ev))
            if(ev.type==Common::EVENT_QUIT||(ev.type==Common::EVENT_KEYDOWN&&ev.kbd.keycode==Common::KEYCODE_ESCAPE))cancel=true;
        input_event ie;RgdsTouchFrame frame;
        for(int n=0;n<256&&read(fd,&ie,sizeof(ie))==sizeof(ie);++n){
            if(!state.consume(ie,frame))continue;
            if(frame.down&&frame.changed){pressed=system.getMillis();samples=0;sumx=sumy=0;}
            if(frame.down&&system.getMillis()>=ready){sumx+=frame.x;sumy+=frame.y;++samples;}
            if(!frame.down&&frame.changed&&samples>0&&system.getMillis()-pressed>=40&&system.getMillis()>=ready){
                double tx=.08+(index%3)*.42,ty=.10+(index/3)*.39;
                points[index]={(sumx/samples-ax.minimum)/(ax.maximum-ax.minimum),(sumy/samples-ay.minimum)/(ay.maximum-ay.minimum),tx,ty};
                fprintf(stderr,"Target %d screen=%.1f,%.1f raw=%.2f,%.2f\n",index+1,tx*(surface.w-1),ty*(surface.h-1),sumx/samples,sumy/samples);
                ++index;samples=0;ready=system.getMillis()+200;break;
            }
        }
        if(index<9)present(system,surface,index);
        system.delayMillis(10);
    }
    RgdsCalibration fit;double worst=0,ss=0;
    bool ok=!cancel&&index==9&&rgdsFitCalibration(points,9,fit);
    if(ok){for(const auto &p:points){double x,y;fit.map(p.u,p.v,x,y);double dx=(x-p.x)*(surface.w-1),dy=(y-p.y)*(surface.h-1);double e=std::sqrt(dx*dx+dy*dy);if(e>worst)worst=e;ss+=e*e;}
        fprintf(stderr,"Fit residual max=%.2f RMS=%.2f pixels\n",worst,std::sqrt(ss/9));ok=worst<=12&&std::sqrt(ss/9)<=6;
    }
    if(ok){std::string tmp=std::string(save)+".new";FILE *file=fopen(tmp.c_str(),"w");
        if(!file)ok=false;
        else{int n=fprintf(file,"RGDS_TOUCH_CALIBRATION_V1\n%.17g %.17g %.17g %.17g %.17g %.17g\n",fit.a,fit.b,fit.c,fit.d,fit.e,fit.f);bool closed=fclose(file)==0;
            ok=n>0&&closed&&rename(tmp.c_str(),save)==0;if(!ok)unlink(tmp.c_str());}
    }
    present(system,surface,0,ok?"CALIBRACAO GUARDADA":"CALIBRACAO NAO GUARDADA");
    system.delayMillis(2200);
    fprintf(stderr,"Integrated calibration %s\n",ok?"saved":"rejected or cancelled");
    close(fd);surface.free();system.hideOverlay();return ok?0:1;
}

// Schedule a new session without deleting the last known good calibration.
bool rgdsTouchCalibrationAvailable() {
    const char *path=getenv("SCUMMVM_RGDS_TOUCH_CALIBRATION");
    return path && *path;
}
bool requestRgdsRecalibration() {
    const char *path=getenv("SCUMMVM_RGDS_TOUCH_CALIBRATION");
    if(!path || !*path)return false;
    std::string marker=std::string(path)+".recalibrate";
    fprintf(stderr,"RGDS recalibration requested: %s\n",marker.c_str());
    FILE *file=fopen(marker.c_str(),"w");
    if(!file)return false;
    bool ok=fputs("recalibrate\n",file)>=0;
    return fclose(file)==0 && ok;
}
