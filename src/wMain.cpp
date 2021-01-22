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
   testswitch( NULL ),
   imgTestSrc( NULL )
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

#if 0
    comRView->range(1,360);
#else
    comRView->range(1,31);
#endif
    comRView->position(1);
    imageview( 0 );

    if ( _runsatfullscreen == true )
    {
        mainWindow->enlarge();
    }
}

wMain::~wMain()
{
    if ( imgTestSrc != NULL )
    {
        discard_user_rgb_image( imgTestSrc );
    }
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
#ifdef DEBUG
            comRView->resizemethod( 1, true );
#else
            comRView->resizemethod( 2, true );
#endif /// of DEBUG
            comRView->color( 0xFF33CC00 );
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
    if ( imgTestSrc == NULL )
    {
        Fl_PNG_Image* pngimg = new Fl_PNG_Image( "test.png" );
        if ( pngimg != NULL )
        {
#ifdef DEBUG
            printf( "PNG source image loaded : %ux%ux%u\n",
                    pngimg->w(), pngimg->h(), pngimg->d() );
#endif // DEBUG
        }

        imgTestSrc = (Fl_RGB_Image*)pngimg->copy();

        delete pngimg;
    }

    if ( imgTestSrc != NULL )
    {
        Fl_RGB_Image* newimg = NULL;
#if 0
        static char tmps[256] = {0};
        float fv = (float)idx / 360.f;
        float fr = 6.28318530718f;
        fv *= fr;
        sprintf( tmps, "Free rotate %f degree ...", fv );
        boxStatus->label( tmps );
        newimg = rotatefree( imgTestSrc, fv );

#else
        switch( idx )
        {
            default:
            case 0:
                //boxStatus->label( "Normal image" );
                //newimg = (Fl_RGB_Image*)imgTestSrc->copy();
                {
                    boxStatus->label( "Gradation Vertical." );
                    newimg = fl_imgtk::makegradation_v( comRView->w(),
                                                        comRView->h(),
                                                        0x7070FFFF,
                                                        0x1010907F,
                                                        true );
                }
                break;

            case 1:
                //boxStatus->label( "Flip Vertical" );
                //newimg = flipvertical( imgTestSrc );
                {
                    boxStatus->label( "Gradation Horizontal." );
                    newimg = fl_imgtk::makegradation_h( comRView->w(),
                                                        comRView->h(),
                                                        0x7070FFFF,
                                                        0x1010907F,
                                                        true );
                }
                break;

            case 2:
                {
                    boxStatus->label( "Draw Lines." );
                    newimg = (Fl_RGB_Image*)imgTestSrc->copy();
                    /*
                    newimg = fl_imgtk::makeanempty( imgTestSrc->w(),
                                                    imgTestSrc->h(),
                                                    4,
                                                    0xFFFFFFFF );
                    */
                    if ( newimg != NULL )
                    {
                        unsigned ygap = 50;

                        unsigned x1 = 10;
                        unsigned x2 = newimg->w() - 10;
                        unsigned y1 = 10;
                        unsigned y2 = y1 + ygap;

                        unsigned cols[] = { 0xFF0000FF,
                                            0x00FF00FF,
                                            0x0000FFFF,
                                            0xFF00007F,
                                            0x00FF007F,
                                            0x0000FF7F };

                        for( size_t cnt=0; cnt<6; cnt++ )
                        {
                            fl_imgtk::draw_line( newimg,
                                                 x1, y1 + ( 10 * cnt ),
                                                 x2, y2 + ( 10 * cnt ),
                                                 cols[cnt] );
                        }

                        y1 = 100;
                        y2 = y1 + ygap;

                        for( size_t cnt=0; cnt<6; cnt++ )
                        {
                            fl_imgtk::draw_smooth_line( newimg,
                                                        x1, y1 + ( 10 * cnt ),
                                                        x2, y2 + ( 10 * cnt ),
                                                        cols[cnt] );
                        }

                        y1 = 200;
                        y2 = y1 + ygap;

                        for( size_t cnt=0; cnt<6; cnt++ )
                        {
                            fl_imgtk::draw_smooth_line_ex( newimg,
                                                           x1, y1 + ( 10 * cnt ),
                                                           x2, y2 + ( 10 * cnt ),
                                                           2.f,
                                                           cols[cnt] );
                        }

                        y1 = 300;
                        y2 = y1 + ygap;

                        for( size_t cnt=0; cnt<6; cnt++ )
                        {
                            fl_imgtk::draw_smooth_line_ex( newimg,
                                                           x1, y1 + ( 10 * cnt ),
                                                           x2, y2 + ( 10 * cnt ),
                                                           10.f,
                                                           cols[cnt] );

                            unsigned centercol = 0x18181870;
                            fl_imgtk::draw_smooth_line( newimg,
                                                        x1, y1 + ( 10 * cnt ),
                                                        x2, y2 + ( 10 * cnt ),
                                                        centercol );

                        }

                    }
                }
                break;

            case 3:
                {
                    boxStatus->label( "Draw Polygons" );
                    newimg = (Fl_RGB_Image*)imgTestSrc->copy();

                    fl_imgtk::vecpoint star[] =
                    { { 488,  38}, { 634, 328 }, { 961, 365 }, { 724, 581}, { 776, 896 },
                      { 490, 750}, { 195, 890 }, { 253, 580 }, {  22, 365}, { 344, 322 } };

                    for( size_t cnt=0; cnt<10; cnt++ )
                    {
                        star[cnt].x *= 0.75f;
                        star[cnt].y *= 0.75f;
                    }

                    DWORD tk1 = GetTickCount();
                    fl_imgtk::draw_polygon( newimg, star, 10, 0xFFE32DCF );
                    DWORD tk2 = GetTickCount();

                    printf( "#1 performance : %u ms.\n", tk2 - tk1 );

                    for( size_t cnt=0; cnt<10; cnt++ )
                    {
                        star[cnt].x += 50;
                    }

                    tk1 = GetTickCount();
                    fl_imgtk::draw_2xaa_polygon( newimg, star, 10, 0x2DE3FFCF );
                    tk2 = GetTickCount();

                    printf( "#2 formance : %u ms.\n", tk2 - tk1 );
                }
                break;

            case 4:
                boxStatus->label( "Flip Vertical" );
                newimg = flipvertical( imgTestSrc );
                break;

            case 5:
                boxStatus->label( "Flip Horizental" );
                newimg = fliphorizontal( imgTestSrc );
                break;

            case 6:
                boxStatus->label( "Fast rotate 90 degree" );
                newimg = rotate90( imgTestSrc );
                break;

            case 7:
                boxStatus->label( "Fast rotate 180 degree" );
                newimg = rotate180( imgTestSrc );
                break;

            case 8:
                boxStatus->label( "Fast rotate 270 degree" );
                newimg = rotate270( imgTestSrc );
                break;

            case 9:
                boxStatus->label( "Free rotate 20 degree" );
                newimg = rotatefree( imgTestSrc, 20 );
                break;

            case 10:
                {
                    boxStatus->label( "Free rotate 340 degree ( w/ transparency )" );
                    Fl_RGB_Image* img4b = makeanempty( imgTestSrc->w(), imgTestSrc->h(),
                                                       4, 0x00000000 );
                    if ( img4b != NULL )
                    {
                        drawonimage( img4b, imgTestSrc );
                        printf( "img4b : %ux%ux%u\n",
                                img4b->w(), img4b->h(), img4b->d() );
                        newimg = rotatefree( img4b, 360-40 );
                        newimg->uncache();
                        printf( "rotated newimg : %ux%ux%u\n",
                                newimg->w(), newimg->h(), newimg->d() );
                        discard_user_rgb_image( img4b );
                    }
                }
                break;

            case 11:
                boxStatus->label( "Brightness 50% up" );
                newimg = brightness( imgTestSrc, 50 );
                break;

            case 12:
                boxStatus->label( "Contrast 50% up" );
                newimg = contrast( imgTestSrc, 50 );
                break;

            case 13:
                boxStatus->label( "Gamma 150%" );
                newimg = gamma( imgTestSrc, 1.5 );
                break;

            case 14:
                {
                    boxStatus->label( "built in Sharpen filter" );
                    kfconfig* filter = new_kfconfig( "sharpen" );
                    if ( filter != NULL )
                    {
                        newimg = filtered( imgTestSrc, filter );
                        discard_kfconfig( filter );
                    }
                }
                break;

            case 15:
                {
                    boxStatus->label( "built in Blur filter" );
                    kfconfig* filter = new_kfconfig( "blur" );
                    if ( filter != NULL )
                    {
                        newimg = filtered( imgTestSrc, filter );
                        discard_kfconfig( filter );
                    }
                }
                break;

            case 16:
                {
                    boxStatus->label( "built in Blur more filter" );
                    kfconfig* filter = new_kfconfig( "blurmore" );
                    if ( filter != NULL )
                    {
                        newimg = filtered( imgTestSrc, filter );
                        discard_kfconfig( filter );
                    }
                }
                break;

            case 17:
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

                    newimg = filtered( imgTestSrc, &filter );

                    delete filter.m;
                }
                break;

            case 18:
                boxStatus->label( "Render window to image" );
                newimg = draw_widgetimage( mainWindow );
                break;

            case 19:
                boxStatus->label( "Render window to blurred image" );
                newimg = drawblurred_widgetimage( mainWindow, 10 );
                break;

            case 20:
                {
                    boxStatus->label( "Crop image ( center 50% )" );

                    unsigned cw = imgTestSrc->w() / 2;
                    unsigned ch = imgTestSrc->h() / 2;
                    unsigned cx = imgTestSrc->w() - cw;
                    unsigned cy = imgTestSrc->h() - ch;

                    newimg = crop( imgTestSrc, cx, cy, cw, ch );
                }
                break;

            case 21:
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
                        amapsize = makealphamap( amap, imgTestSrc->w(), imgTestSrc->h(), 0.8f );
                    }

                    if ( amapsize > 0 )
                    {
                        newimg = applyalpha( imgTestSrc, amap, amapsize );
                    }

                    if ( amapsrc != NULL )
                    {
                        delete amapsrc;
                    }
                }
                break;

            case 22:
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

                        cfg.src2putx = ( imgTestSrc->w() - mimg->w() ) / 2;
                        cfg.src2puty = ( imgTestSrc->h() - mimg->h() ) / 2;
                        cfg.src1ratio = 1.0f;
                        cfg.src2ratio = 0.8f;

                        newimg = merge( imgTestSrc, (Fl_RGB_Image*)mimg, &cfg );

                        delete mimg;
                    }
                }
                break;

            case 23:
                {
                    boxStatus->label( "Image subtracting A(100%) + B(40%)" );

                    Fl_PNG_Image* mimg = new Fl_PNG_Image( "sherlock_tab_foler.png" );
                    if ( mimg != NULL )
                    {
                        if ( ( mimg->w() == 0 ) || ( mimg->h() == 0 ) )
                        {
                            printf(" PNG file has error !\n" );
                        }

                        int px = ( imgTestSrc->w() - mimg->w() ) / 2;
                        int py = ( imgTestSrc->h() - mimg->h() ) / 2;

                        newimg = subtract( imgTestSrc, (Fl_RGB_Image*)mimg, px, py, 0.4f );

                        delete mimg;
                    }
                }
                break;

            case 24:
                {
                    boxStatus->label( "Draw image to image with alpha." );

                    Fl_PNG_Image* mimg = new Fl_PNG_Image( "sherlock_tab_foler.png" );
                    if ( mimg != NULL )
                    {
                        if ( ( mimg->w() == 0 ) || ( mimg->h() == 0 ) )
                        {
                            printf(" PNG file has error !\n" );
                        }

                        newimg = (Fl_RGB_Image*)imgTestSrc->copy();

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

            case 25:
                boxStatus->label( "Reinhard tone mapping (HDRi)" );
                newimg = tonemapping_reinhard( imgTestSrc, 0.0f, 0.3f );
                break;

            case 26:
                boxStatus->label( "Drago tone mapping (HDRi)" );
                newimg = tonemapping_drago( imgTestSrc );
                break;

            case 27:
                boxStatus->label( "Invert" );
                newimg = invert( imgTestSrc );
                break;

            case 28:
                boxStatus->label( "Edge enhance by factor 8" );
                newimg = edgeenhance( imgTestSrc, 8 );
                break;

            case 29:
                {
                    boxStatus->label( "Color CLAHE, 128x1, clip limit = 90." );

                    unsigned rW = 128;
                    unsigned rH = 1;

                    newimg = CLAHE( imgTestSrc,
                                    rW,
                                    rH,
                                    90.0f );
                }
                break;

            case 30:
                {
                    boxStatus->label( "Noire effect ( Belong to CLAHE ), 128x1, clip limit = 90, bright = 1.5x" );

                    unsigned rW = 128;
                    unsigned rH = 1;

                    newimg = noire( imgTestSrc,
                                    rW,
                                    rH,
                                    90.1f,
                                    1.5f );
                }
                break;

        }
#endif // 1

        comRView->image( newimg );
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
