#pragma once

#include <libgraphic/font/FontFace.h>
#include <libio/Reader.h>

namespace Graphic::SuperCoolFont
{
ResultOr<RefPtr<FontFace>> load(IO::Reader &reader);
} // namespace Graphic::SuperCoolFont