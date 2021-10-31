#pragma once
#include <stdint.h>
#include <stdbool.h>

#define PIC_MASTER  0x20
#define PIC_SLAVE   0xA0

#define PIC_A0  1

#define PIC_ICW1_IC4    (1 << 0)
#define PIC_ICW1_SNGL   (1 << 1)
#define PIC_ICW1_ADI    (1 << 2)
#define PIC_ICW1_LTIM   (1 << 3)
#define PIC_ICW1_INIT   (1 << 4)
#define PIC_ICW1_A5     (1 << 5)
#define PIC_ICW1_A6     (1 << 6)
#define PIC_ICW1_A7     (1 << 7)

#define PIC_ICW2_A8     (1 << 0)
#define PIC_ICW2_A9     (1 << 1)
#define PIC_ICW2_A10    (1 << 2)
#define PIC_ICW2_T3     (1 << 3)
#define PIC_ICW2_T4     (1 << 4)
#define PIC_ICW2_T5     (1 << 5)
#define PIC_ICW2_T6     (1 << 6)
#define PIC_ICW2_T7     (1 << 7)

#define PIC_ICW3_M_S0   (1 << 0)
#define PIC_ICW3_M_S1   (1 << 1)
#define PIC_ICW3_M_S2   (1 << 2)
#define PIC_ICW3_M_S3   (1 << 3)
#define PIC_ICW3_M_S4   (1 << 4)
#define PIC_ICW3_M_S5   (1 << 5)
#define PIC_ICW3_M_S6   (1 << 6)
#define PIC_ICW3_M_S7   (1 << 7)

#define PIC_ICW3_S_ID0  (1 << 0)
#define PIC_ICW3_S_ID1  (1 << 1)
#define PIC_ICW3_S_ID2  (1 << 2)

#define PIC_ICW4_UPM    (1 << 0)
#define PIC_ICW4_AEOI   (1 << 1)
#define PIC_ICW4_MS     (1 << 2)
#define PIC_ICW4_BUF    (1 << 3)
#define PIC_ICW4_SFNM   (1 << 4)

#define PIC_OCW1_M0     (1 << 0)
#define PIC_OCW1_M1     (1 << 1)
#define PIC_OCW1_M2     (1 << 2)
#define PIC_OCW1_M3     (1 << 3)
#define PIC_OCW1_M4     (1 << 4)
#define PIC_OCW1_M5     (1 << 5)
#define PIC_OCW1_M6     (1 << 6)
#define PIC_OCW1_M7     (1 << 7)

#define PIC_OCW2_L0     (1 << 0)
#define PIC_OCW2_L1     (1 << 1)
#define PIC_OCW2_L2     (1 << 2)
#define PIC_OCW2_EOI    (1 << 5)
#define PIC_OCW2_SL     (1 << 6)
#define PIC_OCW2_R      (1 << 7)

void pic_init(uint8_t master_offset, uint8_t slave_offset);
uint8_t pic_get_irq_mask(uint8_t pic);
void pic_set_irq_mask(uint8_t pic, uint8_t mask);
bool pic_irq_is_masked(uint8_t pic, uint8_t irq);
void pic_mask_irq(uint8_t pic, uint8_t irq);
void pic_umask_irq(uint8_t pic, uint8_t irq);
void pic_send_eoi(uint8_t pic);