#include <sys/time.h>
#include <cmath>

#include <FL/Fl_Copy_Surface.H>
#include <FL/fl_draw.H>
#include <fl_imgtk.h>
#include "Fl_ImageViewer.h"

////////////////////////////////////////////////////////////////////////////////

#define MINIMAP_MAX_DISTANCE    150
#define MOVING_AMOUNT_DIVIDER   10
#define COLOR_NAVIBAR           0x9999FF00

////////////////////////////////////////////////////////////////////////////////

Fl_ImageViewer::Fl_ImageViewer(int x,int y,int w,int h)
 : Fl_Box( x, y, w, h ),
   multiplier( 1.0f ),
   imgsrc( NULL ),
   imgcached( NULL ),
   _notifier( NULL ),
   drawnavigator( false ),
   resize_type( 0 ),
   fittype( 1 ),
   naviMin( 0 ),
   naviMax( 1 ),
   naviCur( 0 ),
   naviTimestamp( 0 ),
   naviTimecheck( 0 ),
   naviBarCol( COLOR_NAVIBAR ),
   isdrawing( false ),
   isgeneratingimg( false )
{
    Fl_Box::box( FL_NO_BOX );
    color( FL_BLACK );
    type( 0 );

    labelcolor( FL_WHITE );

    naviTimecheck = gettimenow();
    naviTimestamp = naviTimecheck;
}

Fl_ImageViewer::~Fl_ImageViewer()
{
    unloadimage();
}

void Fl_ImageViewer::image(Fl_RGB_Image* aimg, int fittingtype)
{
    if( aimg == NULL )
        return;

    unloadimage();

    imgsrc = (Fl_RGB_Image*)aimg->copy( aimg->w(), aimg->h() );

    if ( imgsrc != NULL )
    {
        label( NULL );

        switch( fittingtype )
        {
            default:
            case 0:
                fittype = 0;
                fitwidth();
                break;

            case 1:
                fittype = 1;
                fitheight();
                break;

        }
    }
}

void Fl_ImageViewer::unloadimage()
{
    if( imgcached != NULL )
    {
        if ( ( imgcached->array != NULL ) && ( imgcached->alloc_array == 0 ) )
        {
            delete[] imgcached->array;
        }

        delete imgcached;
        imgcached = NULL;
    }

    if ( imgsrc != NULL )
    {
        if ( ( imgsrc->array != NULL ) && ( imgsrc->alloc_array == 0 ) )
        {
            delete[] imgsrc->array;
        }

        delete imgsrc;
        imgsrc = NULL;
    }
}

void Fl_ImageViewer::multiplyratio( float rf )
{
    if ( imgsrc == NULL )
        return;

    if ( isgeneratingimg == true )
        return;

    isgeneratingimg = true;

    bool  recalc_center = false;
    float rc_x = 0.0f;
    float rc_y = 0.0f;

    fl_imgtk::discard_user_rgb_image( imgcached );

    multiplier   = rf;
    float f_w    = imgsrc->w() * multiplier;
    float f_h    = imgsrc->h() * multiplier;

    switch( resize_type )
    {
        default:
        case 0 : /// Normal (near)
            imgcached = (Fl_RGB_Image*)imgsrc->copy( f_w, f_h );
            break;

        case 1 : /// BiLinear
            {
                imgcached = fl_imgtk::rescale( (Fl_RGB_Image*)imgsrc, f_w, f_h,
                                               fl_imgtk::BILINEAR );
            }
            break;

        case 2 : /// BiCubic
            {
                imgcached = fl_imgtk::rescale( (Fl_RGB_Image*)imgsrc, f_w, f_h,
                                               fl_imgtk::BICUBIC );
            }
            break;

        case 3 : /// Lanczos3
            {
                imgcached = fl_imgtk::rescale( (Fl_RGB_Image*)imgsrc, f_w, f_h,
                                               fl_imgtk::LANCZOS );
            }
            break;

    }

    isgeneratingimg = false;
}

unsigned long long Fl_ImageViewer::gettimenow()
{
    timeval tv;
    gettimeofday(&tv, NULL);
    return (unsigned long long)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void Fl_ImageViewer::fitwidth()
{
    if ( imgsrc == NULL )
        return;

    float f_w = float( w() ) / float( imgsrc->w() );

    multiplyratio( f_w );
}

void Fl_ImageViewer::fitheight()
{
    if ( imgsrc == NULL )
        return;

    float f_h = float( h() ) / float( imgsrc->h() );

    multiplyratio( f_h );
}

int Fl_ImageViewer::imgw()
{
    if ( imgcached != NULL )
    {
        return imgcached->w();
    }

    return 0;
}

int Fl_ImageViewer::imgh()
{
    if ( imgcached != NULL )
    {
        return imgcached->h();
    }

    return 0;
}

void Fl_ImageViewer::range( unsigned minv, unsigned maxv )
{
    if ( maxv > minv )
    {
        naviMin = minv;
        naviMax = maxv;

        if ( naviCur > naviMax )
        {
            naviCur = naviMax;
        }

        drawnavigator = true;
    }
    else
    {
        drawnavigator = false;
    }
}

void Fl_ImageViewer::position( unsigned v )
{
    if ( naviMax >= v )
    {
        naviCur = v;
    }
    else
    {
        naviCur = naviMax;
    }
}

int Fl_ImageViewer::handle( int event )
{
    int ret = Fl_Box::handle( event );

    static int  mouse_btn;
    static int  shift_key;
    static int  check_x;
    static int  check_y;

    switch( event )
    {
        case FL_PUSH:
            take_focus();
            shift_key = Fl::event_shift();
            mouse_btn = Fl::event_button();
            return 1;

        case FL_MOVE:
            check_x = Fl::event_x();
            check_y = Fl::event_y();
            return 1;

        case FL_RELEASE:
            shift_key = Fl::event_shift();
            mouse_btn = Fl::event_button();

            if ( mouse_btn == FL_RIGHT_MOUSE )
            {

                if ( imgsrc != NULL )
                {
                    check_x = Fl::event_x() - x();
                    check_y = Fl::event_y() - y();

                    if ( check_x < 0 )
                    {
                        check_x = 0;
                    }

                    if ( check_y < 0 )
                    {
                        check_y = 0;
                    }

                    if ( _notifier != NULL )
                    {
                        _notifier->OnRightClick( check_x, check_y );
                    }
                }
            }
            return 1;

        case FL_MOUSEWHEEL:
            if ( _notifier != NULL )
            {
                _notifier->OnKeyPressed( FL_MOUSEWHEEL,
                                         Fl::event_dx(),
                                         Fl::event_dy(),
                                         0 );
            }
            break;

        case FL_FOCUS:
        case FL_UNFOCUS:
            return 1;

        //case FL_KEYUP:
        case FL_KEYDOWN:
            {
                bool bredraw = false;

                int kbda = Fl::event_alt();
                int kbdc = Fl::event_ctrl();
                int kbds = Fl::event_shift();
                int kbdk = Fl::event_key();

                if ( _notifier != NULL )
                {
                    unsigned short kb = kbdk & 0xFFFF;

                    _notifier->OnKeyPressed( kb, kbds, kbdc, kbda );
                }

                ret = 1;
            }
            break;
    }

    return ret;
}

void Fl_ImageViewer::draw()
{
    if ( isdrawing == true )
        return;

    isdrawing = true;

    fl_push_clip( x(), y(), w(), h() );

    Fl_Color prevC = fl_color();

    // Clear Background.
    fl_color( color() );
    fl_rectf( x(), y(), w(), h() );

    if ( ( imgcached != NULL ) && ( isgeneratingimg == false ) )
    {
        int put_x = x() + ( w() - imgcached->w() ) / 2;
        int put_y = y() + ( h() - imgcached->h() ) / 2;

        imgcached->draw( put_x, put_y );
    }
    else
    {
        fl_font( FL_COURIER, 12 );
        fl_color( FL_RED );

        char outstr[128] = {0};

        sprintf( outstr,
                 "ERROR: No image cached, imgcached = %X, %d",
                 imgcached,
                 isgeneratingimg );

        fl_draw( outstr, 0, 30 );
    }

    // Draw Navigator bar
    if ( drawnavigator == true )
    {
        int nav_amount = naviMax - naviMin;
        int nav_cpos   = naviCur - naviMin;

        int nav_w_m = w() / 256;
        int nav_w   = w() / ( nav_amount + 1 );
        int nav_h   = h() / 256;

        if ( nav_w < 2 )
            nav_w = 2;

        if ( nav_h < 2 )
            nav_h = 2;

        int nav_x = x() + ( nav_w * nav_cpos ) + ( nav_w_m / 2 );
        int nav_y = y() + ( h() - ( h() / 100 ) );

        fl_color( FL_BLACK );
        fl_rectf( nav_x-1, nav_y-1, nav_w+2, nav_h+2 );
        fl_color( naviBarCol );
        fl_rectf( nav_x, nav_y, nav_w, nav_h );
        fl_font( FL_COURIER, 11 );

        static char posinfostr[32] = {0};
        sprintf( posinfostr, "%d/%d", naviCur, naviMax );

        int put_x = nav_x;
        int put_y = nav_y - nav_h;

        fl_color( FL_BLACK );
        fl_draw( posinfostr, put_x-1, put_y-1 );
        fl_draw( posinfostr, put_x, put_y-1 );
        fl_draw( posinfostr, put_x+1, put_y-1 );
        fl_draw( posinfostr, put_x-1, put_y );
        fl_draw( posinfostr, put_x+1, put_y );
        fl_draw( posinfostr, put_x-1, put_y+1 );
        fl_draw( posinfostr, put_x, put_y+1 );
        fl_draw( posinfostr, put_x+1, put_y+1 );
        fl_color( naviBarCol );
        fl_draw( posinfostr, put_x, put_y );
    }

    // Draw itself rectangle.
    Fl_Box::draw();

    fl_color( prevC );

    fl_pop_clip();

    if ( _notifier != NULL )
    {
        _notifier->OnDrawCompleted();
    }

    isdrawing = false;
}

void Fl_ImageViewer::resize(int x,int y,int w,int h)
{
    Fl_Box::resize( x, y, w, h );

    switch( fittype )
    {
        case 0:
            fitwidth();
            break;

        case 1:
            fitheight();
            break;
    }

    redraw();
}

void Fl_ImageViewer::box(Fl_Boxtype new_box)
{
    // It always ignore user custom box type.
    //Fl_Scroll::box( box() );
    //Fl_Box::box( FL_FLAT_BOX );
}

void Fl_ImageViewer::type(uchar t)
{
    // It always ignore user type = NO scroll bars.
    // Fl_Scroll::type( type() );
    Fl_Box::type( 0 );
}

void Fl_ImageViewer::resizemethod( int t, bool autoapply )
{
    if ( t <= 3 )
    {
        if ( resize_type != t )
        {
            resize_type = t;

            if ( autoapply == true )
            {
                multiplyratio( multiplier );
            }
        }
    }
}

