#if !PICO_NO_HARDWARE
	#include "hardware/pio.h"
#endif

#include "ps2.h"
#include "kb_u_codes.h"
#include "g_config.h"
#include "string.h"
#include "config.h"
#include "hardware/clocks.h"

#include "hardware/structs/pll.h"
#include "hardware/structs/systick.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "usb_key.h"

#include <assert.h>

#define KBD_BUFFER_SIZE 45
static volatile uint8_t kbd_buffer[KBD_BUFFER_SIZE];
static volatile uint8_t head, tail;


static uint8_t dataKB[512];
kb_u_state kb_st_ps2;

//состояние шины данных спектрума для любого адресного состояния

uint8_t* zx_keyboard_state=dataKB;//&dataKB[0x100-];

uint64_t keyboard_state=0;//состояние клавиатурной матрицы спектрума


#define pio_program1_wrap_target 8
#define pio_program1_wrap 9

uint16_t pio_program1_instructions[] = {
    0x80a0, //  0: pull   block                      
    0xa027, //  1: mov    x, osr                     
    0x0028, //  2: jmp    !x, 8                      
    0xa042, //  3: nop                
    0x4002, //  4: in     pins, 8          0x4008         
    0xa042, //  5: nop           
    0x8020, //  6: push   block                      
    0x0003, //  7: jmp    3                          
            //     .wrap_target
    0x80a0, //  8: pull   block                      
    0x6008, //  9: out    pins, 8                    
            //     .wrap
};

const struct pio_program pio_program1 = {
    .instructions = pio_program1_instructions,
    .length = 10,
    .origin = -1,
	#if PICO_PIO_VERSION > 0
    .used_gpio_ranges = 0
    #endif
};

//static
void zx_kb_decode(uint8_t* zx_kb_state)
{
	//memset(zx_keys_matrix,0,8);
	static uint64_t tmp_zx_kb_state64[32];
	uint8_t* tmp_zx_kb_state8=(uint8_t*)tmp_zx_kb_state64;
	
	uint8_t zx_keys_matrix[8];
	convert_kb_u_to_kb_zx(&kb_st_ps2,zx_keys_matrix);
	
	for(int i=0;i<256;i++)
	{
		uint8_t out8=0;
		uint8_t inx=i;
		for(int k=0;k<8;k++)
		{
			if (!(inx&1)) out8|=zx_keys_matrix[k];
			inx>>=1;
		}
		tmp_zx_kb_state8[i]=(~out8);
	}
	//для быстрого копирования всего буфера	
	uint64_t* dst_zx_kb_state64=(uint64_t*)zx_kb_state;
	uint64_t* src_zx_kb_state64=tmp_zx_kb_state64;
	for(int i=32;i--;)
	{
		*dst_zx_kb_state64++=*src_zx_kb_state64++;
	}
	
	
};

#define BITINDEX(BINN, BITNUM) ((BINN)<<6 | (BITNUM))
const static __in_flash() uint8_t ps2Scans_NE0[] = {
		/* 0x00: */ 0xff,
		/* 0x01: */ BITINDEX(3, KB_U3_F9_POS),
        /* 0x02: */ 0xff,
		/* 0x03: */ BITINDEX(3, KB_U3_F5_POS),
		/* 0x04: */ BITINDEX(3, KB_U3_F3_POS),
		/* 0x05: */ BITINDEX(3, KB_U3_F1_POS),
		/* 0x06: */ BITINDEX(3, KB_U3_F2_POS),
		/* 0x07: */ BITINDEX(3, KB_U3_F12_POS),

        /* 0x08: */ 0xff,
        /* 0x09: */ BITINDEX(3, KB_U3_F10_POS),
		/* 0x0A: */ BITINDEX(3, KB_U3_F8_POS),
		/* 0x0B: */ BITINDEX(3, KB_U3_F6_POS),
		/* 0x0C: */ BITINDEX(3, KB_U3_F4_POS),
		/* 0x0D: */ BITINDEX(1, KB_U1_TAB_POS),
		/* 0x0E: */ BITINDEX(1, KB_U1_TILDE_POS),
        /* 0x0f: */ 0xff,

        /* 0x10: */ 0xff,
        /* 0x11: */ BITINDEX(1, KB_U1_L_ALT_POS),
		/* 0x12: */ BITINDEX(1, KB_U1_L_SHIFT_POS),
		/* 0x14: */ BITINDEX(1, KB_U1_L_CTRL_POS),
        /* 0x15: */ BITINDEX(0, KB_U0_Q_POS),
		/* 0x16: */ BITINDEX(1, KB_U1_1_POS),
        /* 0x17: */ 0xff,

        /* 0x18: */ 0xff,
        /* 0x19: */ 0xff,
        /* 0x1A: */ BITINDEX(0, KB_U0_Z_POS),
		/* 0x1B: */ BITINDEX(0, KB_U0_S_POS),
		/* 0x1C: */ BITINDEX(0, KB_U0_A_POS),
		/* 0x1D: */ BITINDEX(0, KB_U0_W_POS),
		/* 0x1E: */ BITINDEX(1, KB_U1_2_POS),
        /* 0x1f: */ 0xff,

        /* 0x20: */ 0xff,
        /* 0x21: */ BITINDEX(0, KB_U0_C_POS),
		/* 0x22: */ BITINDEX(0, KB_U0_X_POS),
		/* 0x23: */ BITINDEX(0, KB_U0_D_POS),
		/* 0x24: */ BITINDEX(0, KB_U0_E_POS),
		/* 0x25: */ BITINDEX(1, KB_U1_4_POS),
		/* 0x26: */ BITINDEX(1, KB_U1_3_POS),
        /* 0x27: */ 0xff,

        /* 0x28: */ 0xff,
        /* 0x29: */ BITINDEX(1, KB_U1_SPACE_POS),
		/* 0x2A: */ BITINDEX(0, KB_U0_V_POS),
		/* 0x2B: */ BITINDEX(0, KB_U0_F_POS),
		/* 0x2C: */ BITINDEX(0, KB_U0_T_POS),
		/* 0x2D: */ BITINDEX(0, KB_U0_R_POS),
		/* 0x2E: */ BITINDEX(1, KB_U1_5_POS),
        /* 0x2f: */ 0xff,

        /* 0x30: */ 0xff,
        /* 0x31: */ BITINDEX(0, KB_U0_N_POS),
        /* 0x32: */ BITINDEX(0, KB_U0_B_POS),
		/* 0x33: */ BITINDEX(0, KB_U0_H_POS),
		/* 0x34: */ BITINDEX(0, KB_U0_G_POS),
		/* 0x35: */ BITINDEX(0, KB_U0_Y_POS),
		/* 0x36: */ BITINDEX(1, KB_U1_6_POS),
        /* 0x37: */ 0xff,

        /* 0x38: */ 0xff,
        /* 0x39: */ 0xff,
        /* 0x3A: */ BITINDEX(0, KB_U0_M_POS),
		/* 0x3B: */ BITINDEX(0, KB_U0_J_POS),
		/* 0x3C: */ BITINDEX(0, KB_U0_U_POS),
		/* 0x3D: */ BITINDEX(1, KB_U1_7_POS),
		/* 0x3E: */ BITINDEX(1, KB_U1_8_POS),
        /* 0x3f: */ 0xff,

        /* 0x40: */ 0xff,
        /* 0x41: */ BITINDEX(0, KB_U0_COMMA_POS),
        /* 0x42: */ BITINDEX(0, KB_U0_K_POS),
		/* 0x43: */ BITINDEX(0, KB_U0_I_POS),
		/* 0x44: */ BITINDEX(0, KB_U0_O_POS),
		/* 0x45: */ BITINDEX(1, KB_U1_0_POS),
		/* 0x46: */ BITINDEX(1, KB_U1_9_POS),
        /* 0x47: */ 0xff,

        /* 0x48: */ 0xff,
        /* 0x49: */ BITINDEX(0, KB_U0_PERIOD_POS),
		/* 0x4A: */ BITINDEX(1, KB_U1_SLASH_POS),
		/* 0x4B: */ BITINDEX(0, KB_U0_L_POS),
		/* 0x4C: */ BITINDEX(0, KB_U0_SEMICOLON_POS),
		/* 0x4D: */ BITINDEX(0, KB_U0_P_POS),
		/* 0x4E: */ BITINDEX(1, KB_U1_MINUS_POS),
        /* 0x4f: */ 0xff,

        /* 0x50: */ 0xff,
        /* 0x51: */ 0xff,
        /* 0x52: */ BITINDEX(0, KB_U0_QUOTE_POS),
        /* 0x53: */ 0xff,
		/* 0x54: */ BITINDEX(0, KB_U0_LEFT_BR_POS),
        /* 0x55: */ 0xff,
        /* 0x56: */ 0xff,
        /* 0x57: */ 0xff,


		/* 0x58: */ BITINDEX(1, KB_U1_CAPS_LOCK_POS),
		/* 0x59: */ BITINDEX(1, KB_U1_R_SHIFT_POS),
		/* 0x55: */ BITINDEX(1, KB_U1_EQUALS_POS),
		/* 0x5A: */ BITINDEX(1, KB_U1_ENTER_POS),
		/* 0x5B: */ BITINDEX(0, KB_U0_RIGHT_BR_POS),
        /* 0x5c: */ 0xff,
		/* 0x5D: */ BITINDEX(1, KB_U1_BACKSLASH_POS),
        /* 0x5e: */ 0xff,
        /* 0x5f: */ 0xff,

        /* 0x60: */ 0xff,
        /* 0x61: */ 0xff,
        /* 0x62: */ 0xff,
        /* 0x63: */ 0xff,
        /* 0x64: */ 0xff,
        /* 0x65: */ 0xff,
		/* 0x66: */ BITINDEX(1, KB_U1_BACK_SPACE_POS),
        /* 0x67: */ 0xff,

        /* 0x68: */ 0xff,
        /* 0x69: */ BITINDEX(2, KB_U2_NUM_1_POS),
        /* 0x6a: */ 0xff,
		/* 0x6B: */ BITINDEX(2, KB_U2_NUM_4_POS),
		/* 0x6C: */ BITINDEX(2, KB_U2_NUM_7_POS),
        /* 0x6d: */ 0xff,
        /* 0x6e: */ 0xff,
        /* 0x6f: */ 0xff,

        /* 0x70: */ BITINDEX(2, KB_U2_NUM_0_POS),
		/* 0x71: */ BITINDEX(2, KB_U2_NUM_PERIOD_POS),
		/* 0x72: */ BITINDEX(2, KB_U2_NUM_2_POS),
		/* 0x73: */ BITINDEX(2, KB_U2_NUM_5_POS),
		/* 0x74: */ BITINDEX(2, KB_U2_NUM_6_POS),
		/* 0x75: */ BITINDEX(2, KB_U2_NUM_8_POS),
		/* 0x76: */ BITINDEX(1, KB_U1_ESC_POS),
		/* 0x77: */ BITINDEX(2, KB_U2_NUM_LOCK_POS),

        /* 0x78: */ BITINDEX(3, KB_U3_F11_POS),
		/* 0x79: */ BITINDEX(2, KB_U2_NUM_PLUS_POS),
		/* 0x7A: */ BITINDEX(2, KB_U2_NUM_3_POS),
		/* 0x7B: */ BITINDEX(2, KB_U2_NUM_MINUS_POS),
		/* 0x7C: */ BITINDEX(2, KB_U2_NUM_MULT_POS),
		/* 0x7D: */ BITINDEX(2, KB_U2_NUM_9_POS),
		/* 0x7E: */ BITINDEX(2, KB_U2_SCROLL_LOCK_POS),
        /* 0x7f: */ 0xff,

        /* 0x80: */ 0xff,
        /* 0x81: */ 0xff,
        /* 0x82: */ 0xff,
		/* 0x83: */ BITINDEX(3, KB_U3_F7_POS),
};
#define ps2Scans_NE0_size (sizeof(ps2Scans_NE0)/sizeof(uint8_t))
static_assert(ps2Scans_NE0_size == 0x84, "Wrong array size!");

const static __in_flash() uint8_t ps2Scans_E0[] = {
        /* 0x00..0x07: (8)*/
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        /* 0x08..0x0f: (8)*/
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        /* 0x10: */ 0xff,
        /* 0x11 */ BITINDEX(1, KB_U1_R_ALT_POS),
        /* 0x12 */ BITINDEX(2, KB_U2_PRT_SCR_POS),
        /* 0x13: */ 0xff,
        /* 0x14 */ BITINDEX(1, KB_U1_R_CTRL_POS),
        /* 0x15..0x17: (3)*/ 
            0xff, 0xff, 0xff,
        /* 0x18..0x1e: (7) */
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        /* 0x1F */ BITINDEX(1, KB_U1_L_WIN_POS),
        /* 0x20..0x26: (7) */
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        /* 0x27 */ BITINDEX(1, KB_U1_R_WIN_POS),
        /* 0x28..0x2e: (7) */
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        /* 0x2F */ BITINDEX(1, KB_U1_MENU_POS),
        /* 0x30..0x37: (8)*/
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        /* 0x38..0x3f: (8)*/
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        /* 0x40..0x47: (8)*/
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        /* 0x48: */ 0xff,
        /* 0x49: */ 0xff,
        /* 0x4A */ BITINDEX(2, KB_U2_NUM_SLASH_POS),
        /* 0x4b..0x4f: (5)*/
            0xff, 0xff, 0xff, 0xff, 0xff,  
        /* 0x50..0x57: (8)*/
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        /* 0x58: */ 0xff,
        /* 0x59: */ 0xff,
        /* 0x5A */ BITINDEX(2, KB_U2_NUM_ENTER_POS),
        /* 0x5b..0x5f: (5)*/
            0xff, 0xff, 0xff, 0xff, 0xff,  
        /* 0x60..0x67: (8)*/
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        /* 0x68: */ 0xff,
        /* 0x69 */ BITINDEX(2, KB_U2_END_POS),
        /* 0x6a: */ 0xff,
        /* 0x6B */ BITINDEX(2, KB_U2_LEFT_POS),
        /* 0x6C */ BITINDEX(2, KB_U2_HOME_POS),
        /* 0x6d..0x6f: (3)*/
            0xff, 0xff, 0xff,  
        /* 0x70 */ BITINDEX(2, KB_U2_INSERT_POS),
        /* 0x71 */ BITINDEX(2, KB_U2_DELETE_POS),
        /* 0x72 */ BITINDEX(2, KB_U2_DOWN_POS),
        /* 0x73: */ 0xff,
        /* 0x74 */ BITINDEX(2, KB_U2_RIGHT_POS),
        /* 0x75 */ BITINDEX(2, KB_U2_UP_POS),
        /* 0x76..0x79: (4)*/
            0xff, 0xff, 0xff, 0xff,  
        /* 0x7A */ BITINDEX(2, KB_U2_PAGE_DOWN_POS),
        /* 0x7b: */ 0xff,
        /* 0x7c: */ 0xff,
        /* 0x7D */ BITINDEX(2, KB_U2_PAGE_UP_POS),
};
#define ps2Scans_E0_size (sizeof(ps2Scans_E0)/sizeof(uint8_t))
static_assert(ps2Scans_E0_size == 0x7e, "Wrong array size!");

void translate_scancode(uint8_t code, bool is_press, bool is_e0, bool is_e1)
{
    uint8_t bi = 0xff;

    if (is_e1){
		if (code==0x14) { bi = BITINDEX(2, KB_U2_PAUSE_BREAK_POS); }
	}
	
	if (!is_e0) {
        if (code >= ps2Scans_NE0_size) return;
        bi = ps2Scans_NE0[code];
    }

	else {
        if (code >= ps2Scans_E0_size) return;
        bi = ps2Scans_E0[code];
    }
	
    if (bi == 0xff) return;
    int bin = bi >> 6;
    int keybit = bi & 31;
    if (is_press) {
        kb_st_ps2.u[bin] |= 1<<keybit;  
    } else {
        kb_st_ps2.u[bin] &= ~(1<<keybit);  
    }

	
	
}



void  __not_in_flash_func(ps2_proc) (uint8_t val){
    static uint8_t bitcount=0;
	static uint8_t incoming=0;
    uint8_t n;
	val=val?1:0;

	n = bitcount - 1;
	if (n <= 7) {
		incoming |= (val << n);
	}
	bitcount++;
	if (bitcount == 11) {
		uint8_t i = head + 1;
		if (i >= KBD_BUFFER_SIZE) i = 0;
		if (i != tail) {
			kbd_buffer[i] = incoming;
			head = i;
		}
		bitcount = 0;
		incoming = 0;
	}
	
}

uint8_t __not_in_flash_func(get_scan_code)(void)
{
	uint8_t c, i;
	i = tail;
	if (i == head) return 0;
	i++;
	if (i >= KBD_BUFFER_SIZE) i = 0;
	c = kbd_buffer[i];
	tail = i;
	return c;
}

bool decode_PS2(){
	static bool is_e0=false;
	static bool is_e1=false;
	static bool is_f0=false;
	uint8_t scancode=get_scan_code();
	if (scancode==0xe0) {is_e0=true;return false;}
	if (scancode==0xe1) {is_e1=true;return false;}
	if (scancode==0xf0) {is_f0=true;return false;}  
	if (scancode){
		//сканкод 
		//получение универсальных кодов из сканкодов PS/2
		translate_scancode(scancode,!is_f0,is_e0,is_e1);
		is_e0=false;
		if (is_f0) is_e1=false;
		is_f0=false;
		return true;//произошли изменения
	}
	return false;
}



//аппаратная работа с интерфейсом PS/2
//через захват пинов с помощью PIO
int dma_chan;

#define SIZE_LINE_CAPTURE (100)
//количество буферов ДМА можно увеличить, если будет пропуск, но не сильно, иначе будет большая задержка
#define N_DMA_BUF_CAPTURE 1
#define SIZE_DMA_BUF_CAPTURE (SIZE_LINE_CAPTURE*N_DMA_BUF_CAPTURE)
static uint8_t DMA_BUF_CAP[4][SIZE_DMA_BUF_CAPTURE];
static uint8_t* DMA_BUF_ADDR_CAP[4];


void __not_in_flash_func(dma_handler_capture()) {
	
	static uint32_t inx_buf_dma;  
	
	dma_hw->ints1 = 1u << dma_chan;
	dma_channel_set_read_addr(dma_chan,&DMA_BUF_ADDR_CAP[inx_buf_dma&3], false);
	
	inx_buf_dma++;
	uint8_t* buf8=DMA_BUF_CAP[inx_buf_dma&3];

	//работа с клавиатурой PS/2
	uint8_t* kb_line_data=buf8;
	static uint8_t old_state_PS2_CLK;
	for(int k=0;k<SIZE_DMA_BUF_CAPTURE;k++)
	{   
		//работа с данными PS/2

 		const uint8_t mask_PS2_CLK=0x01;
		const uint8_t mask_PS2_DATA=0x02;    
		uint8_t data8=*kb_line_data;
		uint8_t state_PS2_CLK=data8&mask_PS2_CLK;
		if (old_state_PS2_CLK^state_PS2_CLK) //поменялось состояние PS2_CLK
		{
			
			old_state_PS2_CLK=state_PS2_CLK;
			if (state_PS2_CLK==0) ps2_proc(data8&mask_PS2_DATA);//считываем данные по 0 состоянию CLK PS2

		} 
	//	
		kb_line_data++;
		
	}
	//
	
};
void start_PS2_capture(){   
    // Определяем PIO на основе конфигурации
    PIO pio = PIO_PS2;
    bool is_pio1 = (pio == pio1);
    
    // Инициализация GPIO для работы с интерфейсом PS/2 
    gpio_init(beginPS2_PIN); //CLOCK  линия PS/2
    gpio_init(beginPS2_PIN+1); //DATA линия PS/2
    // Отключение внутренних подтягивающих резисторов для обоих пинов
    // (в PS/2 подтяжка обычно реализуется на стороне устройства)
    gpio_disable_pulls(beginPS2_PIN); 
    gpio_disable_pulls(beginPS2_PIN+1);
    // Установка повышенной выходной мощности (12 мА) для обеспечения 
    // стабильного сигнала (актуально для режима OUTPUT)
    gpio_set_drive_strength(beginPS2_PIN, GPIO_DRIVE_STRENGTH_8MA); 
    gpio_set_drive_strength(beginPS2_PIN+1, GPIO_DRIVE_STRENGTH_8MA); 
    // Настройка направления пинов на ВХОД (PS/2 - bidirectional протокол, 
    // но на начальном этапе контроллер работает как устройство)
    gpio_set_dir(beginPS2_PIN, GPIO_IN); 
    gpio_set_dir(beginPS2_PIN+1, GPIO_IN); 

    // назначение номера стейт машины
    int sm = SM_PS2;
    int pin = beginPS2_PIN;
    if (sm == -1) {
        sm = pio_claim_unused_sm(pio, true);
    }
    
    // Добавляем программу в PIO
    uint offset = pio_add_program(pio, &pio_program1);
    
    // Настройка SM
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + pio_program1_wrap_target, offset + pio_program1_wrap);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
    sm_config_set_in_shift(&c, false, true, 32);
    sm_config_set_in_pins(&c, pin);
    
    pio_sm_init(pio, sm, offset+3, &c);
    pio_sm_set_enabled(pio, sm, true);
    
    // Настройка частоты
    float fdiv = clock_get_hz(clk_sys) / 200000.0f;
    pio_sm_set_clkdiv(pio, sm, fdiv);
    
    // Выбор подпрограммы захвата
    pio_sm_put(pio, sm, sm);
    
    // Инициализация DMA буферов
    DMA_BUF_ADDR_CAP[0] = &DMA_BUF_CAP[0][0];
    DMA_BUF_ADDR_CAP[1] = &DMA_BUF_CAP[1][0];
    DMA_BUF_ADDR_CAP[2] = &DMA_BUF_CAP[2][0];
    DMA_BUF_ADDR_CAP[3] = &DMA_BUF_CAP[3][0];
    
    // Настройка DMA каналов
    int dma_chan0 = dma_claim_unused_channel(true);
    dma_chan = dma_claim_unused_channel(true);
    
    // Конфигурация первого DMA канала (чтение из PIO)
    dma_channel_config c0 = dma_channel_get_default_config(dma_chan0);
    channel_config_set_transfer_data_size(&c0, DMA_SIZE_8);
    channel_config_set_read_increment(&c0, false);
    channel_config_set_write_increment(&c0, true);
    
    // Определяем DREQ в зависимости от используемого PIO
    uint dreq;
    if (is_pio1) {
        dreq = DREQ_PIO1_RX0 + sm;
    } else {
        dreq = DREQ_PIO0_RX0 + sm;
    }
    
    channel_config_set_dreq(&c0, dreq);
    channel_config_set_chain_to(&c0, dma_chan);
    
    // Адрес RX FIFO зависит от PIO - правильный тип io_ro_32*
    io_ro_32 *rx_fifo = is_pio1 ? 
                        &pio1->rxf[sm] : 
                        &pio0->rxf[sm];
    
    dma_channel_configure(
        dma_chan0,
        &c0,
        &DMA_BUF_CAP[0][0],    // Write address
        (void*)rx_fifo,        // Read address - приведение к void*
        SIZE_DMA_BUF_CAPTURE,  // Transfer count
        false                  // Don't start yet
    );
    
    // Конфигурация второго DMA канала (управление)
    dma_channel_config c1 = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&c1, DMA_SIZE_32);
    channel_config_set_read_increment(&c1, false);
    channel_config_set_write_increment(&c1, false);
    channel_config_set_chain_to(&c1, dma_chan0);
    
    dma_channel_configure(
        dma_chan,
        &c1,
        &dma_hw->ch[dma_chan0].write_addr, // Write address
        &DMA_BUF_ADDR_CAP[0],              // Read address
        1,                                 // Transfer count
        false                              // Don't start yet
    );
    
    // Настройка прерываний DMA
    dma_channel_set_irq1_enabled(dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_1, dma_handler_capture);
    irq_set_enabled(DMA_IRQ_1, true);
    
    // Запуск DMA
    dma_start_channel_mask((1u << dma_chan0) | (1u << dma_chan));

   // обнуление состояния клавиш
 //  kb_st_ps2.u[0]=kb_st_ps2.u[1]=kb_st_ps2.u[2]=kb_st_ps2.u[3]=0;
  

};

/* void start_PS2_capture1(){   

// Инициализация GPIO для работы с интерфейсом PS/2 
gpio_init(beginPS2_PIN); //CLOCK  линия PS/2
gpio_init(beginPS2_PIN+1); //DATA линия PS/2

// Отключение внутренних подтягивающих резисторов для обоих пинов
// (в PS/2 подтяжка обычно реализуется на стороне устройства)
gpio_disable_pulls(beginPS2_PIN); 
gpio_init(beginPS2_PIN+1); 

// Установка повышенной выходной мощности (12 мА) для обеспечения 
// стабильного сигнала (актуально для режима OUTPUT)
gpio_set_drive_strength(beginPS2_PIN, GPIO_DRIVE_STRENGTH_12MA); 
gpio_set_drive_strength(beginPS2_PIN+1, GPIO_DRIVE_STRENGTH_12MA); 

// Настройка направления пинов на ВХОД (PS/2 - bidirectional протокол, 
// но на начальном этапе контроллер работает как устройство)
gpio_set_dir(beginPS2_PIN, GPIO_IN); 
gpio_set_dir(beginPS2_PIN+1, GPIO_IN); 


// Определяем PIO на основе конфигурации
PIO pio = PIO_PS2;
bool is_pio1 = (pio == pio1);

// назначение номера стейт машины
	int sm=SM_PS2;
	int pin=beginPS2_PIN;
	if (sm == -1) {
       sm = pio_claim_unused_sm(PIO_PS2, true);
    } else {
       sm = sm;
    }
	
// rp2350
// int pio_set_gpio_base(PIO pio, uint gpio_base);
//pio_set_gpio_base(PIO_PS2, 16);
//


//uint offset = pio_claim_free_sm_and_add_program_for_gpio_range();
uint offset = pio_add_program(pio, &pio_program1);
//static bool is_program_gpio_compatible(PIO pio, const pio_program_t *program)
//is_program_gpio_compatible(PIO_PS2,  &pio_program1);

	// Настройка SM
	pio_sm_config c = pio_get_default_sm_config();
	sm_config_set_wrap(&c, offset + pio_program1_wrap_target, offset + pio_program1_wrap);
	sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
	sm_config_set_in_shift(&c, false, true, 32);
	sm_config_set_in_pins(&c, pin);
	pio_sm_init(pio, sm, offset+3, &c);
	pio_sm_set_enabled(pio, sm, true);
	
	// Настройка частоты
	float fdiv=clock_get_hz(clk_sys)/200000;//частота опроса ps/2 X4 
	//PIO_PS2->sm[sm].clkdiv=(uint32_t) (fdiv * (1 << 16)); //делитель для конкретной sm
	 pio_sm_set_clkdiv(pio, sm, fdiv);
	 pio_sm_put(pio, sm, sm);//выбор подпрограммы захвата по номеру SM
	//PIO_PS2->txf[sm]=sm;//выбор подпрограммы захвата по номеру SM
	
	// Инициализация DMA буферов
	DMA_BUF_ADDR_CAP[0]=&DMA_BUF_CAP[0][0];
	DMA_BUF_ADDR_CAP[1]=&DMA_BUF_CAP[1][0];
	DMA_BUF_ADDR_CAP[2]=&DMA_BUF_CAP[2][0];
	DMA_BUF_ADDR_CAP[3]=&DMA_BUF_CAP[3][0];
	
    // Настройка DMA каналов
	int dma_chan0 = dma_claim_unused_channel(true);
	dma_chan  = dma_claim_unused_channel(true);

	// Конфигурация первого DMA канала (чтение из PIO)
	dma_channel_config c0 = dma_channel_get_default_config(dma_chan0);
	channel_config_set_transfer_data_size(&c0, DMA_SIZE_8);
	channel_config_set_read_increment(&c0, false);
	channel_config_set_write_increment(&c0, true);
	
    // Определяем DREQ в зависимости от используемого PIO
    uint dreq;
    if (is_pio1) {
        dreq = DREQ_PIO1_RX0 + sm;
    } else {
        dreq = DREQ_PIO0_RX0 + sm;
    }
    //--------------------------------------------------
	
	channel_config_set_dreq(&c0, dreq);
	channel_config_set_chain_to(&c0, dma_chan);
	
    // Адрес RX FIFO зависит от PIO
    io_rw_32 *rx_fifo = is_pio1 ? 
                        &pio1->rxf[sm] : 
                        &pio0->rxf[sm];
    //----------------------------------

	dma_channel_configure(
		dma_chan0,
		&c0,
		&DMA_BUF_CAP[0][0], // Write address 
		rx_fifo,            // Read address RX FIFO зависит от PIO
		SIZE_DMA_BUF_CAPTURE, // 
		false			 // Don't start yet
	);
	
	// Конфигурация второго DMA канала (управление)

	dma_channel_config c1 = dma_channel_get_default_config(dma_chan);
	channel_config_set_transfer_data_size(&c1, DMA_SIZE_32);
	
	channel_config_set_read_increment(&c1, false);
	channel_config_set_write_increment(&c1, false);

	channel_config_set_chain_to(&c1, dma_chan0);						 // chain to other channel
	
	
	dma_channel_configure(
		dma_chan,
		&c1,
		&dma_hw->ch[dma_chan0].write_addr, // Write address 
		&DMA_BUF_ADDR_CAP[0],			 // read address 
		1, // 
		false			 // Don't start yet
	);
	
	
	// Настройка прерываний DMA
	dma_channel_set_irq1_enabled(dma_chan, true);
	
	// Configure the processor to run dma_handler() when DMA IRQ 0 is asserted
	irq_set_exclusive_handler(DMA_IRQ_1, dma_handler_capture);
	irq_set_enabled(DMA_IRQ_1, true);
	
	// Запуск DMA
	dma_start_channel_mask((1u << dma_chan)) ;	
};
 */