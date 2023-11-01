#include "mui_element.h"
#include "m-string.h"
#include "mui_defines.h"

void mui_element_scrollbar(mui_canvas_t *p_canvas, uint32_t pos, uint16_t total)
{
    uint8_t width = mui_canvas_get_width(p_canvas);
    uint8_t height = mui_canvas_get_height(p_canvas);
    // prevent overflows
    mui_canvas_set_draw_color(p_canvas, 0);
    mui_canvas_draw_box(p_canvas, width - 4, 0, 4, height);
    // dot line
    mui_canvas_set_draw_color(p_canvas, 1);
    for (uint8_t i = 0; i < height; i += 2)
    {
        mui_canvas_draw_dot(p_canvas, width - 2, i);
    }
    // Position block
    if (total)
    {
        float block_h = ((float)height) / total;
        mui_canvas_draw_box(p_canvas, width - 3, block_h * pos, 3, block_h > 1 ? block_h : 1);
    }
}

void mui_element_multiline_text(mui_canvas_t *p_canvas, uint8_t x, uint8_t y, const char *text)
{

    uint8_t font_height = mui_canvas_current_font_height(p_canvas);
    string_t str;
    string_init(str);
    const char *start = text;
    char *end;
    do
    {
        end = strchr(start, '\n');
        if (end)
        {
            string_set_strn(str, start, end - start);
        }
        else
        {
            string_set_str(str, start);
        }
        mui_canvas_draw_utf8(p_canvas, x, y, string_get_cstr(str));
        start = end + 1;
        y += font_height;
    } while (end && y < 64);
    string_clear(str);
}

void mui_element_autowrap_text(mui_canvas_t *p_canvas, uint8_t x, uint8_t y, uint8_t w, uint8_t h, const char *text)
{
    uint8_t font_height = mui_canvas_current_font_height(p_canvas);
    uint8_t xi = x;
    uint8_t yi = y;

    char *p = text;
    char utf8[5];

    while (*p != 0 && yi < y + h)
    {

        uint8_t utf8_size = mui_canvas_get_utf8_bytes(p);
        memcpy(utf8, p, utf8_size);
        utf8[utf8_size] = '\0';
        uint8_t utf8_x = mui_canvas_get_utf8_width(p_canvas, utf8);
        if (utf8_x + xi > x + w)
        {
            xi = x;
            yi += font_height;
        }
        uint8_t utf8_w = mui_canvas_draw_utf8(p_canvas, xi, yi, utf8);
        xi += utf8_w;
        p += utf8_size;
    }
}

uint16_t mui_element_text_height(mui_canvas_t *p_canvas, uint8_t w, const char *text)
{
    uint8_t font_height = mui_canvas_current_font_height(p_canvas);
    uint8_t xi = 0;
    uint16_t yi = 0;

    char *p = text;
    char utf8[5];

    while (*p != 0)
    {

        uint8_t utf8_size = mui_canvas_get_utf8_bytes(p);
        memcpy(utf8, p, utf8_size);
        utf8[utf8_size] = '\0';
        uint8_t utf8_x = mui_canvas_get_utf8_width(p_canvas, utf8);
        if (utf8_x + xi > w)
        {
            xi = 0;
            yi += font_height;
        }
        xi += utf8_x;
        p += utf8_size;
    }
    return yi + font_height;
}

void mui_element_button(mui_canvas_t *p_canvas, uint8_t x, uint8_t y, const char *str, uint8_t selected)
{
    uint8_t font_height = mui_canvas_current_font_height(p_canvas);
    uint8_t canvas_width = mui_canvas_get_width(p_canvas);
    uint8_t canvas_height = mui_canvas_get_height(p_canvas);
    uint8_t button_width = mui_canvas_get_utf8_width(p_canvas, str);

    if (selected)
    {
        mui_canvas_set_draw_color(p_canvas, 1);
        mui_canvas_draw_box(p_canvas, x, y + 2, button_width + 4, font_height + 2);
        mui_canvas_set_draw_color(p_canvas, 0);
        mui_canvas_draw_utf8(p_canvas, x + 2, y + font_height, str);
        mui_canvas_set_draw_color(p_canvas, 1);
    }
    else
    {
        mui_canvas_set_draw_color(p_canvas, 1);
        mui_canvas_draw_rframe(p_canvas, x, y + 2, button_width + 4, font_height, 1);
        mui_canvas_draw_utf8(p_canvas, x + 2, y + font_height, str);
    }
}