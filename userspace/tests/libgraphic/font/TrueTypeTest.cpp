#include <libgraphic/font/Parser.h>
#include <libio/File.h>

#include "tests/Driver.h"

TEST(font_load_truetype)
{
    IO::File file("/Files/Tests/font/Roboto-Regular.ttf", OPEN_READ);
    Assert::is_true(file.exist());
    auto result = Graphic::SuperCoolFont::load(file);
    Assert::equal(result.result(), Result::SUCCESS);
}