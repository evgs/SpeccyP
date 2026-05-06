#include "zx_machine.h"
#include "config.h"
#include "../aySoft.h"

#include "quorum.h"

extern bool psram_type;
extern uint8_t pent_config;
extern uint32_t ticks_per_frame;

extern bool zx_state_48k_MODE_BLOCK;

extern uint8_t zx_7ffd_lastOut;
extern uint8_t zx_1ffd_lastOut;
extern uint8_t zx_0000_lastOut;

extern uint32_t zx_RAM_bank_active;


extern uint8_t rom;
extern uint8_t *zx_cpu_ram[];
extern uint8_t *zx_ram_bank[]; 
extern uint8_t* zx_rom_bank[];

extern volatile uint8_t * PSRAM_DATA ;

extern uint8_t zx_keyboardDecode(uint8_t addrH);
extern uint8_t port_atr(void);
extern ZX_Input_t zx_input;
extern void trdos_out(uint8_t port, uint8_t val);


#ifdef MURM1
#include "psram_spi.h"
#endif

//#include "rom/QU7V45T5.h" // original Quorum 256-Quorum1024 rom
#include "rom/QU7V42T5.h" // original Quorum 256-Quorum1024 rom


void init_rom_ram_Q1024() {
	zx_rom_bank[0]=ROM_B128_QU1024;//128k 
	zx_rom_bank[1]=ROM_B48_QU1024; //48k 
	zx_rom_bank[2]=ROM_TRD_QU1024; //TRDOS 6.04
	zx_rom_bank[3]=ROM_SM_QU1024;  //SYSTEM MENU
	rom=3;
	zx_cpu_ram[0]=zx_rom_bank[3]; // 0x0000 - 0x3FFF с какой банки стартовать

   	zx_RAM_bank_active =0x00;

	zx_state_48k_MODE_BLOCK = false;
}


void nmi_Quorum1024(Machine *self)
{
    rom=3;  
    zx_cpu_ram[0] = zx_rom_bank[3];
    zx_0000_lastOut = 0; 
}

#ifdef MURM1
// чтение из памяти Quorum (PSRAM MURM1)
inline static uint8_t fast(read_z80_q)(Machine *self, uint16_t addr)
{
    const uint16_t masked_addr = addr & 0x3fff;  // Предвычисление маскированного адреса
    uint8_t x = (addr >> 14);
    // Обработка первого сегмента (0x0000-0x3fff)
    if(x == 0) 
    {
        uint8_t ram0 = zx_0000_lastOut & 0b1001;
        switch (ram0) {
        case 0b0001: return zx_ram_bank[0][masked_addr]; break;
        case 0b1001: return read8psram((uint32_t)(8 << 14) | masked_addr); break;
        default: return zx_cpu_ram[0][masked_addr]; break;
        }
    }
    // Обработка верхнего сегмента (0xc000-0xffff)   
    if(x == 3) 
    {
    	//	 write_z80_qif (zx_RAM_bank_active > 7)// 0b 1111 1000
		if (zx_RAM_bank_active & 0xf8)  return read8psram((uint32_t)(zx_RAM_bank_active << 14) | masked_addr); 
    }  
    // Общий случай для x=1,2 и x=3 с обычной RAM
	return zx_cpu_ram[x][masked_addr];
}
//##########################################################################################
// запись в память Quorum
inline static void fast(write_z80_q)(Machine *self, uint16_t addr, uint8_t val)
{
    const uint16_t masked_addr = addr & 0x3fff;  // Предвычисление маскированного адреса
	uint8_t x = (addr >> 14);
    // Обработка первого сегмента (0x0000-0x3fff)
    if(x == 0) 
    {
        uint8_t ram0 = zx_0000_lastOut & 0b1001;
        switch (ram0) {
        case 0b0001: zx_ram_bank[0][masked_addr] = val; return; break;
        case 0b1001: write8psram((uint32_t)(8 << 14) | (masked_addr), val); return; break;
        default: return; //TODO write shadowed page
        }
    }
  
    if(x == 3) {
        if (zx_RAM_bank_active & 0xf8) {
            write8psram((uint32_t)(zx_RAM_bank_active << 14) | (masked_addr), val);
            return;
        }
    }  
    // Общий случай для x=1,2 и x=3 с обычной RAM
	zx_cpu_ram[x][masked_addr] = val;
}
#endif

// чтение из памяти Quorum
inline static uint8_t fast(_read_z80_q)(Machine *self, uint16_t addr)
{
    const uint16_t masked_addr = addr & 0x3fff;  // Предвычисление маскированного адреса
    uint8_t x = (addr >> 14);
    // Обработка первого сегмента (0x0000-0x3fff)
    if(x == 0) 
    {
        uint8_t ram0 = zx_0000_lastOut & 0b1001;
        switch (ram0) {
        case 0b0001: return zx_ram_bank[0][masked_addr]; break;
        case 0b1001: return PSRAM_DATA[(8 << 14) | masked_addr]; break;
        default: return zx_cpu_ram[0][masked_addr]; break;
        }
    }
    // Обработка верхнего сегмента (0xc000-0xffff)   
    if(x == 3) {
		if (zx_RAM_bank_active & 0xf8) return PSRAM_DATA[(zx_RAM_bank_active << 14) | masked_addr];
    }  
    // Общий случай для x=1,2 и x=3 с обычной RAM
	return zx_cpu_ram[x][masked_addr];
}
// запись в память Quorum
inline static void fast(_write_z80_q)(Machine *self, uint16_t addr, uint8_t val)
{
    const uint16_t masked_addr = addr & 0x3fff;  // Предвычисление маскированного адреса
	uint8_t x = (addr >> 14);
    // Обработка первого сегмента (0x0000-0x3fff)
    if(x == 0) 
    {
        uint8_t ram0 = zx_0000_lastOut & 0b1001;
        switch (ram0) {
        case 0b0001: zx_ram_bank[0][masked_addr] = val; return; break;
        case 0b1001: PSRAM_DATA[(8 << 14) | masked_addr] = val; return; break;
        default: return; break;
        }
    }
  
    if(x == 3) 
    {
        if (zx_RAM_bank_active & 0xf8) // >7
        {
            PSRAM_DATA[(zx_RAM_bank_active << 14) | masked_addr]=val;
            return;
        }
    }  
    // Общий случай для x=1,2 и x=3 с обычной RAM
	zx_cpu_ram[x][masked_addr] = val;
}

//###############################################
// IN QUORUM
//###############################################

inline static uint8_t fast(in_z80quorum)(Machine *self, uint16_t port16) {
	uint8_t portH = port16 >> 8;
	uint8_t portL = (uint8_t)port16&0x00ff;

//return 0;

    #if defined(GENERAL_SOUND)
    if (portL == 0xB3) return in_GSP(GS_READ_IN_B3); 
    if (portL == 0xBB) return in_GSP(GS_STATUS_IN_BB); 
    #endif 
    #if defined(Z_CONTROLER)
    if (portL == 0x57) return in_GSP(ZC_READ_IN_57); 
    if (portL == 0x77) return in_GSP(ZC_READ_IN_77); 
    #endif
    #if defined(RTC_NOVA)
    if (portL == 0x89) return in_GSP(RTC_READ_IN_89); 
    #endif

    #ifdef MIDI    
    if (port16 == 0xa1cf ) 	return in_GSP(MIDI_IN); 
    #endif

	if (trdos) // если это tr-dos
	{
        if (portL == 0xFF)       return Requests;
        //((port == 0x7F) || (port == 0x5F) || (port == 0x3F) || (port == 0x1F))
        if ((portL & 0x7F) == portL) return WD1793_Read((portL>>5) & 0b11); // Read from 0x7F to 0x1F port
            
        #if defined RTC_SMUC  // теневой порт
        if (port16  ==  0xDFBA) { return in_GSP(RTC_READ_IN_DFBA);}//чтение порта часов
        if (port16  == 0x5FBA) return 0b01101000;//SMUK_VER;
        #endif
        return 0xFF;  
	} // end tr-dos

	if (portL & 1<<0)
	{
		// МЫШЬ
        if (port16 == 0xfadf) return mouse[1]; //#FADF - поpт  кнопок
        if (port16 == 0xfbdf) return mouse[2]; //#FBDF - поpт X-кооpдинаты;
        if (port16 == 0xffdf) return mouse[3]; //#FFDF - поpт У-кооpдинаты.

        //Kempston джойстик    
        if (portL==0x1f) return (zx_input.kempston | joy_k);
            
        #ifdef  TURBOSOUND         
        if ((port16 & 0xc002) == 0xc000) 	return in_GSP(TS_READ_IN_FFFD); 
        #else  
		if ((port16 & 0xc002) == 0xc000) 	return AY_in_FFFD(); 
        #endif    
	}
	else    //PORTL = 0bxxxxxx0
    {   
        // 0xXX7e - extended keyboard
        if (portL == 0x7e) return 0xff; //TODO
		//загрузка с магнитофона и опрос клавиатуры
		if (hw_zx_get_bit_LOAD())  return zx_keyboardDecode(portH);
		else return(zx_keyboardDecode(portH) & 0b10111111);	
    }

  if (portL== 0xFF) return port_atr();
	return 0xFF;
}

//===========================================================================
// Quorum 512
//===========================================================================
inline static void fast(out_z80quorum)(Machine *self, uint16_t port16, uint8_t val)
{
//	uint8_t portH = port16 >> 8;
	uint8_t portL = (uint8_t)port16;
	uint16_t not_port16 = ~port16;

// QUORUM
 if (portL == 0x00) {
//	if ((val&0b00100000) == 0) zx_cpu_ram[0] = zx_rom_bank[3]; 
//	else  
	zx_0000_lastOut = val;	// QUORUM
	rom_select(); // переключение ПЗУ по портам и по сигналу DOS
	return;
} 
// QUORUM

	if (trdos) {trdos_out(portL,val); return;}// если это tr-dos

		#ifdef GENERAL_SOUND   
        if (portL == 0xB3) {out_GSP(GS_WRITE_OUT_B3,  val);   return;}// передача данных в GS
        if (portL == 0xBB) {out_GSP(GS_COMMAND_OUT_BB,val);   return;}// передача команды в GS
        #else
		//SAA1099
		if(port16 == 0x01FF){saa1099_write(1,val);return;}					
		if(port16 == 0x00FF){saa1099_write(0,val);return;}
        #endif
        #ifdef Z_CONTROLER 
        if (portL == 0x57) {out_GSP(ZC_WRITE_OUT_57,  val); return;}// передача данных в SD карту
        if (portL == 0x77) {out_GSP(ZC_WRITE_OUT_77,val);z_controler_cs = val; return;}//управление SD   SD_SPI_CS0_PIN val&0x02
        #endif
        #ifdef  RTC_NOVA
     case QUORUM128:   if (portL  ==  0x88 ) {out_GSP(RTC_WRITE_OUT_88,  val); return;}//номер регистра часов
        if (portL  ==  0x89 ) {out_GSP(RTC_WRITE_OUT_89,  val); return;}//данные регистра часов
        #endif




	if (port16 & 1) // 
	{

		 if (((not_port16 & 0x8002) == 0x8002)) // 7ffd  1100 0111
		{
			zx_machine_set_7ffd_out(val);
			//return; // 
		};


        #ifdef  TURBOSOUND   
        if (((not_port16 & 0x0002) == 0x0002) && ((port16 & 0xc000) == 0xc000)) // 0xFFFD
		{out_GSP(TS_WRITE_OUT_FFFD, val);   return;}   // OUT(#FFFD),val
    	if (((not_port16 & 0x4002) == 0x4002) && ((port16 & 0x8000) == 0x8000)) // 0xBFFD
		{out_GSP(TS_WRITE_OUT_BFFD, val);   return;}    
        #else    
        if (((not_port16 & 0x0002) == 0x0002) && ((port16 & 0xc000) == 0xc000)) // 0xFFFD
		{AY_out_FFFD(val); return;}											// OUT(#FFFD),val
	    if (((not_port16 & 0x4002) == 0x4002) && ((port16 & 0x8000) == 0x8000)) // 0xBFFD
		{AY_out_BFFD(val); return;}
        #endif

	}
	else
	{
		//hw_zx_set_snd_out(val & 0b10000);					// 10000
		hw_beep_out(val  & 0b00010000);	
		zx_Border_color = ((val & 0x7) << 4) | (val & 0x7); // дублируем для 4 битного видеобуфера
	}
	
}
// end Quorum_1024


void machine_Quorum1024(Machine *self) {
    self->cpu.context      = self;
    #ifdef MURM1
    if (psram_type) {
        self->cpu.fetch_opcode = (Z80Read )read_z80_q;
        self->cpu.fetch        = (Z80Read )read_z80_q;
        self->cpu.nop          = (Z80Read )read_z80_q;
        self->cpu.read         = (Z80Read )read_z80_q;
        self->cpu.write        = (Z80Write)write_z80_q;
    }
    else
    #endif
    {
        self->cpu.fetch_opcode = (Z80Read )_read_z80_q;
        self->cpu.fetch        = (Z80Read )_read_z80_q;
        self->cpu.nop          = (Z80Read )_read_z80_q;
        self->cpu.read         = (Z80Read )_read_z80_q;
        self->cpu.write        = (Z80Write)_write_z80_q;
    }
    self->cpu.in           = (Z80Read )in_z80quorum;
    self->cpu.out          = (Z80Write)out_z80quorum;
    self->cpu.halt         = Z_NULL;
    self->cpu.nmia         = (Z80Read )nmi_Quorum1024;
    self->cpu.inta         = Z_NULL;//= (Z80Read )inta_callback;
    self->cpu.int_fetch    = Z_NULL;
    self->cpu.ld_i_a       = Z_NULL;
    self->cpu.ld_r_a       = Z_NULL;
    self->cpu.reti         = Z_NULL;
    self->cpu.retn         = Z_NULL;
    self->cpu.hook         = Z_NULL;
    self->cpu.illegal      = Z_NULL;
    
    pent_config = QUORUM1024;
    ticks_per_frame=71680 ;// 71680- Пентагон //70908 - 128 +2A // 70784 Scorpion
}
