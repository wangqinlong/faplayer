/*****************************************************************************
 * algo_basic.c : Basic algorithms for the VLC deinterlacer
 *****************************************************************************
 * Copyright (C) 2000-2011 the VideoLAN team
 * $Id: 3436356c965e03338605f518a2b0d4852a527923 $
 *
 * Author: Sam Hocevar <sam@zoy.org>
 *         Damien Lucas <nitrox@videolan.org>  (Bob, Blend)
 *         Laurent Aimar <fenrir@videolan.org> (Bob, Blend)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include <stdint.h>

#include <vlc_common.h>
#include <vlc_picture.h>
#include <vlc_filter.h>

#include "merge.h"
#include "deinterlace.h" /* definition of p_sys, needed for Merge() */

#include "algo_basic.h"

/*****************************************************************************
 * RenderDiscard: only keep TOP or BOTTOM field, discard the other.
 *****************************************************************************/

void RenderDiscard( filter_t *p_filter,
                    picture_t *p_outpic, picture_t *p_pic, int i_field )
{
    int i_plane;

    /* Copy image and skip lines */
    for( i_plane = 0 ; i_plane < p_pic->i_planes ; i_plane++ )
    {
        uint8_t *p_in, *p_out_end, *p_out;
        int i_increment;

        p_in = p_pic->p[i_plane].p_pixels
                   + i_field * p_pic->p[i_plane].i_pitch;

        p_out = p_outpic->p[i_plane].p_pixels;
        p_out_end = p_out + p_outpic->p[i_plane].i_pitch
                             * p_outpic->p[i_plane].i_visible_lines;

        switch( p_filter->fmt_in.video.i_chroma )
        {
        case VLC_CODEC_I420:
        case VLC_CODEC_J420:
        case VLC_CODEC_YV12:

            for( ; p_out < p_out_end ; )
            {
                vlc_memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );

                p_out += p_outpic->p[i_plane].i_pitch;
                p_in += 2 * p_pic->p[i_plane].i_pitch;
            }
            break;

        case VLC_CODEC_I422:
        case VLC_CODEC_J422:

            i_increment = 2 * p_pic->p[i_plane].i_pitch;

            if( i_plane == Y_PLANE )
            {
                for( ; p_out < p_out_end ; )
                {
                    vlc_memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );
                    p_out += p_outpic->p[i_plane].i_pitch;
                    vlc_memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );
                    p_out += p_outpic->p[i_plane].i_pitch;
                    p_in += i_increment;
                }
            }
            else
            {
                for( ; p_out < p_out_end ; )
                {
                    vlc_memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );
                    p_out += p_outpic->p[i_plane].i_pitch;
                    p_in += i_increment;
                }
            }
            break;

        default:
            break;
        }
    }
}

/*****************************************************************************
 * RenderBob: renders a BOB picture - simple copy
 *****************************************************************************/

void RenderBob( filter_t *p_filter,
                picture_t *p_outpic, picture_t *p_pic, int i_field )
{
    int i_plane;

    /* Copy image and skip lines */
    for( i_plane = 0 ; i_plane < p_pic->i_planes ; i_plane++ )
    {
        uint8_t *p_in, *p_out_end, *p_out;

        p_in = p_pic->p[i_plane].p_pixels;
        p_out = p_outpic->p[i_plane].p_pixels;
        p_out_end = p_out + p_outpic->p[i_plane].i_pitch
                             * p_outpic->p[i_plane].i_visible_lines;

        switch( p_filter->fmt_in.video.i_chroma )
        {
            case VLC_CODEC_I420:
            case VLC_CODEC_J420:
            case VLC_CODEC_YV12:
                /* For BOTTOM field we need to add the first line */
                if( i_field == 1 )
                {
                    vlc_memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );
                    p_in += p_pic->p[i_plane].i_pitch;
                    p_out += p_outpic->p[i_plane].i_pitch;
                }

                p_out_end -= 2 * p_outpic->p[i_plane].i_pitch;

                for( ; p_out < p_out_end ; )
                {
                    vlc_memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );

                    p_out += p_outpic->p[i_plane].i_pitch;

                    vlc_memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );

                    p_in += 2 * p_pic->p[i_plane].i_pitch;
                    p_out += p_outpic->p[i_plane].i_pitch;
                }

                vlc_memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );

                /* For TOP field we need to add the last line */
                if( i_field == 0 )
                {
                    p_in += p_pic->p[i_plane].i_pitch;
                    p_out += p_outpic->p[i_plane].i_pitch;
                    vlc_memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );
                }
                break;

            case VLC_CODEC_I422:
            case VLC_CODEC_J422:
                /* For BOTTOM field we need to add the first line */
                if( i_field == 1 )
                {
                    vlc_memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );
                    p_in += p_pic->p[i_plane].i_pitch;
                    p_out += p_outpic->p[i_plane].i_pitch;
                }

                p_out_end -= 2 * p_outpic->p[i_plane].i_pitch;

                if( i_plane == Y_PLANE )
                {
                    for( ; p_out < p_out_end ; )
                    {
                        vlc_memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );

                        p_out += p_outpic->p[i_plane].i_pitch;

                        vlc_memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );

                        p_in += 2 * p_pic->p[i_plane].i_pitch;
                        p_out += p_outpic->p[i_plane].i_pitch;
                    }
                }
                else
                {
                    for( ; p_out < p_out_end ; )
                    {
                        vlc_memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );

                        p_out += p_outpic->p[i_plane].i_pitch;
                        p_in += 2 * p_pic->p[i_plane].i_pitch;
                    }
                }

                vlc_memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );

                /* For TOP field we need to add the last line */
                if( i_field == 0 )
                {
                    p_in += p_pic->p[i_plane].i_pitch;
                    p_out += p_outpic->p[i_plane].i_pitch;
                    vlc_memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );
                }
                break;
        }
    }
}

/*****************************************************************************
 * RenderLinear: BOB with linear interpolation
 *****************************************************************************/

void RenderLinear( filter_t *p_filter,
                   picture_t *p_outpic, picture_t *p_pic, int i_field )
{
    int i_plane;

    /* Copy image and skip lines */
    for( i_plane = 0 ; i_plane < p_pic->i_planes ; i_plane++ )
    {
        uint8_t *p_in, *p_out_end, *p_out;

        p_in = p_pic->p[i_plane].p_pixels;
        p_out = p_outpic->p[i_plane].p_pixels;
        p_out_end = p_out + p_outpic->p[i_plane].i_pitch
                             * p_outpic->p[i_plane].i_visible_lines;

        /* For BOTTOM field we need to add the first line */
        if( i_field == 1 )
        {
            vlc_memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );
            p_in += p_pic->p[i_plane].i_pitch;
            p_out += p_outpic->p[i_plane].i_pitch;
        }

        p_out_end -= 2 * p_outpic->p[i_plane].i_pitch;

        for( ; p_out < p_out_end ; )
        {
            vlc_memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );

            p_out += p_outpic->p[i_plane].i_pitch;

            Merge( p_out, p_in, p_in + 2 * p_pic->p[i_plane].i_pitch,
                   p_pic->p[i_plane].i_pitch );

            p_in += 2 * p_pic->p[i_plane].i_pitch;
            p_out += p_outpic->p[i_plane].i_pitch;
        }

        vlc_memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );

        /* For TOP field we need to add the last line */
        if( i_field == 0 )
        {
            p_in += p_pic->p[i_plane].i_pitch;
            p_out += p_outpic->p[i_plane].i_pitch;
            vlc_memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );
        }
    }
    EndMerge();
}

/*****************************************************************************
 * RenderMean: Half-resolution blender
 *****************************************************************************/

void RenderMean( filter_t *p_filter,
                 picture_t *p_outpic, picture_t *p_pic )
{
    int i_plane;

    /* Copy image and skip lines */
    for( i_plane = 0 ; i_plane < p_pic->i_planes ; i_plane++ )
    {
        uint8_t *p_in, *p_out_end, *p_out;

        p_in = p_pic->p[i_plane].p_pixels;

        p_out = p_outpic->p[i_plane].p_pixels;
        p_out_end = p_out + p_outpic->p[i_plane].i_pitch
                             * p_outpic->p[i_plane].i_visible_lines;

        /* All lines: mean value */
        for( ; p_out < p_out_end ; )
        {
            Merge( p_out, p_in, p_in + p_pic->p[i_plane].i_pitch,
                   p_pic->p[i_plane].i_pitch );

            p_out += p_outpic->p[i_plane].i_pitch;
            p_in += 2 * p_pic->p[i_plane].i_pitch;
        }
    }
    EndMerge();
}

/*****************************************************************************
 * RenderBlend: Full-resolution blender
 *****************************************************************************/

void RenderBlend( filter_t *p_filter,
                  picture_t *p_outpic, picture_t *p_pic )
{
    int i_plane;

    /* Copy image and skip lines */
    for( i_plane = 0 ; i_plane < p_pic->i_planes ; i_plane++ )
    {
        uint8_t *p_in, *p_out_end, *p_out;

        p_in = p_pic->p[i_plane].p_pixels;

        p_out = p_outpic->p[i_plane].p_pixels;
        p_out_end = p_out + p_outpic->p[i_plane].i_pitch
                             * p_outpic->p[i_plane].i_visible_lines;

        switch( p_filter->fmt_in.video.i_chroma )
        {
            case VLC_CODEC_I420:
            case VLC_CODEC_J420:
            case VLC_CODEC_YV12:
                /* First line: simple copy */
                vlc_memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );
                p_out += p_outpic->p[i_plane].i_pitch;

                /* Remaining lines: mean value */
                for( ; p_out < p_out_end ; )
                {
                    Merge( p_out, p_in, p_in + p_pic->p[i_plane].i_pitch,
                           p_pic->p[i_plane].i_pitch );

                    p_out += p_outpic->p[i_plane].i_pitch;
                    p_in  += p_pic->p[i_plane].i_pitch;
                }
                break;

            case VLC_CODEC_I422:
            case VLC_CODEC_J422:
                /* First line: simple copy */
                vlc_memcpy( p_out, p_in, p_pic->p[i_plane].i_pitch );
                p_out += p_outpic->p[i_plane].i_pitch;

                /* Remaining lines: mean value */
                if( i_plane == Y_PLANE )
                {
                    for( ; p_out < p_out_end ; )
                    {
                        Merge( p_out, p_in, p_in + p_pic->p[i_plane].i_pitch,
                               p_pic->p[i_plane].i_pitch );

                        p_out += p_outpic->p[i_plane].i_pitch;
                        p_in  += p_pic->p[i_plane].i_pitch;
                    }
                }
                else
                {
                    for( ; p_out < p_out_end ; )
                    {
                        Merge( p_out, p_in, p_in + p_pic->p[i_plane].i_pitch,
                               p_pic->p[i_plane].i_pitch );

                        p_out += p_outpic->p[i_plane].i_pitch;
                        p_in  += 2*p_pic->p[i_plane].i_pitch;
                    }
                }
                break;
        }
    }
    EndMerge();
}
