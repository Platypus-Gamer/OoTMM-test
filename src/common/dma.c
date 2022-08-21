#include <combo.h>
#include <stdint.h>

#define OOT_DMA_ADDR    0x7430
#define OOT_DMA_COUNT   0x5e6
#define MM_DMA_ADDR     (0x1a500 | MM_BASE)
#define MM_DMA_COUNT    0x610

#if defined(GAME_OOT)
# define DMA_ADDR           OOT_DMA_ADDR
# define DMA_COUNT          OOT_DMA_COUNT
# define DMA_ADDR_FOREIGN   MM_DMA_ADDR
# define DMA_COUNT_FOREIGN  MM_DMA_COUNT
#else
# define DMA_ADDR           MM_DMA_ADDR
# define DMA_COUNT          MM_DMA_COUNT
# define DMA_ADDR_FOREIGN   OOT_DMA_ADDR
# define DMA_COUNT_FOREIGN  OOT_DMA_COUNT
#endif

#define VROM_FOREIGN_OFFSET  0x04000000

typedef struct
{
    uint32_t vstart;
    uint32_t vend;
    uint32_t pstart;
    uint32_t pend;
}
DmaEntry;

ALIGNED(16) DmaEntry kComboDmaData[DMA_COUNT + DMA_COUNT_FOREIGN + 1];

extern s16 gDmaDataCount;

void comboDma(void* addr, uint32_t cartAddr, uint32_t size);

void comboInitDma(void)
{
    DmaEntry* e;
    comboDma(kComboDmaData, DMA_ADDR, DMA_COUNT * sizeof(DmaEntry));
    comboDma(kComboDmaData + DMA_COUNT, DMA_ADDR_FOREIGN, DMA_COUNT_FOREIGN * sizeof(DmaEntry));
    for (int i = DMA_COUNT; i < DMA_COUNT + DMA_COUNT_FOREIGN; ++i)
    {
        e = kComboDmaData + i;

        if (e->pend == 0xffffffff)
            continue;

        e->vstart |= VROM_FOREIGN_OFFSET;
        e->vend |= VROM_FOREIGN_OFFSET;
    }

#if defined(GAME_MM)
    gDmaDataCount = DMA_COUNT + DMA_COUNT_FOREIGN;
#endif
}

static void waitForPi(void)
{
    u32 status;

    for (;;)
    {
        status = IO_READ(PI_STATUS_REG);
        if ((status & 3) == 0)
            return;
    }
}

void comboDma(void* dramAddr, u32 cartAddr, u32 size)
{
    waitForPi();
    IO_WRITE(PI_DRAM_ADDR_REG, (u32)dramAddr & 0x1fffffff);
    IO_WRITE(PI_CART_ADDR_REG, cartAddr | 0x10000000);
    IO_WRITE(PI_WR_LEN_REG, size - 1);
    waitForPi();
    osInvalDCache(dramAddr, size);
    osInvalDCache(dramAddr, size);
}
