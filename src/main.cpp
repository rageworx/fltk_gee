#include "wMain.h"

#include <FL/Fl.H>
#include <FL/Fl_Tooltip.H>
#include <FL/fl_ask.H>

#if ( FL_ABI_VERSION < 10304 )
    #error "Error, FLTK ABI need 1.3.3 or above"
#endif

////////////////////////////////////////////////////////////////////////////////

#define DEF_APP_CLSNAME         "rkrawvSTDO"

////////////////////////////////////////////////////////////////////////////////

void procAutoLocale()
{
    LANGID currentUIL = GetSystemDefaultLangID();

#ifdef DEBUG
    printf("current LANG ID = %08X ( %d )\n", currentUIL, currentUIL );
#endif // DEBUG

    const char* convLoc = NULL;

    switch( currentUIL & 0xFF )
    {
        case LANG_KOREAN:
            convLoc = "korean";
            break;

        case LANG_JAPANESE:
            convLoc = "japanese";
            break;

        case LANG_CHINESE:
            convLoc = "chinese";
            break;

        default:
            convLoc = "C";
            break;
    }

    setlocale( LC_ALL, convLoc );
}

void presetFLTKenv()
{
    Fl::set_font( FL_FREE_FONT, "Tahoma" );
    Fl_Double_Window::default_xclass( DEF_APP_CLSNAME );

    fl_message_font_ = FL_FREE_FONT;
    fl_message_size_ = 11;

    Fl_Tooltip::color( fl_darker( FL_DARK3 ) );
    Fl_Tooltip::textcolor( FL_WHITE );
    Fl_Tooltip::size( 11 );
    Fl_Tooltip::font( FL_FREE_FONT );

    Fl::scheme( "flat" );
}

int main (int argc, char ** argv)
{
    int reti = 0;

    presetFLTKenv();

    wMain* pWMain = new wMain( argc, argv );

    if ( pWMain != NULL )
    {
        procAutoLocale();

        reti = pWMain->Run();
    }

    return reti;
}
