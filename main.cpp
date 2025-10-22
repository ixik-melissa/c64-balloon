
#include <cstdio>
#include <cstdint>

namespace
{
    struct c64_hardware
    {
        volatile std::uint8_t* base_address = reinterpret_cast<std::uint8_t*>(0x0000);
        volatile std::uint8_t* vic_base = base_address + 0xD000;
        volatile std::uint8_t* screen_base = base_address + 0x0400;
        std::uint8_t screen_text_columns = 40;
        std::uint8_t screen_text_rows = 25;
        std::uint8_t sprite_block_size = 64;
        std::uint8_t sprite_size_bytes = 63;
    } c64;

    auto clear_screen()
    {
        auto const screen_size = c64.screen_text_columns * c64.screen_text_rows;
        for (auto i = 0; i < screen_size; ++i)
            *(c64.screen_base + i) = 32; // PETSCII space
    }

    auto jiffy_wait(std::uint8_t time_in_sixtieth)
    {
        // https://llvm-mos.org/wiki/C_Inline_Assembly
        asm volatile (
            "  LDA #05\n"
            "  CLC\n"
            "  ADC $A2\n"
            "JIFFLOOP:\n"
            "  CMP $A2\n"
            "  BNE JIFFLOOP\n"
            : /* no output */
            : "a" (time_in_sixtieth)
        );
    }

    auto load_sprite(std::uint8_t sprite_block, std::uint8_t const* sprite_data)
    {
        auto* sprite_memory = c64.base_address + sprite_block * c64.sprite_block_size;
        for (auto i = 0; i < c64.sprite_size_bytes; ++i)
            *(sprite_memory + i) = sprite_data[i];
    }

    auto load_balloon(std::uint8_t sprite_block)
    {
        std::uint8_t const sprite_data[63] = {
            0,127,0,
            1,255,192,
            3,255,224,
            3,231,224,
            7,217,240,
            7,223,240,
            7,217,240,
            3,231,224,
            3,255,224,
            3,255,224,
            2,255,160,
            1,127,64,
            1,62,64,
            0,156,128,
            0,156,128,
            0,73,0,
            0,73,0,
            0,62,0,
            0,62,0,
            0,62,0,
            0,28,0
        };
        load_sprite(sprite_block, sprite_data);
    }

    auto define_block_for_sprite(std::uint8_t sprite_number, std::uint8_t sprite_block)
    {
        *(c64.base_address + 0x7F8 + sprite_number) = sprite_block;
    }

    auto enable_sprite(std::uint8_t sprite_number)
    {
        *(c64.vic_base + 0x15) |= (1 << sprite_number);
    }

    auto enable_double_sprites(std::uint8_t sprite_number)
    {
        *(c64.vic_base + 0x17) |= (1 << sprite_number);
        *(c64.vic_base + 0x1D) |= (1 << sprite_number);
    }

    auto move_sprite_around(std::uint8_t sprite_number)
    {
        auto* sprite_pos_x = c64.vic_base + 2 * sprite_number + 0;
        auto* sprite_pos_y = c64.vic_base + 2 * sprite_number + 1;

        for (;;)
        {
            for (std::uint8_t x = 0; x <= 200; ++x) 
            {
                *sprite_pos_x = x;
                *sprite_pos_y = x;
                jiffy_wait(30);
            }
        }
    }
}

int main()
{
    clear_screen();
    auto const sprite_number = 2;
    auto const sprite_block = 13;
    load_balloon(sprite_block);
    define_block_for_sprite(sprite_number, sprite_block);
    enable_sprite(sprite_number);
    enable_double_sprites(sprite_number);
    move_sprite_around(sprite_number);
}
