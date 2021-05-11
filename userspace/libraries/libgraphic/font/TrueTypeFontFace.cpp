#include <libgraphic/font/TrueTypeFontFace.h>
#include <libio/FourCC.h>
#include <libio/ReadCounter.h>
#include <libio/Skip.h>
#include <libio/Streams.h>
#include <libutils/Endian.h>
#include <libutils/Sort.h>

// Font tags
// TrueType Version 1
constexpr uint32_t TAG_TrueType_V_1 = IO::make_tag('1', 0, 0, 0);
// TrueType with Type 1 font
constexpr uint32_t TAG_TrueType_T_1 = IO::make_tag('t', 'y', 'p', '1');
// OpenType Version 1
constexpr uint32_t TAG_OpenType_V_1 = IO::make_tag(0, 1, 0, 0);
// OpenType with CFF
constexpr uint32_t TAG_OpenType_CFF = IO::make_tag('O', 'T', 'T', 'O');

// Table tags
constexpr uint32_t TABLE_TAG_CMAP = IO::make_tag('c', 'm', 'a', 'p');
constexpr uint32_t TABLE_TAG_LOCA = IO::make_tag('l', 'o', 'c', 'a');
constexpr uint32_t TABLE_TAG_HEAD = IO::make_tag('h', 'e', 'a', 'd');
constexpr uint32_t TABLE_TAG_GLYF = IO::make_tag('g', 'l', 'y', 'f');
constexpr uint32_t TABLE_TAG_HHEA = IO::make_tag('h', 'h', 'e', 'a');
constexpr uint32_t TABLE_TAG_HMTX = IO::make_tag('h', 'm', 't', 'x');
constexpr uint32_t TABLE_TAG_KERN = IO::make_tag('k', 'e', 'r', 'n');
constexpr uint32_t TABLE_TAG_GPOS = IO::make_tag('G', 'P', 'O', 'S');

// Optional table tags
constexpr uint32_t TABLE_TAG_MAXP = IO::make_tag('m', 'a', 'x', 'p');

// TODO: we don't require a memreader here really
ResultOr<RefPtr<Graphic::SuperCoolFont::TrueTypeFontFace>> Graphic::SuperCoolFont::TrueTypeFontFace::load(IO::Reader &reader)
{
    RefPtr<Graphic::SuperCoolFont::TrueTypeFontFace> result;
    auto data = TRY(IO::read_all(reader));
    IO::MemoryReader mem_reader(data);

    auto tag = TRY(IO::read_tag(mem_reader));
    switch (tag())
    {
    case TAG_TrueType_V_1:
    case TAG_OpenType_V_1:
        result = make<TrueTypeFontFace>();
        break;

    case TAG_TrueType_T_1:
    case TAG_OpenType_CFF:
        IO::logln("Not supported font formats yet");
        return Result::ERR_NOT_IMPLEMENTED;

    default:
        IO::logln("Unknown FourCC tag for fonts: {}", tag());
        return Result::ERR_INVALID_DATA;
    }

    // First read the tables metadata
    TRY(result->read_tables(mem_reader));

    // Check if we're missing any required data
    if (!result->has_table(TABLE_TAG_CMAP) ||
        !result->has_table(TABLE_TAG_HEAD) ||
        !result->has_table(TABLE_TAG_HHEA) ||
        !result->has_table(TABLE_TAG_HMTX))
    {
        IO::errln("Font is missing a required table!");
        return Result::ERR_INVALID_DATA;
    }

    if (result->has_table(TABLE_TAG_GLYF))
    {
        // required for truetype
        if (!result->has_table(TABLE_TAG_LOCA))
        {
            IO::errln("Font requires LOCA table when using GLYF table!");
            return Result::ERR_INVALID_DATA;
        }
    }
    else
    {
        IO::errln("Only supporting GLYF tables for glyph data current");
        return Result::ERR_NOT_IMPLEMENTED;
    }

    // Now actually parse all tables
    TRY(result->parse_tables(mem_reader));

    IO::logln("Font has {} glyphs", result->_num_glyphs);

    return result;
}

Result Graphic::SuperCoolFont::TrueTypeFontFace::read_tables(IO::MemoryReader &reader)
{
    auto num_tables = TRY(IO::read<be_uint16_t>(reader));
    IO::logln("Font contains {} tables", num_tables());

    // We don't really need any of these
    auto search_range = TRY(IO::read<be_uint16_t>(reader));
    auto entry_selector = TRY(IO::read<be_uint16_t>(reader));
    auto range_shift = TRY(IO::read<be_uint16_t>(reader));
    UNUSED(search_range);
    UNUSED(entry_selector);
    UNUSED(range_shift);

    _tables.resize(num_tables());

    for (uint16_t i = 0; i < num_tables(); i++)
    {
        // Read the table entry
        _tables[i] = TRY(IO::read<TrueTypeTable>(reader));
    }

    // Reorder the tables on how we need them to be parsed (Head & MaxP required before the others)
    _tables.sort([](auto a, auto b) {
        if (a.tag() == TABLE_TAG_HEAD)
            return false;

        if (a.tag() == TABLE_TAG_MAXP)
            return false;

        UNUSED(b);
        return true;
    });

    return Result::SUCCESS;
}

Result Graphic::SuperCoolFont::TrueTypeFontFace::parse_tables(IO::MemoryReader &reader)
{
    for (const auto &table : _tables)
    {
        // Jump to the table position and create our readers
        TRY(reader.seek(IO::SeekFrom::start(table.offset())));
        IO::ScopedReader table_reader(reader, table.size());

        // TODO: validate the checksum

        switch (table.tag())
        {
        case TABLE_TAG_CMAP:
            IO::logln("CMAP: {}", table.offset());
            TRY(read_table_cmap(table_reader));
            break;
        case TABLE_TAG_LOCA:
            IO::logln("LOCA: {}", table.offset());
            TRY(read_table_loca(table_reader));
            break;
        case TABLE_TAG_HEAD:
            IO::logln("HEAD: {}", table.offset());
            TRY(read_table_head(table_reader));
            break;
        case TABLE_TAG_GLYF:
            IO::logln("GLYF: {} {}", table.offset(), table.size());
            TRY(read_table_glyf(table_reader));
            break;
        case TABLE_TAG_HHEA:
            IO::logln("HHEA: {}", table.offset());
            break;
        case TABLE_TAG_HMTX:
            IO::logln("HMTX: {}", table.offset());
            break;
        case TABLE_TAG_MAXP:
            IO::logln("MAXP: {}", table.offset());
            TRY(read_table_maxp(table_reader));
            break;
        default:
            // IO::logln("Unknown table tag for font: {}", table_tag);
            break;
        }
    }

    return Result::SUCCESS;
}

String Graphic::SuperCoolFont::TrueTypeFontFace::family()
{
    return "";
}

Graphic::SuperCoolFont::FontStyle Graphic::SuperCoolFont::TrueTypeFontFace::style()
{
    return FONT_STYLE_REGULAR;
}

Optional<Graphic::SuperCoolFont::Glyph> Graphic::SuperCoolFont::TrueTypeFontFace::glyph(Codepoint c)
{
    if (!_codepoint_glyph_mapping.has_key(c))
    {
        return {};
    }

    // TODO: this is probably not correct yet
    auto idx = _codepoint_glyph_mapping[c];
    return _glyphs[idx];
}

ResultOr<Graphic::SuperCoolFont::TrueTypeVersion> Graphic::SuperCoolFont::TrueTypeFontFace::read_version(IO::Reader &reader)
{
    auto major = TRY(IO::read<be_uint16_t>(reader));
    auto minor = TRY(IO::read<be_uint16_t>(reader));

    return TrueTypeVersion{major, minor};
}

Result Graphic::SuperCoolFont::TrueTypeFontFace::read_table_cmap(IO::Reader &_reader)
{
    auto reader = IO::ReadCounter(_reader);
    auto version = TRY(IO::read<uint16_t>(reader));
    UNUSED(version);
    auto num_encodings = TRY(IO::read<be_uint16_t>(reader));
    IO::logln("CMAP num_encodings: {}", num_encodings());

    uint32_t encoding_table_offset = 0;

    // Find an encoding we can work with (Unicode)
    for (uint16_t i = 0; i < num_encodings() && encoding_table_offset == 0; i++)
    {
        auto platform_id = TRY(IO::read<be_uint16_t>(reader));
        auto encoding_id = TRY(IO::read<be_uint16_t>(reader));
        UNUSED(encoding_id);
        auto subtable_offset = TRY(IO::read<be_uint32_t>(reader));

        switch (platform_id())
        {
        case TT_PLATFORM_WINDOWS:
            if (encoding_id() == TT_ENCODING_WIN_UNICODE_BMP ||
                encoding_id() == TT_ENCODING_WIN_UNICODE_FULL)
            {
                encoding_table_offset = subtable_offset();
            }
            break;
        case TT_PLATFORM_UNICODE:
        {
            encoding_table_offset = subtable_offset();
        }
        default:
            break;
        }

        if (encoding_table_offset == 0)
        {
            IO::logln("Skipped unsupported encoding table (Platform: {} Encoding: {})", platform_id(), encoding_id());
        }
        else
        {
            IO::logln("Using encoding table (Platform: {} Encoding: {})", platform_id(), encoding_id());
        }
    }

    if (encoding_table_offset == 0)
    {
        IO::errln("Found no supported encoding table");
        return Result::ERR_INVALID_DATA;
    }

    // Skip to our position
    auto to_skip = encoding_table_offset - reader.count();
    IO::logln("Skipping {} bytes to subtable", to_skip);
    TRY(IO::skip(reader, to_skip));

    // Read the character mapping
    reader.reset();
    auto mapping_format = TRY(IO::read<be_uint16_t>(reader));
    IO::logln("Character mapping format: {}", mapping_format());
    auto length = TRY(IO::read<be_uint16_t>(reader));   // length of table in bytes
    auto language = TRY(IO::read<be_uint16_t>(reader)); // language
    UNUSED(language);

    switch (mapping_format())
    {
    case TT_MAPPING_FMT_APPLE_BYTE_ENCODING:
    {
        // We already the first 6 bytes of this table
        for (uint16_t i = 0; i < (length() - reader.count()) && i < 256; i++)
        {
            _codepoint_glyph_mapping[i] = TRY(IO::read<be_uint8_t>(reader))();
        }
    }
    break;
    case TT_MAPPING_FMT_SEGMENT:
    {
        auto seg_count = TRY(IO::read<be_uint16_t>(reader))() / 2;
        auto search_range = TRY(IO::read<be_uint16_t>(reader));
        UNUSED(search_range);
        auto entry_selector = TRY(IO::read<be_uint16_t>(reader));
        UNUSED(entry_selector);
        auto range_shift = TRY(IO::read<be_uint16_t>(reader));
        UNUSED(range_shift);

        auto end_codes = TRY(IO::read_vector<be_uint16_t>(reader, seg_count));
        TRY(IO::skip(reader, sizeof(uint16_t)));
        auto start_codes = TRY(IO::read_vector<be_uint16_t>(reader, seg_count));
        auto deltas = TRY(IO::read_vector<be_uint16_t>(reader, seg_count));
        auto range_offsets = TRY(IO::read_vector<be_uint16_t>(reader, seg_count));

        auto header_length = 16 + (seg_count * 8);
        auto num_glyphs = (length() - header_length) / sizeof(uint16_t);
        auto glyph_ids = TRY(IO::read_vector<be_uint16_t>(reader, num_glyphs));

        // Read all segments, last segment is a pseudo-segment (start & end both set to 0xFFFF)
        for (uint16_t current_seg = 0; current_seg < (seg_count - 1); current_seg++)
        {
            const auto &start_code = start_codes[current_seg]();
            const auto &end_code = end_codes[current_seg]();
            const auto &delta = deltas[current_seg]();
            const auto &range_offset = range_offsets[current_seg]();

            auto range = end_code - start_code;

            for (uint16_t i = 0; i <= range; i++)
            {
                uint16_t glyph_index = 0;
                if (range_offset == 0)
                {
                    glyph_index = start_code + i + delta;
                }
                else
                {
                    glyph_index = glyph_ids[(range_offset / 2) + i - seg_count + current_seg]();
                }

                _codepoint_glyph_mapping[start_code + i] = glyph_index;
            }
        }
    }
    break;
    default:
        IO::errln("Unsupported mapping format: {}", mapping_format());
        return Result::ERR_NOT_IMPLEMENTED;
    }

    return Result::SUCCESS;
}

Result Graphic::SuperCoolFont::TrueTypeFontFace::read_table_head(IO::Reader &reader)
{
    _header = TRY(IO::read<TrueTypeHeader>(reader));

    if (_header.magic_number() != TRUETYPE_MAGIC_NUMBER)
    {
        return Result::ERR_INVALID_DATA;
    }

    // TODO: use what we need

    return Result::SUCCESS;
}

Result Graphic::SuperCoolFont::TrueTypeFontFace::read_table_maxp(IO::Reader &reader)
{
    auto version = TRY(read_version(reader));

    _num_glyphs = TRY(IO::read<be_uint16_t>(reader))();

    // Version 1.0 has extra data
    if (version.x()() == 1 && version.y()() == 0)
    {
        // TODO: read extra data for version 1.0
        // https://docs.microsoft.com/en-us/typography/opentype/spec/maxp
    }

    return Result::SUCCESS;
}

Result Graphic::SuperCoolFont::TrueTypeFontFace::read_table_loca(IO::Reader &reader)
{
    _glyph_offsets.resize(_num_glyphs + 1);
    for (uint16_t i = 0; i < _num_glyphs + 1; i++)
    {
        if (_header.glyph_data_fmt() == 0)
        {
            _glyph_offsets[i] = TRY(IO::read<be_uint16_t>(reader))() * 2;
        }
        else
        {
            _glyph_offsets[i] = TRY(IO::read<be_uint32_t>(reader))();
        }
    }

    return Result::SUCCESS;
}

Result Graphic::SuperCoolFont::TrueTypeFontFace::read_table_glyf(IO::Reader &reader)
{
    auto glyph_data = TRY(IO::read_all(reader));
    IO::MemoryReader glyph_reader(glyph_data);
    _glyphs.resize(_num_glyphs);

    for (uint16_t glyph_idx = 0; glyph_idx < _num_glyphs; glyph_idx++)
    {
        if (_glyph_offsets[glyph_idx] == _glyph_offsets[glyph_idx + 1])
        {
            IO::logln("This is an empty glyph");
            continue;
        }

        TRY(glyph_reader.seek(IO::SeekFrom::start(_glyph_offsets[glyph_idx])));
        auto header = TRY(IO::read<TrueTypeGlyphHeader>(glyph_reader));

        // Simple glyph
        if (header.num_contours() > 0)
        {
            auto end_indices = TRY(IO::read_vector<be_uint16_t>(glyph_reader, header.num_contours()));
            auto num_instructions = TRY(IO::read<be_uint16_t>(glyph_reader));
            auto instructions = TRY(IO::read_vector<be_uint8_t>(glyph_reader, num_instructions()));

            // Number of points is the end index of the last contour
            auto num_points = end_indices.peek_back();

            // TODO: resize these in advance & put them in a single vector (Vec2<int16_t>)
            Vector<TrueTypeGlyphSimpleFlags> flags_vec(num_points());
            Vector<Math::Vec2<int16_t>> coords(num_points());
            coords.resize(num_points());

            // Read flags
            {
                uint8_t repeat_count = 0;
                for (uint16_t point_idx = 0; point_idx < num_points(); point_idx++)
                {
                    if (repeat_count == 0)
                    {
                        auto flags = TRY(IO::read<TrueTypeGlyphSimpleFlags>(glyph_reader));
                        flags_vec.push_back(flags);
                        // This flags tells us that the flag is repeated N times
                        if (flags & TT_GLYPH_REPEAT)
                        {
                            repeat_count = TRY(IO::read<be_uint8_t>(glyph_reader))();
                        }
                    }
                    else
                    {
                        flags_vec.push_back(flags_vec.peek_back());
                        repeat_count--;
                    }
                }
            }

            // Read x coordinates
            {
                int32_t x = 0;
                for (uint16_t point_idx = 0; point_idx < num_points(); point_idx++)
                {
                    const auto &flags = flags_vec[point_idx];
                    // It's a new coordinate (uint8_t + signflag)
                    if (flags & TT_GLYPH_XSHORT_VECTOR)
                    {
                        int16_t delta_x = TRY(IO::read<be_uint8_t>(glyph_reader))();
                        const bool positive = flags & TT_GLYPH_X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR;
                        x += positive ? delta_x : -delta_x;
                    }
                    // It's a new coordinate (int16_t)
                    else if (!(flags & TT_GLYPH_X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR))
                    {
                        int16_t delta_x = TRY(IO::read<be_int16_t>(glyph_reader))();
                        x += delta_x;
                    }
                    coords[point_idx].x() = x;
                }
            }

            // Read y coordinates
            {
                int32_t y = 0;
                for (uint16_t point_idx = 0; point_idx < num_points(); point_idx++)
                {
                    const auto &flags = flags_vec[point_idx];
                    // It's a new coordinate (uint8_t + signflag)
                    if (flags & TT_GLYPH_YSHORT_VECTOR)
                    {
                        int16_t delta_y = TRY(IO::read<be_uint8_t>(glyph_reader))();
                        const bool positive = flags & TT_GLYPH_Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR;
                        y += positive ? delta_y : -delta_y;
                    }
                    // It's a new coordinate (int16_t)
                    else if (!(flags & TT_GLYPH_Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR))
                    {
                        int16_t delta_y = TRY(IO::read<be_int16_t>(glyph_reader))();
                        y += delta_y;
                    }
                    coords[point_idx].y() = y;
                }
            }

            // Now create our contours
            auto &glyph = _glyphs[glyph_idx];
            glyph.edges.begin();
            for (uint16_t point_idx = 0; point_idx < num_points(); point_idx++)
            {
                Math::Vec2f scaled_point = {coords[point_idx].x() / (float)_header.units_per_em(),
                                            coords[point_idx].y() / (float)_header.units_per_em()};
                glyph.edges.append(scaled_point);

                // This contour is done    
                if(point_idx == end_indices.peek()())
                {
                    glyph.edges.end();
                    glyph.edges.begin();
                    end_indices.pop();
                }
            }
            glyph.edges.end();
            // Only the last indice / end indice should be left in there
            Assert::equal(end_indices.count(), 1);

            // Not store the bounds
            auto minX = header.bounding_box.x()();
            auto minY = header.bounding_box.y()();
            auto maxX = header.bounding_box.width()();
            auto maxY = header.bounding_box.height()();
            glyph.bounds = Math::Recti(minX, minY, maxX - minX, maxY - minY);
        }
        else // Composite glyph
        {
        }
    }

    return Result::SUCCESS;
}