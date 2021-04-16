#pragma once

#include "kernel/devices/PCIDevice.h"
#include "kernel/memory/MMIO.h"
#include <libutils/RingBuffer.h>
#include <libutils/Vector.h>

#define SND_KNOB_MASTER 0
#define SND_KNOB_VENDOR 1024

/* BARs! */
#define AC97_NAMBAR 0x10  /* Native Audio Mixer Base Address Register */
#define AC97_NABMBAR 0x14 /* Native Audio Bus Mastering Base Address Register */

/* Bus mastering IO port offsets */
#define AC97_PO_BDBAR 0x10 /* PCM out buffer descriptor BAR */
#define AC97_PO_CIV 0x14   /* PCM out current index value */
#define AC97_PO_LVI 0x15   /* PCM out last valid index */
#define AC97_PO_SR 0x16    /* PCM out status register */
#define AC97_PO_PICB 0x18  /* PCM out position in current buffer register */
#define AC97_PO_CR 0x1B    /* PCM out control register */

/* Bus mastering misc */
/* Buffer descriptor list constants */
// max buffer len 64kb
#define AC97_RINGBUFFER_LEN 0x10000
#define AC97_BDL_LEN 32 /* Buffer descriptor list length */

// buffer len 16kb so that device can buffer 4 buffers
#define AC97_BDL_BUFFER_LEN 0x4000                    /* Length of buffer in BDL */
#define AC97_CL_GET_LENGTH(cl) ((cl)&0xFFFE)          /* Decode length from cl */
#define AC97_CL_SET_LENGTH(cl, v) ((cl) = (v)&0xFFFE) /* Encode length to cl */
#define AC97_CL_BUP ((uint32_t)1 << 30)               /* Buffer underrun policy in cl */
#define AC97_CL_IOC ((uint32_t)1 << 31)               /* Interrupt on completion flag in cl */

/* PCM out control register flags */
#define AC97_X_CR_RPBM (1 << 0)  /* Run/pause bus master */
#define AC97_X_CR_RR (1 << 1)    /* Reset registers */
#define AC97_X_CR_LVBIE (1 << 2) /* Last valid buffer interrupt enable */
#define AC97_X_CR_FEIE (1 << 3)  /* FIFO error interrupt enable */
#define AC97_X_CR_IOCE (1 << 4)  /* Interrupt on completion enable */

/* Status register flags */
#define AC97_X_SR_DCH (1 << 0)   /* DMA controller halted */
#define AC97_X_SR_CELV (1 << 1)  /* Current equals last valid */
#define AC97_X_SR_LVBCI (1 << 2) /* Last valid buffer completion interrupt */
#define AC97_X_SR_BCIS (1 << 3)  /* Buffer completion interrupt status */
#define AC97_X_SR_FIFOE (1 << 4) /* FIFO error */

/* Mixer IO port offsets */
#define AC97_RESET 0x0000
#define AC97_MASTER_VOLUME 0x0002
#define AC97_AUX_OUT_VOLUME 0x0004
#define AC97_MONO_VOLUME 0x0006
#define AC97_PC_BEEP 0x000A
#define AC97_PCM_OUT_VOLUME 0x0018
#define AC97_EXT_AUDIO_ID 0x0028
#define AC97_EXT_AUDIO_STC 0x002A
#define AC97_FRONT_SPLRATE 0x002C
#define AC97_LR_SPLRATE 0x0032
#define AC97_PLAYBACK_SPEED 48000
#define AC97_GLB_CTRL_STAT 0x0060

struct PACKED AC97BufferDescriptor
{
    uintptr_t pointer;
    uint32_t cl;
};

class AC97 : public PCIDevice
{
private:
    uint16_t _status;

    uint16_t nabmbar;
    uint16_t nambar;

    uint8_t _last_valid_index;

    // device ring buffer
    RingBuffer<char> _buffer{AC97_RINGBUFFER_LEN};

    // buffer descriptors range
    RefPtr<MMIORange> buffer_descriptors_range{};
    // buffer descriptor list of size 32 (MAX SIZE FOR AC97)
    AC97BufferDescriptor *buffer_descriptors_list;
    // 32*30720 buffer array for playing sound using PCM 16 Bit out (DMA)
    Vector<RefPtr<MMIORange>> buffers;

    bool _quirk_5bit_volume;

    uint16_t _volume_PCM;
    uint16_t _volume_master;
    uint16_t _sample_rate;

    void initialise_buffers();

    void query_from_buffer(void *destination, size_t size);

public:
    AC97(DeviceAddress address);

    ~AC97();

    void acknowledge_interrupt() override;

    void handle_interrupt() override;

    bool can_write() override;

    ResultOr<size_t> write(size64_t offset, const void *buffer, size_t size) override;

    Result call(IOCall request, void *args) override;
};
