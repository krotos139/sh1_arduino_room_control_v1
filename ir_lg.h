
#ifndef ir_lg_h
#define ir_lg_h

#include "IRremote.h"
#include <Arduino.h>

class ir_lg
{
public:
    void send(unsigned long command);

    static const long pre_data      = 0x20df0000;
    static const long c_power       = 0x10EF;
    static const long c_input       = 0xD02F;
    static const long c_tv_radio    = 0x0FF0;
    static const long c_q_menu      = 0xA25D;
    static const long c_menu        = 0xC23D;
    static const long c_guide       = 0xD52A;
    static const long c_up          = 0x02FD;
    static const long c_left        = 0xE01F;
    static const long c_right       = 0x609F;
    static const long c_down        = 0x827D;
    static const long c_ok          = 0x22DD;
    static const long c_return_exit = 0x14EB;
    static const long c_info        = 0x55AA;
    static const long c_av_mode     = 0x0CF3;
    static const long c_vol_up      = 0x40BF;
    static const long c_vol_down    = 0xC03F;
    static const long c_mark_fav    = 0x7887;
    static const long c_mute        = 0x906F;
    static const long c_page_up     = 0x00FF;
    static const long c_page_down   = 0x807F;
    static const long c_1           = 0x8877;
    static const long c_2           = 0x48B7;
    static const long c_3           = 0xC837;
    static const long c_4           = 0x28D7;
    static const long c_5           = 0xA857;
    static const long c_6           = 0x6897;
    static const long c_7           = 0xE817;
    static const long c_8           = 0x18E7;
    static const long c_9           = 0x9867;
    static const long c_0           = 0x08F7;
    static const long c_list        = 0xCA35;
    static const long c_q_view      = 0x58A7;
    static const long c_red         = 0x4EB1;
    static const long c_green_stop  = 0x8E71;
    static const long c_yellow_play = 0xC639;
    static const long c_blue_pause  = 0x8679;
    static const long c_text        = 0x04FB;
    static const long c_t_opt_rev   = 0x847B;
    static const long c_subtitle_ffw = 0x9C63;
    static const long c_rec         = 0xBD42;
    static const long c_simplink    = 0x7E81;
    
    
private:
    IRsend irsend;

};
#endif
//
// END OF FILE
//
