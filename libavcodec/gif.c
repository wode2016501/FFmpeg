/*
 * GIF encoder.
 * Copyright (c) 2000 Fabrice Bellard
 * Copyright (c) 2002 Francois Revol
 * Copyright (c) 2006 Baptiste Coudurier
 *
 * first version by Francois Revol <revol@free.fr>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Features and limitations:
 * - uses only a global standard palette
 * - tested with IE 5.0, Opera for BeOS, NetPositive (BeOS), and Mozilla (BeOS).
 *
 * Reference documents:
 * http://www.goice.co.jp/member/mo/formats/gif.html
 * http://astronomy.swin.edu.au/pbourke/dataformats/gif/
 * http://www.dcs.ed.ac.uk/home/mxr/gfx/2d/GIF89a.txt
 */

#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"
#include "lzw.h"

/* The GIF format uses reversed order for bitstreams... */
/* at least they don't use PDP_ENDIAN :) */
#define BITSTREAM_WRITER_LE

#include "put_bits.h"

typedef struct {
    AVFrame picture;
    LZWState *lzw;
    uint8_t *buf;
    AVFrame *last_frame;
} GIFContext;

static int gif_image_write_image(AVCodecContext *avctx,
                                 uint8_t **bytestream, uint8_t *end,
                                 const uint32_t *palette,
                                 const uint8_t *buf, int linesize)
{
    GIFContext *s = avctx->priv_data;
    int len = 0, height = avctx->height, width = avctx->width, y;
    int x_start = 0, y_start = 0;
    const uint8_t *ptr;

    /* Mark one colour as transparent if the input palette contains at least
     * one colour that is more than 50% transparent. */
    if (palette) {
        unsigned i, smallest_alpha = 0xFF, alpha_component = 0;
        for (i = 0; i < AVPALETTE_COUNT; i++) {
            const uint32_t v = palette[i];
            if (v >> 24 < smallest_alpha) {
                smallest_alpha = v >> 24;
                alpha_component = i;
            }
        }
        if (smallest_alpha < 128) {
            bytestream_put_byte(bytestream, 0x21); /* Extension Introducer */
            bytestream_put_byte(bytestream, 0xf9); /* Graphic Control Label */
            bytestream_put_byte(bytestream, 0x04); /* block length */
            bytestream_put_byte(bytestream, 0x01); /* Transparent Color Flag */
            bytestream_put_le16(bytestream, 0x00); /* no delay */
            bytestream_put_byte(bytestream, alpha_component);
            bytestream_put_byte(bytestream, 0x00);
        }
    }

    /* Crop image */
    // TODO support with palette change
    if (s->last_frame && !palette) {
        const uint8_t *ref = s->last_frame->data[0];
        const int ref_linesize = s->last_frame->linesize[0];
        int x_end = avctx->width  - 1,
            y_end = avctx->height - 1;

        /* skip common lines */
        while (y_start < height) {
            if (memcmp(ref + y_start*ref_linesize, buf + y_start*linesize, width))
                break;
            y_start++;
        }
        while (y_end > y_start) {
            if (memcmp(ref + y_end*ref_linesize, buf + y_end*linesize, width))
                break;
            y_end--;
        }
        height = y_end + 1 - y_start;

        /* skip common columns */
        while (x_start < width) {
            int same_column = 1;
            for (y = y_start; y < y_end; y++) {
                if (ref[y*ref_linesize + x_start] != buf[y*linesize + x_start]) {
                    same_column = 0;
                    break;
                }
            }
            if (!same_column)
                break;
            x_start++;
        }
        while (x_end > x_start) {
            int same_column = 1;
            for (y = y_start; y < y_end; y++) {
                if (ref[y*ref_linesize + x_end] != buf[y*linesize + x_end]) {
                    same_column = 0;
                    break;
                }
            }
            if (!same_column)
                break;
            x_end--;
        }
        width = x_end + 1 - x_start;

        av_log(avctx, AV_LOG_DEBUG,"%dx%d image at pos (%d;%d) [area:%dx%d]\n",
               width, height, x_start, y_start, avctx->width, avctx->height);
    }

    /* image block */
    bytestream_put_byte(bytestream, 0x2c);
    bytestream_put_le16(bytestream, x_start);
    bytestream_put_le16(bytestream, y_start);
    bytestream_put_le16(bytestream, width);
    bytestream_put_le16(bytestream, height);

    if (!palette) {
    bytestream_put_byte(bytestream, 0x00); /* flags */
    } else {
        unsigned i;
        bytestream_put_byte(bytestream, 1<<7 | 0x7); /* flags */
        for (i = 0; i < AVPALETTE_COUNT; i++) {
            const uint32_t v = palette[i];
            bytestream_put_be24(bytestream, v);
        }
    }

    bytestream_put_byte(bytestream, 0x08);

    ff_lzw_encode_init(s->lzw, s->buf, width * height,
                       12, FF_LZW_GIF, put_bits);

    ptr = buf + y_start*linesize + x_start;
    for (y = 0; y < height; y++) {
        len += ff_lzw_encode(s->lzw, ptr, width);
        ptr += linesize;
    }
    len += ff_lzw_encode_flush(s->lzw, flush_put_bits);

    ptr = s->buf;
    while (len > 0) {
        int size = FFMIN(255, len);
        bytestream_put_byte(bytestream, size);
        if (end - *bytestream < size)
            return -1;
        bytestream_put_buffer(bytestream, ptr, size);
        ptr += size;
        len -= size;
    }
    bytestream_put_byte(bytestream, 0x00); /* end of image block */
    return 0;
}

static av_cold int gif_encode_init(AVCodecContext *avctx)
{
    GIFContext *s = avctx->priv_data;

    if (avctx->width > 65535 || avctx->height > 65535) {
        av_log(avctx, AV_LOG_ERROR, "GIF does not support resolutions above 65535x65535\n");
        return -1;
    }

    avctx->coded_frame = &s->picture;
    s->lzw = av_mallocz(ff_lzw_encode_state_size);
    if (!s->lzw)
        return AVERROR(ENOMEM);
    s->buf = av_malloc(avctx->width*avctx->height*2);
    if (!s->buf)
         return AVERROR(ENOMEM);
    return 0;
}

/* better than nothing gif encoder */
static int gif_encode_frame(AVCodecContext *avctx, AVPacket *pkt,
                            const AVFrame *pict, int *got_packet)
{
    GIFContext *s = avctx->priv_data;
    AVFrame *const p = &s->picture;
    uint8_t *outbuf_ptr, *end;
    const uint32_t *palette = NULL;
    int ret;

    if ((ret = ff_alloc_packet2(avctx, pkt, avctx->width*avctx->height*7/5 + FF_MIN_BUFFER_SIZE)) < 0)
        return ret;
    outbuf_ptr = pkt->data;
    end        = pkt->data + pkt->size;

    *p = *pict;
    p->pict_type = AV_PICTURE_TYPE_I;
    p->key_frame = 1;

    if (avctx->pix_fmt == AV_PIX_FMT_PAL8)
        palette = (uint32_t*)p->data[1];

    gif_image_write_image(avctx, &outbuf_ptr, end, palette, pict->data[0], pict->linesize[0]);
    if (!s->last_frame) {
        s->last_frame = av_frame_alloc();
        if (!s->last_frame)
            return AVERROR(ENOMEM);
    }
    av_frame_unref(s->last_frame);
    ret = av_frame_ref(s->last_frame, (AVFrame*)pict);
    if (ret < 0)
        return ret;

    pkt->size   = outbuf_ptr - pkt->data;
    pkt->flags |= AV_PKT_FLAG_KEY;
    *got_packet = 1;

    return 0;
}

static int gif_encode_close(AVCodecContext *avctx)
{
    GIFContext *s = avctx->priv_data;

    av_freep(&s->lzw);
    av_freep(&s->buf);
    av_frame_free(&s->last_frame);
    return 0;
}

AVCodec ff_gif_encoder = {
    .name           = "gif",
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_GIF,
    .priv_data_size = sizeof(GIFContext),
    .init           = gif_encode_init,
    .encode2        = gif_encode_frame,
    .close          = gif_encode_close,
    .pix_fmts       = (const enum AVPixelFormat[]){
        AV_PIX_FMT_RGB8, AV_PIX_FMT_BGR8, AV_PIX_FMT_RGB4_BYTE, AV_PIX_FMT_BGR4_BYTE,
        AV_PIX_FMT_GRAY8, AV_PIX_FMT_PAL8, AV_PIX_FMT_NONE
    },
    .long_name      = NULL_IF_CONFIG_SMALL("GIF (Graphics Interchange Format)"),
};
