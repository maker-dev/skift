#pragma once

#include <libgraphic/font/FontFace.h>
#include <libgraphic/font/TrueTypeFont.h>
#include <libio/Reader.h>
#include <libio/ScopedReader.h>
#include <libutils/HashMap.h>
#include <libutils/Vector.h>
#include <libutils/unicode/Codepoint.h>

namespace Graphic::SuperCoolFont
{

class TrueTypeFontFace : public FontFace
{
private:
    uint16_t _num_glyphs = 0;
    HashMap<Codepoint, uint32_t> _codepoint_glyph_mapping;
    TrueTypeHeader _header;
    Vector<uint32_t> _glyph_offsets;
    Vector<TrueTypeTable> _tables;
    Vector<Glyph> _glyphs;

    Result read_tables(IO::MemoryReader &reader);
    Result parse_tables(IO::MemoryReader &reader);

    Result read_table_head(IO::Reader &reader);
    Result read_table_maxp(IO::Reader &reader);
    Result read_table_cmap(IO::Reader &reader);
    Result read_table_loca(IO::Reader &reader);
    Result read_table_glyf(IO::Reader &reader);
    
    ResultOr<TrueTypeVersion> read_version(IO::Reader &reader);

public:
    static ResultOr<RefPtr<TrueTypeFontFace>>
    load(IO::Reader &reader);

    inline bool has_table(uint32_t tag)
    {
        return _tables.find([&](auto &t) { return t.tag() == tag; }).present();
    }

    virtual String family() override;
    virtual FontStyle style() override;
    virtual Optional<Glyph> glyph(Codepoint c) override;
};
} // namespace Graphic::SuperCoolFont