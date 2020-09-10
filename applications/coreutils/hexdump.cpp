#include <libsystem/io/Stream.h>

int main(int argc, char **argv)
{
    if(argc == 1)
    {
        printf("Usage: %s FILENAME", argv[0]);
        return -1;
    }

    Stream *stream = stream_open(argv[1], OPEN_READ);
    
    if(handle_has_error(stream))
    {
        handle_printf_error(stream,  "Couldn't read %s", argv[1]);
        return -1;
    }

    size_t read;
    size_t offset = 0;
    uint8_t buffer[16];

    while((read = stream_read(stream, buffer, 16)) != 0)
    {
        printf("%08x ", offset*16);
        for(size_t i = 0; i < 16;  i++)
        {
            printf("%02x ", buffer[i]);
        }

        printf("\n");
        offset++;

        if(handle_has_error(out_stream))
        {
            return -1;
        }
    }
}

