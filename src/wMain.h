#ifndef __WMAIN_H__
#define __WMAIN_H__

#include <FL/Fl.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Image.H>

#include "Fl_BorderlessWindow.H"
#include "Fl_ImageViewer.h"

#include "Fl_GroupAniSwitch.h"

#include <list>
#include <vector>
#include <string>

class wMain : public Fl_ImageViewerNotifier
{
    public:
        wMain( int argc, char** argv );

    public:
        int  Run();
        void WidgetCB( Fl_Widget* w );
        void MenuCB( Fl_Widget* w );
        void MoveCB( Fl_Widget* w );

    public: /// inherits to Fl_RAWImageViewerNotifier
        void OnRightClick( int x, int y);
        void OnKeyPressed( unsigned short k, int s, int c, int a );
        void OnDrawCompleted();

    private:
        ~wMain();

    protected:
        void parseParams();
        void createComponents();
        void applyThemes();
        void imageview( int idx );

    protected:
        int     _argc;
        char**  _argv;

    protected:
        bool    _runsatfullscreen;
        bool    _keyprocessing;

    protected:
        Fl_BorderlessWindow*    mainWindow;
        Fl_Menu_Bar*            sysMenu;
        Fl_Group*               grpViewer;
        Fl_Group*               grpStatus;
        Fl_Group*               grpOverlay;
        Fl_Box*                 boxStatus;
        Fl_ImageViewer*         comRView;
        Fl_GroupAniSwitch*      testswitch;
};

#endif /// of __WINMAIN_H__
