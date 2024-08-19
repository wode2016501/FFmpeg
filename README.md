FFmpeg README
=============

FFmpeg is a collection of libraries and tools to process multimedia content
such as audio, video, subtitles and related metadata.

## Libraries

* `libavcodec` provides implementation of a wider range of codecs.
* `libavformat` implements streaming protocols, container formats and basic I/O access.
* `libavutil` includes hashers, decompressors and miscellaneous utility functions.
* `libavfilter` provides means to alter decoded audio and video through a directed graph of connected filters.
* `libavdevice` provides an abstraction to access capture and playback devices.
* `libswresample` implements audio mixing and resampling routines.
* `libswscale` implements color conversion and scaling routines.

## Tools

* [ffmpeg](https://ffmpeg.org/ffmpeg.html) is a command line toolbox to
  manipulate, convert and stream multimedia content.
* [ffplay](https://ffmpeg.org/ffplay.html) is a minimalistic multimedia player.
* [ffprobe](https://ffmpeg.org/ffprobe.html) is a simple analysis tool to inspect
  multimedia content.
* Additional small tools such as `aviocat`, `ismindex` and `qt-faststart`.

## Documentation

The offline documentation is available in the **doc/** directory.

The online documentation is available in the main [website](https://ffmpeg.org)
and in the [wiki](https://trac.ffmpeg.org).

### Examples

Coding examples are available in the **doc/examples** directory.

## License

FFmpeg codebase is mainly LGPL-licensed with optional components licensed under
GPL. Please refer to the LICENSE file for detailed information.

## Contributing

Patches should be submitted to the ffmpeg-devel mailing list using
`git format-patch` or `git send-email`. Github pull requests should be
avoided because they are not part of our review process and will be ignored##支持android


##修改添加安卓弃用的代码 android mediacode
c++  libavcodec/mediacodecenc.c
```
251     for (int i = 0; i < FF_ARRAY_ELEMS(color_formats); i++) {
 252         if (avctx->pix_fmt == color_formats[i].pix_fmt) {
 253                 //https://developer.android.com/reference/android/media/MediaCodecInfo.CodecCapabilities
 254 #define COLOR_QCOM_FormatYUV420SemiPlanar 0x7fa30c00
 255 #define COLOR_FormatYUV420Flexible 0x7f420888
 256 #define COLOR_FormatYUV420Planar 0x00000013 //19
 257                 if(color_formats[i].color_format==COLOR_FormatYUV420Planar)
 258                         ff_AMediaFormat_setInt32(format, "color-format",  COLOR_FormatYUV420Flexible);
 259                 else
 260                 ff_AMediaFormat_setInt32(format, "color-format",
 261                                      color_formats[i].color_format);
 262             break;
 263         }
 264     }
 265
.....
298     
 299
 300     ff_AMediaFormat_setInt32(format, "frame-rate", s->fps);
 301     ff_AMediaFormat_setInt32(format, "i-frame-interval", gop);
 302     ff_AMediaFormat_setInt32(format,"profile" ,8);
 303     ff_AMediaFormat_setInt32(format,"level", 65536);
 304     //ff_AMediaFormat_setInt32(format,"max-input-size",900000);
 305     ret = ff_AMediaCodecProfile_getProfileFromAVCodecContext(avctx);
 306     if (ret > 0) {
```
