#include <libgraphic/font/Parser.h>
#include <libgraphic/font/TrueTypeFontFace.h>

ResultOr<RefPtr<Graphic::SuperCoolFont::FontFace>> Graphic::SuperCoolFont::load(IO::Reader &reader)
{
    auto result = TrueTypeFontFace::load(reader);
    if(!result.success())
    {
        return result.result();
    }

    RefPtr<FontFace> tmp = result.unwrap();
    return tmp;
}
