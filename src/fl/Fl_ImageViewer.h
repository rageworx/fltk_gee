#ifndef __Fl_ImageViewer_H__
#define __Fl_ImageViewer_H__

// -----------------------------------------------------------------------------
// Little bit customized for rkDCMviewer.
// -----------------------------------------------------------------------------

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Image.H>

class Fl_ImageViewerNotifier
{
    public:
        virtual void OnRightClick( int x, int y) = 0;
        virtual void OnKeyPressed( unsigned short k, int s, int c, int a ) = 0;
        virtual void OnDrawCompleted() = 0;
};

class Fl_ImageViewer : public Fl_Box
{
    public:
        Fl_ImageViewer(int x,int y,int w,int h);
        virtual ~Fl_ImageViewer();

    public:
        // fittingtype 0: width, 1: height
        void image(Fl_RGB_Image* aimg, int fittingtype = 1);
        void unloadimage();
        void fitwidth();
        void fitheight();
        void box(Fl_Boxtype new_box);
        void type(uchar t);
        // resizemethod new types :
        // t => 0:Nearest, 1:BiLinear, 2:BiCubic, 3:Lanzcos3
        void resizemethod( int t, bool autoapply = true );

    public:
        Fl_RGB_Image* image()       { return imgsrc; }
        int       resizemethod()    { return resize_type; }
        unsigned  position()        { return naviCur; }
        unsigned  minimum()         { return naviMin; }
        unsigned  maximum()         { return naviMax; }

    public:
        void      notifier( Fl_ImageViewerNotifier* p ) { _notifier = p; }
        Fl_ImageViewerNotifier* notifier()              { return _notifier; }

    public:
        int  imgw();
        int  imgh();
        void range( unsigned minv, unsigned maxv );
        void position( unsigned v );

    public:
        // timer call
        void drawtimercheck();

    private:
        void multiplyratio( float rf );
        unsigned long long gettimenow();

    private:/// FLTK inherited ...
        int  handle( int event );
        void draw();
        void resize(int,int,int,int);

    private:
        float       multiplier;

    private:
        Fl_RGB_Image*   imgsrc;
        Fl_RGB_Image*   imgcached;
        Fl_RGB_Image*   imgNavigationBar;

    private:
        bool        drawnavigator;
        bool        isgeneratingimg;
        bool        isdrawing;
        int         resize_type;
        int         fittype;
        unsigned    naviMin;
        unsigned    naviMax;
        unsigned    naviCur;
        unsigned    naviTimestamp;
        unsigned    naviTimecheck;
        unsigned    naviBarCol;

    private:
        Fl_ImageViewerNotifier* _notifier;
};


#endif /// of __Fl_ImageViewer_H__
