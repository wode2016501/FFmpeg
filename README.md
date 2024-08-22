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
78 enum {
79     COLOR_FormatYUV420Planar                              = 0x13,
80     COLOR_FormatYUV420SemiPlanar                          = 0x7f420888,  //修改
81     COLOR_FormatSurface                                   = 0x7F000789,
82 };
83
                                                                                                       78 enu
.....
293     ff_AMediaFormat_setInt32(format, "frame-rate", s->fps);
 294     ff_AMediaFormat_setInt32(format, "i-frame-interval", gop);
 295
 296     if(strcmp(codec_mime,"video/avc")==0){
 297     ff_AMediaFormat_setInt32(format,"profile" ,8);
 298     ff_AMediaFormat_setInt32(format,"level", 65536);
 299     }
 300     ret = ff_AMediaCodecProfile_getProfileFromAVCodecContext(avctx);
 301     if (ret > 0) {
 302         av_log(avctx, AV_LOG_DEBUG, "set profile to 0x%x\n", ret);                                                              303         ff_AMediaFormat_setInt32(format, "profile", ret);
 304     }
```
shell 

```
sed 's/enabled jni   /enabled jni   #/g'  configure
./configure --enable-opencl --disable-iconv  --enable-vulkan --disable-bzlib --disable-lzma --disable-libdrm --enable-mediacodec --enable-jni \                                                                                                        --enable-hwaccels \
--enable-mediacodec \                                                                                                               --enable-decoder=h264_mediacodec \
--enable-decoder=hevc_mediacodec \
--enable-decoder=mpeg4_mediacodec  \                                                                                                --enable-decoder=vp8_mediacodec \                                                                                                   --enable-decoder=vp9_mediacodec \
--enable-decoder=av1_mediacodec
```
