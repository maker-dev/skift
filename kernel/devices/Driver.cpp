#include <libutils/Vector.h>

#include "kernel/devices/LegacyDevice.h"
#include "kernel/devices/PCIDevice.h"
#include "kernel/devices/UNIXDevice.h"

#include "kernel/drivers/AC97.h"
#include "kernel/drivers/BGA.h"
#include "kernel/drivers/E1000.h"
#include "kernel/drivers/LegacyKeyboard.h"
#include "kernel/drivers/LegacyMouse.h"
#include "kernel/drivers/LegacySerial.h"
#include "kernel/drivers/UnixNull.h"
#include "kernel/drivers/UnixRandom.h"
#include "kernel/drivers/UnixZero.h"
#include "kernel/drivers/VirtioBlock.h"
#include "kernel/drivers/VirtioConsole.h"
#include "kernel/drivers/VirtioEntropy.h"
#include "kernel/drivers/VirtioGraphic.h"
#include "kernel/drivers/VirtioNetwork.h"

static Vector<DeviceMatcher *> *_matchers;

void driver_initialize()
{
    logger_info("Installing drivers...");

    _matchers = new Vector<DeviceMatcher *>();

    _matchers->push_back(new PCIDeviceMatcher<BGA>{"QEMU Graphics Adaptator", 0x1234, 0x1111});
    _matchers->push_back(new PCIDeviceMatcher<BGA>{"Virtual Box Graphics Adaptator", 0x80ee, 0xbeef});

    _matchers->push_back(new PCIDeviceMatcher<E1000>{"Intel 82577LM Ethernet Adaptator", 0x8086, 0x10EA});
    _matchers->push_back(new PCIDeviceMatcher<E1000>{"Intel I217 Ethernet Adaptator", 0x8086, 0x153A});
    _matchers->push_back(new PCIDeviceMatcher<E1000>{"Virtual Ethernet Adaptator", 0x8086, 0x100E});

    _matchers->push_back(new PCIDeviceMatcher<AC97>{"Intel ICH", 0x8086, 0x2415});

    _matchers->push_back(new VirtioDeviceMatcher<VirtioBlock>{"VirtI/O Block Device", VIRTIO_DEVICE_BLOCK});
    _matchers->push_back(new VirtioDeviceMatcher<VirtioConsole>{"VirtI/O Console Device", VIRTIO_DEVICE_CONSOLE});
    _matchers->push_back(new VirtioDeviceMatcher<VirtioEntropy>{"VirtI/O Entropy Device", VIRTIO_DEVICE_ENTROPY});
    _matchers->push_back(new VirtioDeviceMatcher<VirtioGraphic>{"VirtI/O Graphic Device", VIRTIO_DEVICE_GRAPHICS});
    _matchers->push_back(new VirtioDeviceMatcher<VirtioNetwork>{"VirtI/O Network Device", VIRTIO_DEVICE_NETWORK});

    _matchers->push_back(new LegacyDeviceMatcher<LegacyKeyboard>{"Legacy Keyboard", LEGACY_KEYBOARD});
    _matchers->push_back(new LegacyDeviceMatcher<LegacyMouse>{"Legacy Mouse", LEGACY_MOUSE});
    _matchers->push_back(new LegacyDeviceMatcher<LegacySerial>{"Legacy Serial Port (COM1)", LEGACY_COM1});
    _matchers->push_back(new LegacyDeviceMatcher<LegacySerial>{"Legacy Serial Port (COM2)", LEGACY_COM2});
    _matchers->push_back(new LegacyDeviceMatcher<LegacySerial>{"Legacy Serial Port (COM3)", LEGACY_COM3});
    _matchers->push_back(new LegacyDeviceMatcher<LegacySerial>{"Legacy Serial Port (COM4)", LEGACY_COM4});

    _matchers->push_back(new UNIXDeviceMatcher<UnixNull>{"Unix Null Device", UNIX_NULL});
    _matchers->push_back(new UNIXDeviceMatcher<UnixRandom>{"Unix Random Device", UNIX_RANDOM});
    _matchers->push_back(new UNIXDeviceMatcher<UnixZero>{"Unix Zero Device", UNIX_ZERO});

    for (size_t i = 0; i < _matchers->count(); i++)
    {
        logger_info("Driver: %s", (*_matchers)[i]->name());
    }
}

DeviceMatcher *driver_for(DeviceAddress address)
{
    for (size_t i = 0; i < _matchers->count(); i++)
    {
        if ((*_matchers)[i]->bus() == address.bus() &&
            (*_matchers)[i]->match(address))
        {
            return (*_matchers)[i];
        }
    }

    return nullptr;
}
