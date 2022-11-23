#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "debug.h"

unsigned long long memmask;
int memoffset;
void *vtpa(unsigned long long vaddr,int fd)
{
    void *p = NULL;
    memmask = vaddr & ~(0xffff);
    memoffset = vaddr & (0xffff);
    p = (void*)mmap(NULL,0x10000/*64K*/, PROT_READ|PROT_WRITE,MAP_SHARED,fd,memmask);
    p = p + memoffset;
    printf("mmap addr start : %p \n",p);
    return p;
}


int releaseMem(void *p)
{
    int status ;
    status = munmap(p-memoffset, 1);
    if(status == -1){
        printf("----------  Release mem Map Error !!! ------\n");
    }
    printf("--------------Release mem Map----------------\n");
    return status;
}

void hexdump (unsigned long bse, char *buf, int len)
{
    int pos;
    char line[80];

    while (len > 0)
    {
        int cnt, i;

        pos = snprintf (line, sizeof (line), "%08lx  ", bse);
        cnt = 16;
        if (cnt > len)
            cnt = len;

        for (i = 0; i < cnt; i++)
        {
            pos += snprintf (&line[pos], sizeof (line) - pos,
                    "%02x ", (unsigned char) buf[i]);
            if ((i & 7) == 7)
                line[pos++] = ' ';
        }

        for (; i < 16; i++)
        {
            pos += snprintf (&line[pos], sizeof (line) - pos, "   ");
            if ((i & 7) == 7)
                line[pos++] = ' ';
        }

        line[pos++] = '|';

        for (i = 0; i < cnt; i++)
            line[pos++] = ((buf[i] >= 32) && (buf[i] < 127)) ? buf[i] : '.';

        line[pos++] = '|';

        line[pos] = 0;

        printf ("%s\n", line);

        /* Print only first and last line if more than 3 lines are identical.  */
        if (len >= 4 * 16
                && ! memcmp (buf, buf + 1 * 16, 16)
                && ! memcmp (buf, buf + 2 * 16, 16)
                && ! memcmp (buf, buf + 3 * 16, 16))
        {
            printf ("*\n");
            do
            {
                bse += 16;
                buf += 16;
                len -= 16;
            }
            while (len >= 3 * 16 && ! memcmp (buf, buf + 2 * 16, 16));
        }

        bse += 16;
        buf += 16;
        len -= cnt;
    }
}
