#include "bootpack.h"

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize)
{
    struct SHTCTL *ctl;
    int i;
    ctl = (struct SHTCTL *)memman_alloc_4k(memman, sizeof(struct SHTCTL));
    if (ctl == 0)
    {
        goto err;
    }
    ctl->vram = vram;
    ctl->xsize = xsize;
    ctl->ysize = ysize;
    ctl->top = -1; /* no top height initialzation */
    for (i = 0; i < MAX_SHEETS; i++)
    {
        ctl->sheets0[i].flags = 0; /* unused */
    }
err:
    return ctl;
}

#define SHEET_USE 1
struct SHEET *sheet_alloc(struct SHTCTL *ctl) // get one free sheet
{
    struct SHEET *sht;
    int i;
    for (i = 0; i < MAX_SHEETS; i++)
    {
        if (ctl->sheets0[i].flags == 0)
        {
            sht = &(ctl->sheets0[i]);
            sht->flags = SHEET_USE; /* mark to 'used'*/
            sht->height = -1;       /* undefined now */
            return sht;
        }
    }
    return 0; /* no free sheet */
}

void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv)
{
    sht->buf = buf;
    sht->bxsize = xsize;
    sht->bysize = ysize;
    sht->col_inv = col_inv;
    return;
}

void sheet_updown(struct SHTCTL *ctl, struct SHEET *sht, int height)
{ // give a sheet and its height, rearrange the order of layers and repaint

    int h, old = sht->height; /* 存储设置前的高度信息 */
    /* 如果指定的高度过高或过低，则进行修正 */
    if (height > ctl->top + 1)
    {
        height = ctl->top + 1;
    }
    if (height < -1)
    {
        height = -1;
    }
    sht->height = height; /* 设定高度 */
    /* 下面主要是进行sheets[ ]的重新排列 */
    if (old > height)
    { /* 比以前低 */
        if (height >= 0)
        {
            /* 把中间的往上提 */
            for (h = old; h > height; h--)
            {
                ctl->sheets[h] = ctl->sheets[h - 1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht;
        }
        else
        { /* 隐藏 */
            if (ctl->top > old)
            {
                /* 把上面的降下来 */
                for (h = old; h < ctl->top; h++)
                {
                    ctl->sheets[h] = ctl->sheets[h + 1];
                    ctl->sheets[h]->height = h;
                }
            }
            ctl->top--; /* 由于显示中的图层减少了一个，所以最上面的图层高度下降 */
        }

        sheet_refresh(ctl); /* 按新图层的信息重新绘制画面 */
    }
    else if (old < height)
    { /* 比以前高 */
        if (old >= 0)
        {
            /* 把中间的拉下去 */
            for (h = old; h < height; h++)
            {
                ctl->sheets[h] = ctl->sheets[h + 1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht;
        }
        else
        { /* 由隐藏状态转为显示状态 */
            /* 将已在上面的提上来 */
            for (h = ctl->top; h >= height; h--)
            {
                ctl->sheets[h + 1] = ctl->sheets[h];
                ctl->sheets[h + 1]->height = h + 1;
            }
            ctl->sheets[height] = sht;
            ctl->top++; /* 由于已显示的图层增加了1个，所以最上面的图层高度增加 */
        }
        sheet_refresh(ctl); /* 按新图层信息重新绘制画面 */
    }

    return;
}

void sheet_slide(struct SHTCTL *ctl, struct SHEET *sht, int vx0, int vy0) 
{ //move on the same layer(height)
    sht->vx0 = vx0; 
    sht->vy0 = vy0; 
    if (sht->height >= 0) { /* if is on display*/ 
        sheet_refresh(ctl); /* refresh */ 
    } 
    return; 
} 

void sheet_refresh(struct SHTCTL *ctl)
{
    int h, bx, by, vx, vy;
    unsigned char *buf, c, *vram = ctl->vram;
    struct SHEET *sht;
    for (h = 0; h <= ctl->top; h++)
    {
        sht = ctl->sheets[h];
        buf = sht->buf;
        for (by = 0; by < sht->bysize; by++)
        {
            vy = sht->vy0 + by;
            for (bx = 0; bx < sht->bxsize; bx++)
            {
                vx = sht->vx0 + bx;
                c = buf[by * sht->bxsize + bx];
                if (c != sht->col_inv)
                {
                    vram[vy * ctl->xsize + vx] = c;
                }
            }
        }
    }
    return;
}

void sheet_free(struct SHTCTL *ctl, struct SHEET *sht) 
{ 
    if (sht->height >= 0) { 
        sheet_updown(ctl, sht, -1); /* if is on display, set it hidden state */ 
    } 
    sht->flags = 0; /* 'unsed' */ 
    return; 
} 
