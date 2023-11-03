#include <onix/chksum.h>

namespace arch
{
    u32 crc32(void *data, int len)
    {
        u32 crc = -1;
        u8 *ptr = (u8 *)data;
        for (int i = 0; i < len; i++)
        {
            crc ^= ptr[i];
            for (int j = 0; j < 8; j++)
            {
                if (crc & 1)
                    crc = (crc >> 1) ^ 0xEDB88320;
                else
                    crc >>= 1;
            }
        }
        return ~crc;
    }
}
