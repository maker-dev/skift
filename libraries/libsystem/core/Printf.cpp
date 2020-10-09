
// printf.c : printf and snprintf internals

#include <libsystem/core/CString.h>
#include <libsystem/core/CType.h>
#include <libsystem/core/Printf.h>
#include <libsystem/utils/NumberFormatter.h>

int __printf_formate_binary(printf_info_t *info, va_list *va)
{
    uint v = va_arg(*va, uint);

    char buffer[33] = {};

    NumberFormater format = FORMAT_BINARY;
    format.padded_with_zero = false;

    format_uint(format, v, buffer, 33);

    PRINTF_PADDING(buffer, PFALIGN_RIGHT);

    for (int i = 0; buffer[i]; i++)
    {
        PRINTF_APPEND(buffer[i]);
    }

    PRINTF_PADDING(buffer, PFALIGN_LEFT);

    return info->written;
}

int __printf_formate_octal(printf_info_t *info, va_list *va)
{
    uint v = va_arg(*va, uint);

    char buffer[33] = {};

    NumberFormater format = FORMAT_OCTAL;
    format.padded_with_zero = false;

    format_uint(format, v, buffer, 33);

    PRINTF_PADDING(buffer, PFALIGN_RIGHT);

    for (int i = 0; buffer[i]; i++)
    {
        PRINTF_APPEND(buffer[i]);
    }

    PRINTF_PADDING(buffer, PFALIGN_LEFT);

    return info->written;
}

int __printf_formate_decimal(printf_info_t *info, va_list *va)
{
    int v = va_arg(*va, int);

    char buffer[33] = {};

    NumberFormater format = FORMAT_DECIMAL;
    format.padded_with_zero = false;

    format_int(format, v, buffer, 33);

    PRINTF_PADDING(buffer, PFALIGN_RIGHT);

    for (int i = 0; buffer[i]; i++)
    {
        PRINTF_APPEND(buffer[i]);
    }

    PRINTF_PADDING(buffer, PFALIGN_LEFT);

    return info->written;
}

int __printf_formate_decimal_unsigned(printf_info_t *info, va_list *va)
{
    uint v = va_arg(*va, uint);

    char buffer[33] = {};

    NumberFormater format = FORMAT_DECIMAL;
    format.padded_with_zero = false;

    format_uint(format, v, buffer, 33);

    PRINTF_PADDING(buffer, PFALIGN_RIGHT);

    for (int i = 0; buffer[i]; i++)
    {
        PRINTF_APPEND(buffer[i]);
    }

    PRINTF_PADDING(buffer, PFALIGN_LEFT);

    return info->written;
}

int __printf_formate_hexadecimal(printf_info_t *info, va_list *va)
{
    uint v = va_arg(*va, uint);

    char buffer[33] = {};

    NumberFormater format = FORMAT_HEXADECIMAL;
    format.padded_with_zero = false;

    format_uint(format, v, buffer, 33);

    PRINTF_PADDING(buffer, PFALIGN_RIGHT);

    for (int i = 0; buffer[i]; i++)
    {
        PRINTF_APPEND(buffer[i]);
    }

    PRINTF_PADDING(buffer, PFALIGN_LEFT);

    return info->written;
}

int __printf_formate_char(printf_info_t *info, va_list *va)
{
    char v = va_arg(*va, int);

    // FIXME: support unicode
    char buffer[2] = {v, 0};

    PRINTF_PADDING(buffer, PFALIGN_RIGHT);

    for (int i = 0; buffer[i]; i++)
    {
        PRINTF_APPEND(buffer[i]);
    }

    PRINTF_PADDING(buffer, PFALIGN_LEFT);

    return info->written;
}

int __printf_formate_string(printf_info_t *info, va_list *va)
{
    const char *v = va_arg(*va, char *);

    if (v == nullptr)
    {
        v = "(null)";
    }

    PRINTF_PADDING(v, PFALIGN_RIGHT);

    for (int i = 0; v[i]; i++)
    {
        PRINTF_APPEND(v[i]);
    }

    PRINTF_PADDING(v, PFALIGN_LEFT);

    return info->written;
}

static printf_formatter_t formaters[] = {
    /* Binary        */ {'b', __printf_formate_binary},
    /* Octal         */ {'o', __printf_formate_octal},
    /* Decimal       */ {'d', __printf_formate_decimal},
    /* Decimal       */ {'i', __printf_formate_decimal},
    /* Decimal       */ {'u', __printf_formate_decimal_unsigned},
    /* Hexadecimal   */ {'x', __printf_formate_hexadecimal},
    /* Hexadecimal   */ {'p', __printf_formate_hexadecimal},

    /* Float         */ {'f', nullptr},

    /* Char          */ {'c', __printf_formate_char},
    /* String        */ {'s', __printf_formate_string},

    /* End of the list */ {'\0', nullptr},
};

void __printf_formate(printf_info_t *info, char c, va_list *va)
{
    for (int i = 0; formaters[i].c; i++)
    {
        if (formaters[i].c == c && formaters[i].impl != nullptr)
        {
            formaters[i].impl(info, va);
            return;
        }
    }

    // For unknown format string just put into the output.
    const int trash = va_arg(*va, int);
    __unused(trash);
    info->append(info, '%');
    info->append(info, c);
}

int __printf(printf_info_t *info, va_list va)
{
    info->written = 0;
    info->format_offset = 0;
    info->state = PFSTATE_ESC;
    info->align = PFALIGN_RIGHT;
    info->padding = ' ';
    info->length = 0;

    if (info->format == nullptr)
    {
        for (int i = 0; "(null)"[i]; i++)
        {
            PRINTF_APPEND("(null)"[i]);
        }
        return 0;
    }

    PRINTF_PEEK();

    while (1)
    {
        switch (info->state)
        {
        case PFSTATE_ESC:
            if (info->c == '%')
            {
                info->state = PFSTATE_PARSE;
            }
            else
            {
                PRINTF_APPEND(info->c);
            }

            PRINTF_PEEK();
            break;

        case PFSTATE_PARSE:
            if (info->c == '0')
            {
                info->padding = '0';
                PRINTF_PEEK();
            }
            else if (info->c == '-')
            {
                info->align = PFALIGN_LEFT;
                PRINTF_PEEK();
            }
            else if (info->c == '%')
            {
                info->state = PFSTATE_ESC;
                PRINTF_APPEND('%');
                PRINTF_PEEK();
            }
            else if (isdigit(info->c))
            {
                info->state = PFSTATE_FORMAT_LENGTH;
            }
            else if (isalpha(info->c))
            {
                info->state = PFSTATE_FINALIZE;
            }
            else
            {
                PRINTF_PEEK();
            }
            break;

        case PFSTATE_FORMAT_LENGTH:
            if (isdigit(info->c))
            {
                info->length *= 10;
                info->length += info->c - '0';

                PRINTF_PEEK();
            }
            else if (isalpha(info->c))
            {
                info->state = PFSTATE_PARSE;
            }
            else
            {
                info->state = PFSTATE_ESC;
            }
            break;

        case PFSTATE_FINALIZE:
            // In x86_64 va_list must be copied. Don't ask me why
            va_list va_cp;
            va_copy(va_cp, va);
            __printf_formate(info, info->c, &va_cp);

            if (info->written == info->allocated)
            {
                return info->written;
            }
            else
            {
                info->length = 0;
                info->state = PFSTATE_ESC;
                info->padding = ' ';
                info->align = PFALIGN_RIGHT;

                PRINTF_PEEK();
            }
            break;

        default:
            break;
        }
    }
}
