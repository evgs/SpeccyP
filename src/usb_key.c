/*

mouse[0] какие устройства подключены по USB 
mouse[1] мышь кнопки 
mouse[2] мышь X
mouse[3] мышь Y


*/
#include "tusb.h"
#include "usb_key.h"
#include "joy.h"
#include "kb_u_codes.h"
#include "ps2.h"
//#include "I2C_rp2040.h"
#include "g_config.h"
#include "config.h"  

#include "hardware/gpio.h"

#include "xinput_host.h"

#include <assert.h>
// usb устройства
//uint8_t usb_device;// 1 клавиатура 2 мышь 3 клавиатура+мышь

static uint8_t mouse_buttons = 0xFF;   /* Все кнопки отпущены (активный низкий уровень) */
static int16_t mouse_x = 420;           /* Накопление координаты X */  
static int16_t mouse_y = 1070;          /* Накопление координаты Y */
static int8_t mouse_wheel = 0;         /* Значение колеса прокрутки */


void convert_kempstonMouse() {
    /* Запись в выходной буфер (обрезание до 8 бит) */
    mouse[1] = mouse_buttons;          /* Состояние кнопок */
    mouse[2] = (uint8_t)(mouse_x / 10);     /* Младший байт X */
    mouse[3] = (uint8_t)(mouse_y / 10);     /* Младший байт Y */
}

///////////////////////////////////////////////////////////////////////////////////////////
/* ============================================================================================= */
/* ============                        XBOX controllers                          =============== */
/* ============================================================================================= */

//================================================================================================
//Since https://github.com/hathach/tinyusb/pull/2222, we can add in custom vendor drivers easily
usbh_class_driver_t const* usbh_app_driver_get_cb(uint8_t* driver_count){
    *driver_count = 1;
    return &usbh_xinput_driver;
}
//===============================================
void tuh_xinput_report_received_cb(uint8_t dev_addr, uint8_t instance, xinputh_interface_t const* xid_itf, uint16_t len)
//void tuh_xinput_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) 
{
  //  auto xid_itf = (xinputh_interface_t *)report;
    const xinput_gamepad_t* p = &xid_itf->pad;
          joy_k = 0x00;//#1F - кемпстон джойстик 0001 1111
  //  if (p->wButtons & XINPUT_GAMEPAD_A)   joy_k |=  0x20  ;//0x1000;// A; 0001.0000 00x0.0000
  //  if (p->wButtons & XINPUT_GAMEPAD_B)  joy_k |= 0x10; //(1 << 5);//0x2000");// B 0x0010 0000 000x 0000

//gamepad1_bits.a = p->wButtons & XINPUT_GAMEPAD_A;

    const uint16_t dpad = p->wButtons ;//& 0x0f;
     if (!dpad) {
      //  gamepad1_bits.up = p->sThumbLY > 0 || p->sThumbRY > 0;
      //  gamepad1_bits.down = p->sThumbLY < 0 || p->sThumbRY < 0;

      //  gamepad1_bits.right = p->sThumbLX > 0 || p->sThumbRX > 0;
      //  gamepad1_bits.left = p->sThumbLX < 0 || p->sThumbRX < 0;
    }
    else {
     //   if (dpad == XINPUT_GAMEPAD_DPAD_UP ) joy_k |=(1 << 3); ;//0x0001
     //   if (dpad == XINPUT_GAMEPAD_DPAD_DOWN ) joy_k |=(1 << 2); ;//0x0002
    //    if (dpad == XINPUT_GAMEPAD_DPAD_LEFT ) joy_k |=(1 << 1); ;//0x0004
    //    if (dpad == XINPUT_GAMEPAD_DPAD_RIGHT ) joy_k |=(1 << 0); ;//0x0008
//if (dpad == 9) joy_k |=(1 << 3)|(1 << 0) ;
//if (dpad == 9) joy_k |=(1 << 3)|(1 << 0) ;

joy_k |= ((dpad&1)<<3) | ((dpad&2)<<1) | ((dpad&4)>>1) | ((dpad&8)>>3) | ((dpad& 0x1000)>>7) | ((dpad& 0x2000)>>9);






      //  gamepad1_biXINPUT_GAMEPAD_DPAD_RIGHTts.up = dpad & XINPUT_GAMEPAD_DPAD_UP;
      //  gamepad1_bits.left = dpad & XINPUT_GAMEPAD_DPAD_LEFT;
      //  gamepad1_bits.right = dpad & XINPUT_GAMEPAD_DPAD_RIGHT;
    }
/*
    if (p->wButtons & XINPUT_GAMEPAD_GUIDE) {
        gamepad1_bits.start = true;
        gamepad1_bits.select = true;
    }
    else {
        gamepad1_bits.start = p->wButtons & XINPUT_GAMEPAD_START;
        gamepad1_bits.select = p->wButtons & XINPUT_GAMEPAD_BACK;
    }
    const uint8_t dpad = p->wButtons & 0xf;

    if (!dpad) {
        gamepad1_bits.up = p->sThumbLY > 0 || p->sThumbRY > 0;
        gamepad1_bits.down = p->sThumbLY < 0 || p->sThumbRY < 0;

        gamepad1_bits.right = p->sThumbLX > 0 || p->sThumbRX > 0;
        gamepad1_bits.left = p->sThumbLX < 0 || p->sThumbRX < 0;
    }
    else {
        gamepad1_bits.down = dpad & XINPUT_GAMEPAD_DPAD_DOWN;
        gamepad1_bits.up = dpad & XINPUT_GAMEPAD_DPAD_UP;
        gamepad1_bits.left = dpad & XINPUT_GAMEPAD_DPAD_LEFT;
        gamepad1_bits.right = dpad & XINPUT_GAMEPAD_DPAD_RIGHT;
    }
*/
    /*char tmp[128];
    sprintf(tmp, "[%02x, %02x], Type: %s, Buttons %04x, LT: %02x RT: %02x, LX: %d, LY: %d, RX: %d, RY: %d\n",
                 dev_addr, instance, type_str, p->wButtons, p->bLeftTrigger, p->bRightTrigger, p->sThumbLX, p->sThumbLY, p->sThumbRX, p->sThumbRY);
    draw_text(tmp, 0,0, 15,0);*/

    tuh_xinput_receive_report(dev_addr, instance);
}
//==============================
/*
case 0:  joy_k |= (1 << 3); break; //3 бит  up
      case 1:  joy_k |= (1 << 3); joy_k |= (1 << 0); break; //3 бит up  0 бит  right  up right
      case 2:  joy_k |= (1 << 0); break; // 0 бит  right
      case 3:  joy_k |= (1 << 2); joy_k |= (1 << 0); break; //2 бит down  0 бит  right  down right
      case 4:  joy_k |= (1 << 2); break; //2 бит down  
      case 5:  joy_k |= (1 << 2); joy_k |= (1 << 1); break; //2 бит down  1 бит  left  down left
      case 6:  joy_k |= (1 << 1); break; // 1 бит  left
      case 7:  joy_k |= (1 << 3); joy_k |= (1 << 1); break; //3 бит up  1 бит  left  up left


*/


//===============================================
/* 
void tuh_xinput_report_received_cb(uint8_t dev_addr, uint8_t instance, xinputh_interface_t const* xid_itf, uint16_t len)
{
    if (len==0) return;
    const xinput_gamepad_t *p = &xid_itf->pad;
    const char* type_str;

    if (xid_itf->last_xfer_result == XFER_RESULT_SUCCESS)
    {
        switch (xid_itf->type)
        {
            case 1: type_str = "Xbox One";          break;
            case 2: type_str = "Xbox 360 Wireless"; break;
            case 3: type_str = "Xbox 360 Wired";    break;
            case 4: type_str = "Xbox OG";           break;
            default: type_str = "Unknown";
        }

        if (xid_itf->connected && xid_itf->new_pad_data)
        {
      //      debug_print("[%02x, %02x], Type: %s, Buttons %04x, LT: %02x RT: %02x, LX: %d, LY: %d, RX: %d, RY: %d\n",
      //          dev_addr, instance, type_str, p->wButtons, p->bLeftTrigger, p->bRightTrigger, p->sThumbLX, p->sThumbLY, p->sThumbRX, p->sThumbRY);

            //How to check specific buttons
        //    if (p->wButtons & XINPUT_GAMEPAD_A) debug_print("You are pressing A\n");
        }
    }
    tuh_xinput_receive_report(dev_addr, instance);  
}*/

void tuh_xinput_mount_cb(uint8_t dev_addr, uint8_t instance, const xinputh_interface_t *xinput_itf)
{
 //   debug_print("XINPUT MOUNTED %02x %d\n", dev_addr, instance);
    // If this is a Xbox 360 Wireless controller we need to wait for a connection packet
    // on the in pipe before setting LEDs etc. So just start getting data until a controller is connected.
    if (xinput_itf->type == XBOX360_WIRELESS && xinput_itf->connected == false)
    {
        tuh_xinput_receive_report(dev_addr, instance);
        return;
    }

      msg_bar=8;
      wait_msg = 5000; 

    tuh_xinput_set_led(dev_addr, instance, 0, true);
    tuh_xinput_set_led(dev_addr, instance, 1, true);
    tuh_xinput_set_rumble(dev_addr, instance, 0, 0, true);
    tuh_xinput_receive_report(dev_addr, instance);
}

void tuh_xinput_umount_cb(uint8_t dev_addr, uint8_t instance)
{
 //   debug_print("XINPUT UNMOUNTED %02x %d\n", dev_addr, instance);
}

///////////////////////////////////////////////////////////////////////////////////////////



//----------------------------------------------

//================================================
bool decode_key(bool menu_mode) 
{
    tuh_task(); // Обновляем состояние USB хоста

    // Обработка событий только если есть активность клавиатуры
    if(!flag_usb_kb) return false;
    // Режим меню - проверяем специфические клавиши
    if(menu_mode)
    {
        // Проверяем биты направления (UP/DOWN)
         bool direction_key = kb_st_ps2.u[2] & (KB_U2_DOWN | KB_U2_UP | KB_U2_INSERT  );
        
        if(direction_key) 
        {
            sleep_ms(DELAY_KEY); // Задержка для антидребезга
            return true;        // Событие направления обработано
        }
        
        direction_key = kb_st_ps2.u[1] & ( KB_U1_ENTER | KB_U1_BACK_SPACE | KB_U1_ESC );
        if(direction_key) 
        {
 
          sleep_ms(DELAY_KEY); // Задержка для антидребезга
          flag_usb_kb = false;    // 
          return true;        // Событие  обработано
        }

    } 
      bool direction_key = kb_st_ps2.u[3] & ( KB_U3_F7| KB_U3_F8 );
        if( direction_key) 
        {
          sleep_ms(DELAY_KEY); // Задержка для антидребезга
        //  flag_usb_kb = false;    // 
          return true;        // Событие  обработано
        }

    // Обычный режим - любое нажатие
    flag_usb_kb = false; // Сбрасываем флаг после обработки
    return true;
}

//==============================================================
bool decode_key_joy()
{
  
 decode_joy_to_keyboard();
 if ((decode_PS2()) | (decode_key(true))) return true;
    return true;
}
//==============================================================
uint8_t keyboard_addr = 0;
uint8_t keyboard_instance = 0;
bool auto_toggle_start = false;
 uint32_t successful_led_toggles = 0;
 uint8_t leds = 7;
 bool is_on = false;
 bool set_report_failed = false;
 //----------------------------------------------------------
void __not_in_flash_func(scancode_usb_s)(uint8_t code){

  if (code & 0x02) kb_st_ps2.u[1]|=KB_U1_L_SHIFT;// left shift  
 //  if (code & 0x02) kb_st_ps2.u[1]|=KB_U1_R_SHIFT;// left shift  переназначение на правый
   if (code & 0x20) kb_st_ps2.u[1]|=KB_U1_R_SHIFT;// righr shift 

   
   if (code & 0x04) kb_st_ps2.u[1]|=KB_U1_L_ALT;  // left alt     0000 0100
   if (code & 0x40) kb_st_ps2.u[1]|=KB_U1_R_ALT;; // right alt   0100 0000
   
   if (code & 0x01) kb_st_ps2.u[1]|=KB_U1_L_CTRL; // right ctrl   
   if (code & 0x10) kb_st_ps2.u[1]|=KB_U1_R_CTRL;
   
   if (code & 0x08) kb_st_ps2.u[1]|=KB_U1_L_WIN;; // win  l
   if (code & 0x80) kb_st_ps2.u[1]|=KB_U1_L_WIN;; // win r
   }
//----------------------------------------------------------

/**  Code size 487924/383300 vs 546500/387376 before optimization
 *  initial  546500/387376 
 *  finally (ps/2+usb) 542320/387376
 */
#define BITINDEX(BINN, BITNUM) ((BINN)<<6 | (BITNUM))

const static __in_flash() uint8_t usbScanMap[] = {
    /* 0x00 */  0xff,
    /* 0x01 */  0xff,
    /* 0x02 */  0xff,
    /* 0x03 */  0xff,
    /* 0x04 */  BITINDEX(0, KB_U0_A_POS ),
    /* 0x05 */  BITINDEX(0, KB_U0_B_POS ),
    /* 0x06 */  BITINDEX(0, KB_U0_C_POS ),
    /* 0x07 */  BITINDEX(0, KB_U0_D_POS ),

    /* 0x08 */  BITINDEX(0, KB_U0_E_POS ),
    /* 0x09 */  BITINDEX(0, KB_U0_F_POS ),
    /* 0x0a */  BITINDEX(0, KB_U0_G_POS ),
    /* 0x0b */  BITINDEX(0, KB_U0_H_POS ),
    /* 0x0c */  BITINDEX(0, KB_U0_I_POS ),
    /* 0x0d */  BITINDEX(0, KB_U0_J_POS ),
    /* 0x0e */  BITINDEX(0, KB_U0_K_POS ),
    /* 0x0f */  BITINDEX(0, KB_U0_L_POS ),

    /* 0x10 */  BITINDEX(0, KB_U0_M_POS ),
    /* 0x11 */  BITINDEX(0, KB_U0_N_POS ),
    /* 0x12 */  BITINDEX(0, KB_U0_O_POS ),
    /* 0x13 */  BITINDEX(0, KB_U0_P_POS ),
    /* 0x14 */  BITINDEX(0, KB_U0_Q_POS ),
    /* 0x15 */  BITINDEX(0, KB_U0_R_POS ),
    /* 0x16 */  BITINDEX(0, KB_U0_S_POS ),
    /* 0x17 */  BITINDEX(0, KB_U0_T_POS ),

    /* 0x18 */  BITINDEX(0, KB_U0_U_POS ),
    /* 0x19 */  BITINDEX(0, KB_U0_V_POS ),
    /* 0x1a */  BITINDEX(0, KB_U0_W_POS ),
    /* 0x1b */  BITINDEX(0, KB_U0_X_POS ),
    /* 0x1c */  BITINDEX(0, KB_U0_Y_POS ),
    /* 0x1d */  BITINDEX(0, KB_U0_Z_POS ),
    /* 0x1e */  BITINDEX(1, KB_U1_1_POS ),
    /* 0x1f */  BITINDEX(1, KB_U1_2_POS ),

    /* 0x20 */  BITINDEX(1, KB_U1_3_POS ),
    /* 0x21 */  BITINDEX(1, KB_U1_4_POS ),
    /* 0x22 */  BITINDEX(1, KB_U1_5_POS ),
    /* 0x23 */  BITINDEX(1, KB_U1_6_POS ),
    /* 0x24 */  BITINDEX(1, KB_U1_7_POS ),
    /* 0x25 */  BITINDEX(1, KB_U1_8_POS ),
    /* 0x26 */  BITINDEX(1, KB_U1_9_POS ),
    /* 0x27 */  BITINDEX(1, KB_U1_0_POS ),

    /* 0x28 */  BITINDEX(1, KB_U1_ENTER_POS ),
    /* 0x29 */  BITINDEX(1, KB_U1_ESC_POS),
    /* 0x2a */  BITINDEX(1, KB_U1_BACK_SPACE_POS ),
    /* 0x2b */  BITINDEX(1, KB_U1_TAB_POS ),
    /* 0x2c */  BITINDEX(1, KB_U1_SPACE_POS ),
    /* 0x2d */  BITINDEX(1, KB_U1_MINUS_POS ),
    /* 0x2e */  BITINDEX(1, KB_U1_EQUALS_POS ),
    /* 0x2f */  BITINDEX(0, KB_U0_LEFT_BR_POS ),

    /* 0x30 */  BITINDEX(0, KB_U0_RIGHT_BR_POS ),
    /* 0x31 */  BITINDEX(1, KB_U1_BACKSLASH_POS),
    /* 0x32 */  BITINDEX(1, KB_U1_TILDE_POS ),  //KEY_HASHTILDE #~ (ISO Layout)
    /* 0x33 */  BITINDEX(0, KB_U0_SEMICOLON_POS ),
    /* 0x34 */  BITINDEX(0, KB_U0_QUOTE_POS ),
    /* 0x35 */  BITINDEX(1, KB_U1_TILDE_POS ),  //KEY_GRAVE `~ (ANSI Layout)
    /* 0x36 */  BITINDEX(0, KB_U0_COMMA_POS ),
    /* 0x37 */  BITINDEX(0, KB_U0_PERIOD_POS ),

    /* 0x38 */  BITINDEX(1, KB_U1_SLASH_POS ),
    /* 0x39 */  BITINDEX(1, KB_U1_CAPS_LOCK_POS ),
    /* 0x3a */  BITINDEX(3, KB_U3_F1_POS ),
    /* 0x3b */  BITINDEX(3, KB_U3_F2_POS ),
    /* 0x3c */  BITINDEX(3, KB_U3_F3_POS ),
    /* 0x3d */  BITINDEX(3, KB_U3_F4_POS ),
    /* 0x3e */  BITINDEX(3, KB_U3_F5_POS ),
    /* 0x3f */  BITINDEX(3, KB_U3_F6_POS ),

    /* 0x40 */  BITINDEX(3, KB_U3_F7_POS ),
    /* 0x41 */  BITINDEX(3, KB_U3_F8_POS ),
    /* 0x42 */  BITINDEX(3, KB_U3_F9_POS ),
    /* 0x43 */  BITINDEX(3, KB_U3_F10_POS ),
    /* 0x44 */  BITINDEX(3, KB_U3_F11_POS ),
    /* 0x45 */  BITINDEX(3, KB_U3_F12_POS ),
    /* 0x46 */  BITINDEX(2, KB_U2_PRT_SCR_POS ),
    /* 0x47 */  BITINDEX(2, KB_U2_SCROLL_LOCK_POS),

    /* 0x48 */  BITINDEX(2, KB_U2_PAUSE_BREAK_POS ),
    /* 0x49 */  BITINDEX(2, KB_U2_INSERT_POS),
    /* 0x4a */  BITINDEX(2, KB_U2_HOME_POS ),
    /* 0x4b */  BITINDEX(2, KB_U2_PAGE_UP_POS ),
    /* 0x4c */  BITINDEX(2, KB_U2_DELETE_POS ),
    /* 0x4d */  BITINDEX(2, KB_U2_END_POS ),
    /* 0x4e */  BITINDEX(2, KB_U2_PAGE_DOWN_POS),
    /* 0x4f */  BITINDEX(2, KB_U2_RIGHT_POS ),

    /* 0x50 */  BITINDEX(2, KB_U2_LEFT_POS ),
    /* 0x51 */  BITINDEX(2, KB_U2_DOWN_POS ),
    /* 0x52 */  BITINDEX(2, KB_U2_UP_POS ),
    /* 0x53 */  BITINDEX(2, KB_U2_NUM_LOCK_POS ),
    /* 0x54 */  BITINDEX(2, KB_U2_NUM_SLASH_POS ),
    /* 0x55 */  BITINDEX(2, KB_U2_NUM_MULT_POS ),
    /* 0x56 */  BITINDEX(2, KB_U2_NUM_MINUS_POS ),
    /* 0x57 */  BITINDEX(2, KB_U2_NUM_PLUS_POS ),

    /* 0x58 */  BITINDEX(2, KB_U2_NUM_ENTER_POS ),
    /* 0x59 */  BITINDEX(2, KB_U2_NUM_1_POS ),
    /* 0x5a */  BITINDEX(2, KB_U2_NUM_2_POS ),
    /* 0x5b */  BITINDEX(2, KB_U2_NUM_3_POS ),
    /* 0x5c */  BITINDEX(2, KB_U2_NUM_4_POS ),
    /* 0x5d */  BITINDEX(2, KB_U2_NUM_5_POS ),
    /* 0x5e */  BITINDEX(2, KB_U2_NUM_6_POS ),
    /* 0x5f */  BITINDEX(2, KB_U2_NUM_7_POS ),

    /* 0x60 */  BITINDEX(2, KB_U2_NUM_8_POS ),
    /* 0x61 */  BITINDEX(2, KB_U2_NUM_9_POS ),
    /* 0x62 */  BITINDEX(2, KB_U2_NUM_0_POS ),
    /* 0x63 */  BITINDEX(2, KB_U2_NUM_PERIOD_POS ),
    /* 0x64 */  0xff,
    /* 0x65 */  BITINDEX(1, KB_U1_MENU_POS),

};
#define usbScanMapSize (sizeof(usbScanMap)/sizeof(uint8_t))
static_assert(usbScanMapSize == 0x66, "Wrong array size!");


void scancode_usb(uint8_t code){
    if (code == 0) return;
    if (code >= usbScanMapSize) return;
    uint8_t bitindex = usbScanMap[code];
    if (bitindex == 0xff) return;
    int bin = bitindex >> 6;
    int keybit = bitindex & 31;
    kb_st_ps2.u[bin] |= 1<<keybit;  
 }
//----------------------------------------------------------------------------
void __not_in_flash_func(keyboard_report) (uint8_t const *report, uint16_t len)
{

 if (report[0]&&report[2]&&report[3]&&report[4]&&report[5]&&report[6]&&report[7]==0) 
  { 
    flag_usb_kb = false;
    return;
  }

    flag_usb_kb = true;   // клавиша клавиатуры usb была нажата / есть событие usb клавиатуры
    kb_st_ps2.u[0] = 0;
    kb_st_ps2.u[1] = 0;
    kb_st_ps2.u[2] = 0;
    kb_st_ps2.u[3] = 0;

    scancode_usb_s(report[0]); // Ctrl, Shift, Alt, Win   /**< Keyboard modifier (KEYBOARD_MODIFIER_* masks). */
   
  //  scancode_usb(key_report[1]);/**< Reserved for OEM use, always set to 0. */

    /**< Key codes of the currently pressed keys. */

    scancode_usb(report[2]);
    
    scancode_usb(report[3]);
   
    scancode_usb(report[4]);
    
    scancode_usb(report[5]);

    scancode_usb(report[6]);
 
    scancode_usb(report[7]);
    
//if ((kb_st_ps2.u[0]=0)&(kb_st_ps2.u[1]=0)&(kb_st_ps2.u[2]=0)&(kb_st_ps2.u[3]=0)) flag_usb_kb = true;
 //  else flag_usb_kb = false;   // клавиша клавиатуры usb была нажата / есть событие usb клавиатуры
   
}

//-------------------------------------------------------------
// USB vid pid 
//-------------------------------------------------------------
void print_lsusb(void) {
  bool no_device = true;
//   gamepad_hid=0;
  for ( uint8_t daddr = 1; daddr < CFG_TUH_DEVICE_MAX+1; daddr++ ) {

    if ( tuh_mounted(daddr-1)) {
  
tuh_vid_pid_get(daddr, &vid, &pid);
// debug_print("Device %u: VID %04X PID %04X\r\n", daddr,vid, pid);




  //  if (vid==0x20bc && pid==0x5500) 
    
  //  gamepad_hid=1;

    //  msg_bar=6;
    //  wait_msg = 5000; 
    //  no_device = false;
    }
  }

  if (no_device) {
   //  debug_print("No device connected (except hub)");
   return;
    }


}
//--------------------------------------------------------------------
// Вызывается, когда устройство с интерфейсом hid отключено
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance)
{
  usb_device = 0;   
  print_lsusb(); // print device summary
}


//--------------------------------------------------------------------
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len)
{
  
  (void)desc_report;
  (void)desc_len;

  //const char* protocol_str[] = { "None", "Keyboard", "Mouse" };
  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

  if (itf_protocol == HID_ITF_PROTOCOL_KEYBOARD )
  {
    if ( !tuh_hid_receive_report(dev_addr, instance) )
    {
      // Ошибка: не удается запросить отчет
     // keyboard_mounted = false;
    }
    else
    {
       // led_blink(2);
      //  LED_ON();
        keyboard_addr = dev_addr;
        keyboard_instance = instance;
       // keyboard_mounted = true;
        usb_device = usb_device | USB_DEVICE_KEYBOARD;

    }
  }
  
 if (itf_protocol == HID_ITF_PROTOCOL_MOUSE )
  {
    if ( !tuh_hid_receive_report(dev_addr, instance) )
    {
      // Ошибка: не удается запросить отчет
    //  keyboard_mounted = false;
    }
    else
    {

/*         keyboard_addr = dev_addr;
        keyboard_instance = instance;
        keyboard_mounted = true; */
        usb_device = usb_device | USB_DEVICE_MOUSE;
        convert_kempstonMouse();    //do cold load kempston mouse ports

    }
  }


   //   debug_print("HID Interface Protocol = other\n");
     tuh_hid_receive_report(dev_addr, instance);// !!! 
//print_lsusb(); // print device summary

tuh_vid_pid_get(dev_addr, &vid, &pid);

    if (vid==0x20bc && pid==0x5500) 
    {
    //gamepad_hid=1;

    gamepad_addr = dev_addr;
    gamepad_instance = instance;
    gamepad_hid = dev_addr+instance;
    }

      msg_bar=6;
      wait_msg = 5000; 
    

}
//-----------------------------------------------------------------
/* Отправка отчета мыши через USB CDC */
static void __not_in_flash_func(mouse_report)(uint8_t const *report, uint16_t len) {
  
  /* Настройки DPI (точек на дюйм) */
  const uint8_t max_dpi = 4;             /* Максимальный коэффициент масштабирования */
  const uint8_t min_dpi = 1;             /* Минимальный коэффициент масштабирования */
  
  /* Структура для разбора формата отчета */
  struct {
      uint8_t btn_offset;    /* Смещение байта кнопок */
      uint8_t x_offset;      /* Смещение байта X */
      uint8_t y_offset;      /* Смещение байта Y */
      uint8_t wheel_offset;  /* Смещение байта колеса */
  } proto;

  /* Определение формата входящего отчета */
  switch(len) {
      case 6:  /* Режим тачпада (6 байт) */
          proto.btn_offset = 1;
          proto.x_offset = 2;
          proto.y_offset = 3;
          proto.wheel_offset = 5;
          break;
          
      case 3:  /* Стандартная мышь (3 байта) */
          proto.btn_offset = 0;
          proto.x_offset = 1;
          proto.y_offset = 2;
          proto.wheel_offset = 0xFF; /* Отсутствие колеса */
          break;
          
      default: /* Неизвестный формат */
          return;
  }

  /* Обработка кнопок (активный низкий уровень) */
  mouse_buttons = 0xFF; /* Сброс всех кнопок */
  const uint8_t btn_report = report[proto.btn_offset];
 
  /* Маппинг физических кнопок на логические биты */
  if(btn_report & (1 << 0)) mouse_buttons &= ~(1 << 1); /* Левая кнопка */
  if(btn_report & (1 << 1)) mouse_buttons &= ~(1 << 0); /* Правая кнопка */
  if(btn_report & (1 << 2)) mouse_buttons &= ~(1 << 2); /* Средняя кнопка */

  /* Обработка перемещения по X с учетом DPI */
  const int8_t x_delta_raw = (int8_t)report[proto.x_offset]; /* Сырое значение */
  const int16_t x_delta = x_delta_raw * conf.mouse_dpi; /* Применение DPI */
  mouse_x += x_delta;

  /* Обработка перемещения по Y с учетом DPI и инверсии */
  const int8_t y_delta_raw = (int8_t)report[proto.y_offset]; /* Сырое значение */
  const int16_t y_delta = y_delta_raw * conf.mouse_dpi; /* Применение DPI */
  mouse_y -= y_delta; /* Инверсия для экранных координат */

  /* Обработка колеса прокрутки (если есть в отчете) */
  if(proto.wheel_offset != 0xFF && report[proto.wheel_offset] != 0) {
      const int8_t wheel_delta = (int8_t)report[proto.wheel_offset];
      mouse_wheel += wheel_delta;
  }

  convert_kempstonMouse();
}

/* Функция изменения DPI (может вызываться извне) */
/* void set_mouse_dpi(uint8_t new_dpi) {
  const uint8_t max_dpi = 4;
  const uint8_t min_dpi = 1;
  
  // Ограничение значения DPI 
  if(new_dpi > max_dpi) new_dpi = max_dpi;
  if(new_dpi < min_dpi) new_dpi = min_dpi;
  
  mouse_dpi = new_dpi;
}  */
  /* Можно добавить индикацию смены DPI (например, мигание светодиодом) */

//-----------------------------------------------------------------
 // send mouse report to usb device CDC
static void mouse_report0(uint8_t const *report, uint16_t len) {
        uint8_t mouse_b = 0xff; ;//#FADF - поpт  кнопок
        uint8_t ms_x =0;
        uint8_t ms_y =0;
      //  uint8_t ms_w =0;
      static  uint8_t mouse_x =0;
      static  uint8_t mouse_y =0;
      static  uint8_t mouse_w =0;
switch (len)
{
case 6: /*мышь тачпад */
  
  if (report[1] & (1<<0) ) mouse_b &= ~(1 << 1); //1 бит - состояние левой кнопки;MOUSE_BUTTON_LEFT
  if (report[1] & (1<<1) ) mouse_b &= ~(1 << 0); //0 бит - состояние правой кнопки; MOUSE_BUTTON_RIGHT
  if (report[1] & (1<<2) ) mouse_b &= ~(1 << 2); //2 бит - состояние сpедней кнопки; MOUSE_BUTTON_MIDDLE)
 
  // X
    if (ms_x != report[2]){ 
    if (report[2]&(1<<7)) mouse_x = mouse_x-1;//mouse_x--;//left
    else mouse_x = mouse_x+1;//mouse_x++;//right 
    ms_x = report[2];
    }

  // мышь 8 байт 
  //  if (len==8) report++;
  // Y   
    if (ms_y != report[3]){ 
    if (report[3]&(1<<7)) mouse_y = mouse_y+1;//mouse_y++;//up
    else mouse_y = mouse_y-1;//mouse_y--; //down
    ms_y = report[3];
    }
  
  // колесо
    if (report[5] !=0){ 
    if (report[5]&(1<<7)) mouse_w++;//up
    else mouse_w--; //down
   }
   mouse[1] =mouse_b; ;//#FADF - поpт  кнопок
   mouse[2]=mouse_x ; // #FBDF
   mouse[3]=(mouse_y); // #FFDF
 // int count = sprintf(tempbuf, "[%u] %c%c%c %d %d %d\r\n", dev_addr, l, m, r, report->x, report->y, report->wheel);
break;


case 3:
  
  if (report[0] & (1<<0) ) mouse_b &= ~(1 << 1); //1 бит - состояние левой кнопки;MOUSE_BUTTON_LEFT
  if (report[0] & (1<<1) ) mouse_b &= ~(1 << 0); //0 бит - состояние правой кнопки; MOUSE_BUTTON_RIGHT
  if (report[0] & (1<<2) ) mouse_b &= ~(1 << 2); //2 бит - состояние сpедней кнопки; MOUSE_BUTTON_MIDDLE)
 
  // X
    if (ms_x != report[1]){ 
    if (report[1]&(1<<7)) mouse_x = mouse_x-conf.mouse_dpi;//mouse_x--;//left
    else mouse_x = mouse_x+conf.mouse_dpi;//mouse_x++;//right 
    ms_x = report[1];
    }

  // мышь 8 байт 
  //  if (len==8) report++;
  // Y   
    if (ms_y != report[2]){ 
    if (report[2]&(1<<7)) mouse_y = mouse_y+conf.mouse_dpi;//mouse_y++;//up
    else mouse_y = mouse_y-conf.mouse_dpi;//mouse_y--; //down
    ms_y = report[2];
    }
  
  // колесо
    if (report[3] !=0){ 
    if (report[3]&(1<<7)) mouse_w++;//up
    else mouse_w--; //down
   }
   mouse[1] =mouse_b; ;//#FADF - поpт  кнопок
   mouse[2]=mouse_x ; // #FBDF
   mouse[3]=mouse_y; // #FFDF

break;

 
}
  
}

// send mouse report to usb device CDC
void mouse_report1( hid_mouse_report_t const* report) 
{
        uint8_t mouse_b = 0xff; ;//#FADF - поpт  кнопок
        uint8_t ms_x =0;
        uint8_t ms_y =0;
      //  uint8_t ms_w =0;
       uint8_t mouse_x =0;
        uint8_t mouse_y =0;
        uint8_t mouse_w =0;

  if (report->buttons & MOUSE_BUTTON_LEFT ) mouse_b &= ~(1 << 1); //1 бит - состояние левой кнопки;MOUSE_BUTTON_LEFT
  if (report->buttons & MOUSE_BUTTON_RIGHT  ) mouse_b &= ~(1 << 0); //0 бит - состояние правой кнопки; MOUSE_BUTTON_RIGHT
  if (report->buttons & MOUSE_BUTTON_MIDDLE ) mouse_b &= ~(1 << 2); //2 бит - состояние сpедней кнопки; MOUSE_BUTTON_MIDDLE)

 
  // X
    if (ms_x != report->x){ 
    if (report->x&(1<<7)) mouse_x = mouse_x-conf.mouse_dpi;//mouse_x--;//left
    else mouse_x = mouse_x+1;//mouse_x++;//right 
    ms_x = report->x;
    }


  // Y   
    if (ms_y != report->y){ 
    if (report->y&(1<<7)) mouse_y = mouse_y+conf.mouse_dpi;//mouse_y++;//up
    else mouse_y = mouse_y-conf.mouse_dpi;//mouse_y--; //down
    ms_y = report->y;
    }
  
  // колесо
    if (report->wheel !=0){ 
    if (report->wheel&(1<<7)) mouse_w++;//up
    else mouse_w--; //down
   }
   mouse[1] =mouse_b; ;//#FADF - поpт  кнопок
   mouse[2]=mouse_x ; // #FBDF
   mouse[3]=(mouse_y); // #FFDF
 // int count = sprintf(tempbuf, "[%u] %c%c%c %d %d %d\r\n", dev_addr, l, m, r, report->x, report->y, report->wheel); 
}
//----------------------------------------------------------
 //кемпстон джойстик
//void __not_in_flash_func(gamepad_1)(uint8_t const *report,uint8_t instance)
void gamepad_10(uint8_t const *report,uint8_t instance)
{

//uint8_t joy_k = 0x00 ;//#1F - кемпстон джойстик 0001 1111
joy_k = 0x00;//#1F - кемпстон джойстик 0001 1111

//if (report[0] !=0)
{
  uint8_t b = report[0];
//if (report[0] & 0b00000001)  joy_k |= (1 << 4);//debug_print("PRESSING A\n");// A
joy_k |= (b & 0b00000001)<<4;//4 A
joy_k |= (b & 0b00000010)<<4;//5 B
joy_k |= (b & 0b00001000)<<2;//6 X
joy_k |= (b & 0b00010000)<<3;//7 Y

/* if (report[0] & 0b00000010)  joy_k |= (1 << 5);//debug_print("PRESSING B\n");// B
if (report[0] & 0b00001000)  joy_k |= (1 << 6);//debug_print("PRESSING X\n");// X
if (report[0] & 0b00010000)  joy_k |= (1 << 7);// debug_print("PRESSING Y\n");// Y
 */
}

if (report[1] !=0)
{
//if (report[1] & 0b00000001)  debug_print("PRESSING L2\r\n");// L2
//if (report[1] & 0b00000010)  debug_print("PRESSING R2\r\n");// R2
//if (report[1] & 0b00000100)  debug_print("PRESSING SELECT\r\n");// SELECT
//if (report[1] & 0b00001000)  debug_print("PRESSING START\r\n");// START
//if (report[1] & 0b00010000)  debug_print("PRESSING MODE\r\n");// MODE
//debug_print("1: 0x%02X 0x%02X 0x%02X 0x%02X  0x%02X 0x%02X 0x%02X ", report[0] ,report[1], report[2], report[3], report[4], report[5], report[6], report[7]);
//debug_print("2: 0x%02X 0x%02X 0x%02X 0x%02X  0x%02X 0x%02X 0x%02X\r\n", report[8] , report[9], report[10], report[11], report[12], report[13], report[14]);



}
if (report[2] !=0x0f)
{     
     uint8_t p = report[2] & 0x0f; // 0000XXXX 0-8
     switch (p){
      case 0:  joy_k |= (1 << 3); break; //3 бит  up
      case 1:  joy_k |= (1 << 3); joy_k |= (1 << 0); break; //3 бит up  0 бит  right  up right
      case 2:  joy_k |= (1 << 0); break; // 0 бит  right
      case 3:  joy_k |= (1 << 2); joy_k |= (1 << 0); break; //2 бит down  0 бит  right  down right
      case 4:  joy_k |= (1 << 2); break; //2 бит down  
      case 5:  joy_k |= (1 << 2); joy_k |= (1 << 1); break; //2 бит down  1 бит  left  down left
      case 6:  joy_k |= (1 << 1); break; // 1 бит  left
      case 7:  joy_k |= (1 << 3); joy_k |= (1 << 1); break; //3 бит up  1 бит  left  up left
      default: break;
      }


}

  //  msg_bar=7;
  //    wait_msg = 1000;  
  
// data = data| joy_k;

//  joy_key_ext = joy_key_ext | joy_k;



}
 //----------------------------------------------------------


 //----------------------------------------------------------
 //кемпстон джойстик
void __not_in_flash_func(gamepad_1)(uint8_t const *report,uint8_t instance)
{
  joy_connected = true;
//uint8_t joy_k = 0x00 ;//#1F - кемпстон джойстик 0001 1111
joy_k = 0x00;//#1F - кемпстон джойстик 0001 1111

if (report[0] !=0)
{
if (report[0] & 0b00000001)  joy_k |=  0x20  ;//(1 << 4);//debug_print("PRESSING A\n");// A
if (report[0] & 0b00000010)  joy_k |= 0x10; //(1 << 5);//debug_print("PRESSING B\n");// B
if (report[0] & 0b00001000)  joy_k |= 0x40;//(1 << 6);//debug_print("PRESSING X\n");// X
if (report[0] & 0b00010000)  joy_k |= 0x80;//(1 << 7);// debug_print("PRESSING Y\n");// Y

//if (report[0] & 0b00100000)  joy_k |=0x10;
if (report[0] & 0b10000000)  joy_k |=0x20;// дубль A
}

if (report[1] !=0)
{
//if (report[1] & 0b00000001)  //debug_print("PRESSING L2\r\n");// L2
//joy_k |=  0x20  ;
if (report[1] & 0b00000010) // debug_print("PRESSING R2\r\n");// R2
joy_k |= 0x10;// fire дубль B

if (report[1] & 0b00000100)  //debug_print("PRESSING SELECT\r\n");// SELECT
 //{ joy_key_ext = SELECT_J;joy_k=0; return;}
 joy_k |=  SELECT_J ;
if (report[1] & 0b00001000) // debug_print("PRESSING START\r\n");// START
// { joy_key_ext = START_J;joy_k=0; return;}
joy_k |=START_J; 
//if (report[1] & 0b00010000)  debug_print("PRESSING MODE\r\n");// MODE
//debug_print("1: 0x%02X 0x%02X 0x%02X 0x%02X  0x%02X 0x%02X 0x%02X ", report[0] ,report[1], report[2], report[3], report[4], report[5], report[6], report[7]);
//debug_print("2: 0x%02X 0x%02X 0x%02X 0x%02X  0x%02X 0x%02X 0x%02X\r\n", report[8] , report[9], report[10], report[11], report[12], report[13], report[14]);



}
if (report[2] !=0x0f)
{     
     uint8_t p = report[2] & 0x0f; // 0000XXXX 0-8
     switch (p){
      case 0:  joy_k |= (1 << 3); break; //3 бит  up
      case 1:  joy_k |= (1 << 3); joy_k |= (1 << 0); break; //3 бит up  0 бит  right  up right
      case 2:  joy_k |= (1 << 0); break; // 0 бит  right
      case 3:  joy_k |= (1 << 2); joy_k |= (1 << 0); break; //2 бит down  0 бит  right  down right
      case 4:  joy_k |= (1 << 2); break; //2 бит down  
      case 5:  joy_k |= (1 << 2); joy_k |= (1 << 1); break; //2 бит down  1 бит  left  down left
      case 6:  joy_k |= (1 << 1); break; // 1 бит  left
      case 7:  joy_k |= (1 << 3); joy_k |= (1 << 1); break; //3 бит up  1 бит  left  up left
      default: break;
      }


}
}
 //----------------------------------------------------------







// Вызывается при получении отчета от устройства через конечную точку прерывания
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len)

{
  (void) len;
  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

 // joy_k =itf_protocol;
 // msg_bar=7;
  //    wait_msg = 5000; 

  switch(itf_protocol)
  {
    case HID_ITF_PROTOCOL_KEYBOARD:

    keyboard_report(report, len);
   // process_kbd_report(dev_addr, instance, (hid_keyboard_report_t const*) report ); 
  
    break;

    case HID_ITF_PROTOCOL_MOUSE:
    mouse_report(report, len);
      break;

 

    default: 
       //  if (gamepad_hid==1)
        {
      //    uint8_t k =  dev_addr+instance;

    //   if (gamepad_hid == k)
if (gamepad_addr == dev_addr)
    if (instance==0)
           gamepad_1(report,instance);// kempston
        }  
    break;
  }

  // продолжайте запрашивать получение отчета
 if ( !tuh_hid_receive_report(dev_addr, instance) )
  {
    // Не удается запросить отчет
  }
}
//-------------------------------------------------------
bool init_usb_hid(void)
{
  return tuh_init(0);
}
//--------------------------------------------------------







void wait_enter(void)
     {
      
      while(1)
      {
       decode_key_joy();
      if (!(kb_st_ps2.u[1]&KB_U1_ENTER) ) return;
   //    sleep_ms(DELAY_KEY);
      }
    } 

void wait_esc(void)
     {
    //  return;
      while(1)
      {
       decode_key_joy();
      if (!(kb_st_ps2.u[1]&KB_U1_ESC) ) return;
   //    sleep_ms(DELAY_KEY);
      }
    } 