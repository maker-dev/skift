#pragma once
#include <libmath/Rect.h>
#include <libmath/Vec2.h>
#include <libutils/Endian.h>
#include <stdint.h>

namespace Graphic::SuperCoolFont
{
using TrueTypeVersion = Math::Vec2<be_uint16_t>;
using TrueTypeFixed = Math::Vec2<be_uint16_t>;

struct TrueTypeTable
{
    be_uint32_t tag;
    be_uint32_t checksum;
    be_uint32_t offset;
    be_uint32_t size;
};

struct TrueTypeHeader
{
    TrueTypeVersion version;
    TrueTypeFixed revision;
    be_uint32_t checksum_adjustment;
    be_uint32_t magic_number;
    be_uint16_t flags;
    be_uint16_t units_per_em;
    be_uint64_t created;
    be_uint64_t modified;
    // Bounding box that matches all glyphs
    Math::Rect<be_int16_t> glyphs_bounding_box;
    be_uint16_t style;
    be_uint16_t lowest_rec_ppem;
    be_int16_t font_direction_hint;
    be_int16_t index_to_loc_fmt;
    be_int16_t glyph_data_fmt;
};

constexpr uint32_t TRUETYPE_MAGIC_NUMBER = 0x5F0F3CF5;

struct TrueTypeGlyphHeader
{
    be_int16_t num_contours;
    Math::Rect<be_int16_t> bounding_box;
};

enum TrueTypeGlyphSimpleFlags : uint8_t
{
    TT_GLYPH_ON_CURVE_POINT = (1 << 0),
    TT_GLYPH_XSHORT_VECTOR = (1 << 1),
    TT_GLYPH_YSHORT_VECTOR = (1 << 2),
    TT_GLYPH_REPEAT = (1 << 3),
    TT_GLYPH_X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR = (1 << 4),
    TT_GLYPH_Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR = (1 << 5),
    TT_GLYPH_OVERLAP_SIMPLE = (1 << 6)
};

enum TrueTypePlatform : uint16_t
{
    TT_PLATFORM_UNICODE = 0,
    TT_PLATFORM_MACINTOSH = 1,
    TT_PLATFORM_ISO = 2, // was deprecated
    TT_PLATFORM_WINDOWS = 3,
    TT_PLATFORM_CUSTOM = 4,
};

enum TrueTypeEncodingWindows : uint16_t
{
    TT_ENCODING_WIN_SYMBOL = 0,
    TT_ENCODING_WIN_UNICODE_BMP = 1,
    TT_ENCODING_WIN_SHIFTJIS = 2,
    TT_ENCODING_WIN_PRC = 3,
    TT_ENCODING_WIN_BIG5 = 4,
    TT_ENCODING_WIN_WANSUNG = 5,
    TT_ENCODING_WIN_JOHAB = 6,
    TT_ENCODING_WIN_UNICODE_FULL = 10,
};

enum TrueTypeMappingFormat : uint16_t
{
    TT_MAPPING_FMT_APPLE_BYTE_ENCODING = 0,
    TT_MAPPING_FMT_HIGH_BYTE = 2,
    TT_MAPPING_FMT_SEGMENT = 4,
    TT_MAPPING_FMT_TRIMMED_TABLE = 6,
    TT_MAPPING_FMT_MIXED = 8,
    TT_MAPPING_FMT_TRIMMED_ARRAY = 10,
    TT_MAPPING_FMT_SEGMENTED_COV = 12,
    TT_MAPPING_FMT_MANY_TO_ONE = 13,
    TT_MAPPING_FMT_UNICODE_VARIATIONS = 14
};
} // namespace Graphic::SuperCoolFont