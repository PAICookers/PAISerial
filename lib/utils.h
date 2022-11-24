#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdint.h>

/*
    UART transfering from Byte1 to Byte15, from MSB to LSB.

    Byte1 [7:0]: L2 cluster [7:0] clock enable
    Byte2 [7:0]: L2 cluster [15:8] clock enable
    Byte3 [7:0]: L2 cluster [23:16] clock enable
    Byte4 [7:0]: L2 cluster [31:24] clock enable
    Byte5 [7:0]: L2 cluster [39:32] clock enable
    Byte6 [7:0]: L2 cluster [47:40] clock enable
    Byte7 [7:0]: L2 cluster [55:48] clock enable
    Byte8 [7:0]: L2 cluster [63:56] clock enable

    @note:
        1. When 4 clusters of L2 are disabled, the cluster of L3 will be disable, too.
        2. When 16 clusters of L2 are disabled, the clusters of L4 & 4 clusters of L3 will be disabled, too.
        3. When 64 clusters of L2 are disabled, the clusters of L5 & 4 clusters of L4 & 16 clusters of L3 will be disabled, too.

    Byte9:
        [7:2]: CLKF of PLL, NF=CLKF+1
        [1:0]: CLKR of PLL, NR=CLKR+1
    
    Byte10:
        [7:6]: CLKR of PLL: NR=CLKR+1
        [5:2]: CLKOD of PLL: OD=CLKOD+1
        [1:0]: BWADJ of PLL: BWADJ=CLKF, when INTFB=1
    
    Byte11:
        [7:4]: BWADJ of PLL: BWADJ=CLKF
        [3:0]: chip_x, X address of chip
    
    @note:
        Fout = Fref * NF / (NR * OD)
        Fref = 24MHz
        And MAKE SURE: Fref * NF / NR >= 360MHz
        
        For the maximum clock frequency:
            Fout = 24M * 64 / (1 * 1) = 1536MHz
        For the minimum clock frequency:
            Fout = 360MHz / 16 = 22.5MHz
    
    Byte12:
        [7]: chip_x, X address of chip
        [6:2]: chip_y, Y address of chip
        [1:0]: delay_global_signal, global delay of signal
    
    Byte13:
        [7:0]: delay_global_signal, global delay of signal
    
    Byte14:
        [7:3]: width_global_signal, signal width of global signals Sync_all, Clear_all, Initial_all
        [2:0]: busy_mask_global_signal, mask value of top busy signal

    Byte15:
        [7:1]: busy_mask_global_signal, mask value of top busy signal
        [0]: Debug mode enable

    @note:
        When debug mode is enabled, PAICORE will returned busy and done signals via UART:
        1. Busy signals of 1 cluster of L5 & 4 clusters of L4 & 16 clusters of L3 & 64 clusters of L2, 85 busy signals totally;
        2. Done signals of 64 cluster of L2;
        3. Extra 19 bytes info will be output;
    
    Extra bytes will be returned as below:
    
    Byte1:
        [7]: busy_of_L5, busy signal of L5
        [6:3]: busy_out_L4_X, busy signals of 1~4 clusters of L4
        [2:0]: 3'b000
    
    Byte2 [7:0]: busy_out_L3_X, busy signals of 1~8 clusters of L3
    Byte3 [7:0]: busy_out_L3_X, busy signals of 9~16 clusters of L3
    Byte4 [7:0]: busy_out_L2_X, busy signals of 1~8 clusters of L2
    Byte5 [7:0]: busy_out_L2_X, busy signals of 9~16 clusters of L2
    Byte6 [7:0]: busy_out_L2_X, busy signals of 17~24 clusters of L2
    Byte7 [7:0]: busy_out_L2_X, busy signals of 25~32 clusters of L2
    Byte8 [7:0]: busy_out_L2_X, busy signals of 33~40 clusters of L2
    Byte9 [7:0]: busy_out_L2_X, busy signals of 41~48 clusters of L2
    Byte10 [7:0]: busy_out_L2_X, busy signals of 49~56 clusters of L2
    Byte11 [7:0]: busy_out_L2_X, busy signals of 57~64 clusters of L2
    Byte12 [7:0]: done_out_L2_X, done signals of 1~8 clusters of L2
    Byte13 [7:0]: done_out_L2_X, done signals of 9~16 clusters of L2
    Byte14 [7:0]: done_out_L2_X, done signals of 17~24 clusters of L2
    Byte15 [7:0]: done_out_L2_X, done signals of 25~32 clusters of L2
    Byte16 [7:0]: done_out_L2_X, done signals of 33~40 clusters of L2
    Byte17 [7:0]: done_out_L2_X, done signals of 41~48 clusters of L2
    Byte18 [7:0]: done_out_L2_X, done signals of 49~56 clusters of L2
    Byte19 [7:0]: done_out_L2_X, done signals of 57~64 clusters of L2
*/

typedef struct {
    // uint64_t clk_en;
    uint8_t clk_en_byte_1;
    uint8_t clk_en_byte_2;
    uint8_t clk_en_byte_3;
    uint8_t clk_en_byte_4;
    uint8_t clk_en_byte_5;
    uint8_t clk_en_byte_7;
    uint8_t clk_en_byte_8;
} clk_en_l2_t;

typedef struct {
    clk_en_l2_t CLK_EN_L2;
    uint8_t PLL_CLKF;
    uint8_t PLL_CLKR;
    uint8_t PLL_CLKOD;
    uint8_t BWADJ;
    uint8_t ADDRESS_CHIP_X;
    uint8_t ADDRESS_CHIP_Y;
    uint16_t GLOBAL_DELAY;
    uint8_t GLOBAL_SIGNALS_WIDTH;
    uint16_t GLOBAL_BUSY_MASK;
    uint8_t DEBUG_MODE_ENABLE;
} serial_config_t, *serial_config_p;

typedef struct {
    uint8_t L5_BUSY;
    uint8_t L4_BUSY;
    uint16_t L3_BUSY;
    uint64_t L2_BUSY;
    uint64_t L2_DONE;
} serial_debug_info_t;

#define CLK_EN_L2_0_MASK    (0x80)
#define CLK_EN_L2_1_MASK    (0x40)
#define CLK_EN_L2_2_MASK    (0x20)
#define CLK_EN_L2_3_MASK    (0x10)
#define CLK_EN_L2_4_MASK    (0x08)
#define CLK_EN_L2_5_MASK    (0x04)
#define CLK_EN_L2_6_MASK    (0x02)
#define CLK_EN_L2_7_MASK    (0x01)

#endif