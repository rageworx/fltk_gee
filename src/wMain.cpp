#ifdef _WIN32
    #include <windows.h>
#endif // of _WIN32

#include <unistd.h>
#include <time.h>
#include <dirent.h>

#include <cstdio>
#include <cstring>
#include <cmath>

#include "wMain.H"
#include <FL/fl_ask.H>
#include <FL/Fl_draw.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_JPEG_Image.H>

#include "fl_imgtk.h"
#include <FL/images/png.h>

#include "themes.h"
#include "resource.h"

////////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace fl_imgtk;

////////////////////////////////////////////////////////////////////////////////

#define DEF_APP_NAME            "fl_imgtk tech demo"
#define DEF_APP_DEFAULT_W       1000
#define DEF_APP_DEFAULT_H       750

////////////////////////////////////////////////////////////////////////////////

void fl_w_cb( Fl_Widget* w, void* p );
void fl_menu_cb( Fl_Widget* w, void* p );
void fl_move_cb( Fl_Widget* w, void* p );

////////////////////////////////////////////////////////////////////////////////

Fl_Menu_Item    sysMenuLists[] =
{
    {"&Help", 0, 0, 0, FL_SUBMENU},
        {"About this program", 0,       fl_menu_cb,  (void*)30, 0 },
        {"About open sources", 0,       fl_menu_cb,  (void*)31, 0 },
        {0},
    {0},
};

////////////////////////////////////////////////////////////////////////////////

void dump_flimg2png( Fl_Image* img, const char* fname )
{
    if ( ( img != NULL ) && ( fname != NULL ) )
    {
        if ( img->d() < 3 )
            return;

        if ( access( fname, F_OK ) == 0 )
        {
            if ( unlink( fname ) != 0 )
                return; /// failed to remove file !
        }

        FILE* fp = fopen( fname, "wb" );
        if ( fp != NULL )
        {
            png_structp png_ptr     = NULL;
            png_infop   info_ptr    = NULL;
            png_bytep   row         = NULL;

            png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING,
                                               NULL,
                                               NULL,
                                               NULL );
            if ( png_ptr != NULL )
            {
                info_ptr = png_create_info_struct( png_ptr );
                if ( info_ptr != NULL )
                {
                    if ( setjmp( png_jmpbuf( (png_ptr) ) ) == 0 )
                    {
                        int png_c_type = PNG_COLOR_TYPE_RGB;

                        if ( img->d() == 4 )
                        {
                            png_c_type = PNG_COLOR_TYPE_RGBA;
                        }

                        png_init_io( png_ptr, fp );
                        png_set_IHDR( png_ptr,
                                      info_ptr,
                                      img->w(),
                                      img->h(),
                                      8,
                                      png_c_type,
                                      PNG_INTERLACE_NONE,
                                      PNG_COMPRESSION_TYPE_BASE,
                                      PNG_FILTER_TYPE_BASE);

                        png_write_info( png_ptr, info_ptr );

                        const uchar* buff = (const uchar*)img->data()[0];

                        unsigned rowsz = img->w() * sizeof( png_byte ) * img->d();

                        row = (png_bytep)malloc( rowsz );
                        if ( row != NULL )
                        {
                            int bque = 0;

                            for( int y=0; y<img->h(); y++ )
                            {
                                memcpy( row, &buff[bque], rowsz );
                                bque += rowsz;

                                png_write_row( png_ptr, row );
                            }

                            png_write_end( png_ptr, NULL );

                            fclose( fp );

                            free(row);
                        }

                        png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
                        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
                    }
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

wMain::wMain( int argc, char** argv )
 : _argc( argc ),
   _argv( argv ),
   _runsatfullscreen( false ),
   _keyprocessing( false ),
   testswitch( NULL )
{
    parseParams();
    createComponents();

#ifdef _WIN32
    HICON
    hIconWindowLarge = (HICON)LoadImage( fl_display,
                                         MAKEINTRESOURCE( IDC_ICON_A ),
                                         IMAGE_ICON,
                                         64,
                                         64,
                                         LR_SHARED );

    HICON
    hIconWindowSmall = (HICON)LoadImage( fl_display,
                                         MAKEINTRESOURCE( IDC_ICON_A ),
                                         IMAGE_ICON,
                                         16,
                                         16,
                                         LR_SHARED );

    SendMessage( fl_xid( mainWindow ),
                 WM_SETICON,
                 ICON_BIG,
                 (LPARAM)hIconWindowLarge );

    SendMessage( fl_xid( mainWindow ),
                 WM_SETICON,
                 ICON_SMALL,
                 (LPARAM)hIconWindowSmall );

    mainWindow->titleicon( hIconWindowSmall );

#endif // _WIN32

    //comRView->range(1,28);
    comRView->range(1,360);
    comRView->position(1);
    imageview( 0 );

    if ( _runsatfullscreen == true )
    {
        mainWindow->enlarge();
    }
}

wMain::~wMain()
{

}

int wMain::Run()
{
    return Fl::run();
}

void wMain::parseParams()
{
}

void wMain::createComponents()
{
    int ver_i[] = {APP_VERSION};
    static char wintitle[128] = {0};

    sprintf( wintitle, "%s, version %d.%d.%d.%d",
             DEF_APP_NAME,
             ver_i[0],
             ver_i[1],
             ver_i[2],
             ver_i[3] );

    mainWindow = new Fl_BorderlessWindow( DEF_APP_DEFAULT_W,DEF_APP_DEFAULT_H, wintitle );
    if ( mainWindow != NULL)
    {
        int cli_x = mainWindow->clientarea_x();
        int cli_y = mainWindow->clientarea_y();
        int cli_w = mainWindow->clientarea_w();
        int cli_h = mainWindow->clientarea_h();

        Fl_Group* grpMenu = new Fl_Group( cli_x, cli_y, cli_w, 20 );
        if ( grpMenu != NULL )
        {
            grpMenu->begin();
        }

            sysMenu = new Fl_Menu_Bar( cli_x, cli_y, cli_w, 20 );
            if ( sysMenu != NULL )
            {
                sysMenu->menu( sysMenuLists );
                sysMenu->callback( fl_w_cb, this );
            }

        if ( grpMenu != NULL )
        {
            grpMenu->end();
        }

        cli_y += 20;
        cli_h -= 20;

        grpViewer = new Fl_Group( cli_x, cli_y , cli_w, cli_h - 20 );
        if ( grpViewer != NULL )
        {
            grpViewer->begin();
        }

        comRView = new Fl_ImageViewer( cli_x, cli_y, cli_w, cli_h - 20 );
        if ( comRView != NULL )
        {
#ifndef DEBUG
            comRView->resizemethod( 1 );
#endif // DEBUG
            comRView->callback( fl_w_cb, this );
            comRView->notifier( this );
        }

        if ( grpViewer != NULL )
        {
            grpViewer->end();
        }

        grpStatus = new Fl_Group( cli_x, cli_y + cli_h - 20, cli_w, 20 );
        if ( grpStatus != NULL )
        {
            grpStatus->begin();

            boxStatus = new Fl_Box( cli_x, cli_y + cli_h - 20, cli_w, 20 );
            if ( boxStatus != NULL )
            {
                boxStatus->label( "NONE." );
            }

            grpStatus->end();
        }

        grpOverlay = new Fl_Group( cli_x, cli_y, cli_w, cli_h );
        if ( grpOverlay != NULL )
        {
            grpOverlay->box( FL_NO_BOX );
            grpOverlay->begin();

            int pp_w = cli_w / 2;
            int pp_h = cli_h / 2;
            int pp_x = ( cli_w - pp_w ) / 2;
            int pp_y = ( cli_h - pp_h ) / 2;

            Fl_Box* boxtest = new Fl_Box( cli_x + pp_x , cli_y + pp_y, pp_w, pp_h, "A\nTEST BOX\nHERE !" );
            if ( boxtest != NULL )
            {
                boxtest->box( FL_FLAT_BOX );
                boxtest->color( FL_WHITE );
            }

            grpOverlay->end();
            grpOverlay->hide();
        }

        mainWindow->end();
        mainWindow->callback( fl_w_cb, this );
        mainWindow->callback_onmove( fl_move_cb, this );

        int min_w_w = ( DEF_APP_DEFAULT_W  / 3 ) * 2;
        int min_w_h = ( DEF_APP_DEFAULT_H  / 3 ) * 2;

        mainWindow->size_range( min_w_w, min_w_h );

        // Set resize area ...
        if ( comRView != NULL )
        {
            mainWindow->clientarea()->resizable( comRView );
        }

        // put it center of current desktop.
        int dsk_x = 0;
        int dsk_y = 0;
        int dsk_w = 0;
        int dsk_h = 0;

        Fl::screen_work_area( dsk_x, dsk_y, dsk_w, dsk_h );

        int win_p_x = ( ( dsk_w - dsk_x ) - DEF_APP_DEFAULT_W ) / 2;
        int win_p_y = ( ( dsk_h - dsk_y ) - DEF_APP_DEFAULT_H ) / 2;

        mainWindow->position( win_p_x, win_p_y );

        mainWindow->show();
    }

    applyThemes();
}

void wMain::applyThemes()
{
    rkrawv::InitTheme();

    if ( mainWindow != NULL )
    {
        rkrawv::ApplyBWindowTheme( mainWindow );
    }

    if ( sysMenu != NULL )
    {
        rkrawv::ApplyMenuBarTheme( sysMenu );
    }

    if ( grpStatus != NULL )
    {
        rkrawv::ApplyGroupTheme( grpStatus );

        if ( boxStatus != NULL )
        {
            rkrawv::ApplyStatusBoxTheme( boxStatus );
        }
    }
}

void wMain::imageview( int idx )
{
    Fl_JPEG_Image* jpgimg = new Fl_JPEG_Image( "test.jpg" );
    if ( jpgimg != NULL )
    {
        Fl_RGB_Image* newimg = NULL;

#if 0
        static char tmps[256] = {0};
        float fv = (float)idx / 360.f;
        float fr = 6.28318530718f;
        fv *= fr;
        sprintf( tmps, "Free rotate %f degree ...", fv );
        boxStatus->label( tmps );
        newimg = rotatefree( (Fl_RGB_Image*)jpgimg, fv );

#else
        switch( idx )
        {
            default:
            case 0:
                boxStatus->label( "Normal image" );
                newimg = (Fl_RGB_Image*)jpgimg->copy();
                break;

            case 1:
                boxStatus->label( "Flip Vertical" );
                newimg = flipvertical( (Fl_RGB_Image*)jpgimg );
                break;

            case 2:
                boxStatus->label( "Flip Horizental" );
                newimg = fliphorizontal( (Fl_RGB_Image*)jpgimg );
                break;

            case 3:
                boxStatus->label( "Fast rotate 90 degree" );
                newimg = rotate90( (Fl_RGB_Image*)jpgimg );
                break;

            case 4:
                boxStatus->label( "Fast rotate 180 degree" );
                newimg = rotate180( (Fl_RGB_Image*)jpgimg );
                break;

            case 5:
                boxStatus->label( "Fast rotate 270 degree" );
                newimg = rotate270( (Fl_RGB_Image*)jpgimg );
                break;

            case 6:
                boxStatus->label( "Free rotate 20 degree" );
                newimg = rotatefree( (Fl_RGB_Image*)jpgimg, 20 );
                break;

            case 7:
                boxStatus->label( "Free rotate 358 degree" );
                newimg = rotatefree( (Fl_RGB_Image*)jpgimg, 0.1 );
                break;

            case 8:
                boxStatus->label( "Brightness 50% up" );
                newimg = brightness( (Fl_RGB_Image*)jpgimg, 50 );
                break;

            case 9:
                boxStatus->label( "Contrast 50% up" );
                newimg = contrast( (Fl_RGB_Image*)jpgimg, 50 );
                break;

            case 10:
                boxStatus->label( "Gamma 150%" );
                newimg = gamma( (Fl_RGB_Image*)jpgimg, 1.5 );
                break;

            case 11:
                {
                    boxStatus->label( "built in Sharpen filter" );
                    kfconfig* filter = new_kfconfig( "sharpen" );
                    if ( filter != NULL )
                    {
                        newimg = filtered( (Fl_RGB_Image*)jpgimg, filter );
                        discard_kfconfig( filter );
                    }
                }
                break;

            case 12:
                {
                    boxStatus->label( "built in Blur filter" );
                    kfconfig* filter = new_kfconfig( "blur" );
                    if ( filter != NULL )
                    {
                        newimg = filtered( (Fl_RGB_Image*)jpgimg, filter );
                        discard_kfconfig( filter );
                    }
                }
                break;

            case 13:
                {
                    boxStatus->label( "built in Blur more filter" );
                    kfconfig* filter = new_kfconfig( "blurmore" );
                    if ( filter != NULL )
                    {
                        newimg = filtered( (Fl_RGB_Image*)jpgimg, filter );
                        discard_kfconfig( filter );
                    }
                }
                break;

            case 14:
                {
                    boxStatus->label( "Custom 10x10 blurring filter (may takes seconds)" );
                    // 10x10 large filter.
                    uchar fsz = 10;
                    kfconfig filter = { fsz, fsz, 1.0f, 0.0f, fsz*fsz, NULL };
                    filter.m = new float[ fsz*fsz ];

                    for( unsigned cnt=0; cnt<(fsz*fsz); cnt++ )
                    {
                        filter.m[ cnt ] = 1.0 / ( fsz * fsz );
                    }

                    newimg = filtered( (Fl_RGB_Image*)jpgimg, &filter );

                    delete filter.m;
                }
                break;

            case 15:
                boxStatus->label( "Render window to image" );
                newimg = draw_widgetimage( mainWindow );
                break;

            case 16:
                boxStatus->label( "Render window to blurred image" );
                newimg = drawblurred_widgetimage( mainWindow, 10 );
                break;

            case 17:
                {
                    boxStatus->label( "Crop image ( center 50% )" );

                    unsigned cw = jpgimg->w() / 2;
                    unsigned ch = jpgimg->h() / 2;
                    unsigned cx = jpgimg->w() - cw;
                    unsigned cy = jpgimg->h() - ch;

                    newimg = crop( (Fl_RGB_Image*)jpgimg, cx, cy, cw, ch );
                }
                break;

            case 18:
                {
                    boxStatus->label( "Alpha channel mapping" );

                    uchar* amap = NULL;
                    unsigned amapsize = 0;

                    Fl_PNG_Image* amapsrc = new Fl_PNG_Image( "amap.png" );
                    if ( amapsrc != NULL )
                    {
                        amapsize = makealphamap( amap, (Fl_RGB_Image*)amapsrc );
                    }
                    else
                    {
                        amapsize = makealphamap( amap, jpgimg->w(), jpgimg->h(), 0.8f );
                    }

                    if ( amapsize > 0 )
                    {
                        newimg = applyalpha( (Fl_RGB_Image*)jpgimg, amap, amapsize );
                    }

                    if ( amapsrc != NULL )
                    {
                        delete amapsrc;
                    }
                }
                break;

            case 19:
                {
                    boxStatus->label( "Image merging A(100%) + B(80%)" );

                    Fl_PNG_Image* mimg = new Fl_PNG_Image( "sherlock_tab_foler.png" );
                    if ( mimg != NULL )
                    {
                        if ( ( mimg->w() == 0 ) || ( mimg->h() == 0 ) )
                        {
                            printf(" PNG file has error !\n" );
                        }

                        mergeconfig cfg = {0};

                        cfg.src2putx = ( jpgimg->w() - mimg->w() ) / 2;
                        cfg.src2puty = ( jpgimg->h() - mimg->h() ) / 2;
                        cfg.src1ratio = 1.0f;
                        cfg.src2ratio = 0.8f;

                        newimg = merge( (Fl_RGB_Image*)jpgimg, (Fl_RGB_Image*)mimg, &cfg );

                        delete mimg;
                    }
                }
                break;

            case 20:
                {
                    boxStatus->label( "Image subtracting A(100%) + B(40%)" );

                    Fl_PNG_Image* mimg = new Fl_PNG_Image( "sherlock_tab_foler.png" );
                    if ( mimg != NULL )
                    {
                        if ( ( mimg->w() == 0 ) || ( mimg->h() == 0 ) )
                        {
                            printf(" PNG file has error !\n" );
                        }

                        int px = ( jpgimg->w() - mimg->w() ) / 2;
                        int py = ( jpgimg->h() - mimg->h() ) / 2;

                        newimg = subtract( (Fl_RGB_Image*)jpgimg, (Fl_RGB_Image*)mimg, px, py, 0.4f );

                        delete mimg;
                    }
                }
                break;

            case 21:
                {
                    boxStatus->label( "Draw image to image with alpha." );

                    Fl_PNG_Image* mimg = new Fl_PNG_Image( "sherlock_tab_foler.png" );
                    if ( mimg != NULL )
                    {
                        if ( ( mimg->w() == 0 ) || ( mimg->h() == 0 ) )
                        {
                            printf(" PNG file has error !\n" );
                        }

                        newimg = (Fl_RGB_Image*)jpgimg->copy();

                        int imgw = mimg->w();
                        int imgh = mimg->h();

                        drawonimage( newimg, mimg, -(imgw/3), -(imgh/3), 0.8f );
                        drawonimage( newimg, mimg, (newimg->w() - imgw) / 2,
                                                   (newimg->h() - imgh) / 2, 0.5f );
                        drawonimage( newimg, mimg, newimg->w() - (imgw*0.6),
                                                   newimg->h() - (imgh*0.6), 1.0f );

                        delete mimg;
                    }
                }
                break;

            case 22:
                boxStatus->label( "Reinhard tone mapping (HDRi)" );
                newimg = tonemapping_reinhard( (Fl_RGB_Image*)jpgimg, 0.0f, 0.3f );
                break;

            case 23:
                boxStatus->label( "Drago tone mapping (HDRi)" );
                newimg = tonemapping_drago( (Fl_RGB_Image*)jpgimg );
                break;

            case 24:
                boxStatus->label( "Invert" );
                newimg = invert( (Fl_RGB_Image*)jpgimg );
                break;

            case 25:
                boxStatus->label( "Edge enhance by factor 8" );
                newimg = edgeenhance( (Fl_RGB_Image*)jpgimg, 8 );
                break;

            case 26:
                {
                    boxStatus->label( "Color CLAHE, 128x1, clip limit = 90." );

                    unsigned rW = 128;
                    unsigned rH = 1;

                    newimg = CLAHE( (Fl_RGB_Image*)jpgimg,
                                    rW,
                                    rH,
                                    90.0f );
                }
                break;

            case 27:
                {
                    boxStatus->label( "Noire effect ( Belong to CLAHE ), 128x1, clip limit = 90, bright = 1.5x" );

                    unsigned rW = 128;
                    unsigned rH = 1;

                    newimg = noire( (Fl_RGB_Image*)jpgimg,
                                    rW,
                                    rH,
                                    90.1f,
                                    1.5f );
                }
                break;

        }
#endif // 1

        comRView->image( newimg );

        delete jpgimg;
        discard_user_rgb_image( newimg );
    }

    comRView->position( idx + 1 );
}

void wMain::WidgetCB( Fl_Widget* w )
{
    if ( w == mainWindow )
    {
        fl_message_title( "Program quit" );
        int retask = fl_ask( "%s", "Program may be terminated if you select YES, Proceed it ?" );

        if ( retask > 0 )
        {
            mainWindow->hide();
            delete mainWindow;
            mainWindow = NULL;
        }

        return;
    }
}

void wMain::MenuCB( Fl_Widget* w )
{
    Fl_Menu_* mw = (Fl_Menu_*)w;
    const Fl_Menu_Item* mi = mw->mvalue();
    if ( mi != NULL )
    {
        void* param = mi->user_data();
        if ( param != NULL )
        {
            if ( param == (void*)30 )
            {
                char tmpstr[32] = {0};

                #ifdef USING_OMP
                strcat( tmpstr, "with OpenMP," );
                #endif

                fl_message_title( "About this Program:" );
                fl_message( "%s, %s MinGW-W64 6.3.0\n"
                            "\n"
                            "(C) Copyright 2016, 2017 Rageworx software, Raphael Kim (rageworx@@gmail.com)\n"
                            "*   This program is an open source project, and part of FLTK project.\n"
                            "*   All program codes by Rapahel Kay.",
                            DEF_APP_NAME, tmpstr );
            }
            else
            if ( param == (void*)31 )
            {
                fl_message_title( "Open sources:" );
                fl_message( "[%s] used these open source projects, \n"
                            "\n"
                            "- fltk-1.3.4-ts, https://github.com/rageworx/fltk-1.3.4-1-ts",
                            DEF_APP_NAME );
            }
        }
    }
}

void wMain::MoveCB( Fl_Widget* w )
{
}

void wMain::OnRightClick( int x, int y)
{
}

void wMain::OnKeyPressed( unsigned short k, int s, int c, int a )
{
    if ( _keyprocessing == true )
        return;

    if ( comRView == NULL )
        return;

    _keyprocessing = true;

    int rcnidx = comRView->position();

    switch( k )
    {
        case FL_MOUSEWHEEL:
            {
                if ( c > 0 )
                {
                    if( rcnidx > 1 )
                        rcnidx--;
                }
                else
                if ( c < 0 )
                {
                    if ( rcnidx < comRView->maximum() )
                        rcnidx++;
                }
            }
            break;

        case FL_Home:
            rcnidx = 1;
            break;

        case FL_End:
            rcnidx = comRView->maximum();
            break;

        case FL_Left:
            if( rcnidx > 1 )
                rcnidx--;
            break;

        case FL_Right:
            if ( rcnidx < comRView->maximum() )
                rcnidx++;
            break;

        case FL_Escape:
            comRView->unloadimage();
            mainWindow->do_callback();
            _keyprocessing = false;
            return;

        case 'd': /// dump current image to PNG.
            {
                Fl_Image* curimg = comRView->image();
                if ( curimg != NULL )
                {
                    time_t current_time;
                    struct tm* current_tm;

                    current_time = time( NULL );
                    current_tm = localtime( &current_time );

                    char tmpstr[256] = {0};
                    sprintf( tmpstr,
                             "%04d%02d%02d%02d%02d%02d-%08X.png",
                             current_tm->tm_year + 1900,
                             current_tm->tm_mon + 1,
                             current_tm->tm_mday,
                             current_tm->tm_hour,
                             current_tm->tm_min,
                             current_tm->tm_sec,
                             GetTickCount() );

                    dump_flimg2png( curimg, tmpstr );
                }
            }
            break;

        case 't': /// doing test switching.
            if ( testswitch != NULL )
            {
                _keyprocessing = false;
                return;
            }
            else
            if ( grpOverlay->visible_r() == 0 )
            {
                boxStatus->label( "Page switching effect (right to left)" );

                testswitch
                = new Fl_GroupAniSwitch( mainWindow->clientarea(),
                                         grpViewer,
                                         grpOverlay,
                                         Fl_GroupAniSwitch::ATYPE_RIGHT2LEFT,
                                         false,
                                         5000 );

                testswitch->WaitForFinish();

                delete testswitch;
                testswitch = NULL;
            }
            else
            {
                boxStatus->label( "Page switching effect (left to right)" );

                testswitch
                = new Fl_GroupAniSwitch( mainWindow->clientarea(),
                                         grpViewer,
                                         grpOverlay,
                                         Fl_GroupAniSwitch::ATYPE_LEFT2RIGHT,
                                         true,
                                         5000 );

                testswitch->WaitForFinish();

                delete testswitch;
                testswitch = NULL;

                mainWindow->redraw();
            }

            _keyprocessing = false;
            return;

        default:
            _keyprocessing = false;
            return;
    }

    imageview( rcnidx - 1 );
}

void wMain::OnDrawCompleted()
{
    _keyprocessing = false;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void fl_w_cb( Fl_Widget* w, void* p )
{
    if ( p != NULL )
    {
        wMain* wm = (wMain*)p;
        wm->WidgetCB( w );
    }
}

void fl_menu_cb( Fl_Widget* w, void* p )
{
    if ( p != NULL )
    {
        wMain* wm = (wMain*)p;
        wm->MenuCB( w );
    }
}

void fl_move_cb( Fl_Widget* w, void* p )
{
    if ( p != NULL )
    {
        wMain* wm = (wMain*)p;
        wm->MoveCB( w );
    }
}
