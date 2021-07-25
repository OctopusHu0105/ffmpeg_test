#include <stdio.h>
// #include <libavutil/log.h>
#include <libavformat/avio.h>
#include <libavformat/avformat.h>

int main(int argc, char *argv[])
{
    int err_code;
    char errors[1024];

    char *src_filename = NULL;
    char *dst_filename = NULL;

    FILE *dst_fd = NULL;

    int audio_stream_index = -1;
    int len;

    AVFormatContext *ofmt_ctx = NULL;
    AVOutputFormat *output_fmt = NULL;

    AVStream *out_stream = NULL;

    AVFormatContext *fmt_ctx = NULL;
    AVFrame *frame = NULL;
    AVPacket pkt;

    av_log_set_level(AV_LOG_DEBUG);

    if (argc < 3)
    {
        av_log(NULL, AV_LOG_DEBUG, "the count of parameters should be more than three!\n");
        return -1;
    }

    src_filename = argv[1];
    dst_filename = argv[2];

    if (src_filename == NULL || dst_filename == NULL)
    {
        av_log(NULL, AV_LOG_DEBUG, "src or dts file is null, plz check them!\n");
        return -1;
    }

    av_register_all();

    dst_fd = fopen(dst_filename, "wb");
    if (!dst_fd)
    {
        av_log(NULL, AV_LOG_DEBUG, "Could not open destination file %s\n", dst_filename);
        return -1;
    }

    /*open input media file, and allocate format context*/
    if ((err_code = avformat_open_input(&fmt_ctx, src_filename, NULL, NULL)) < 0)
    {clean.sh
        av_strerror(err_code, errors, 1024);
        av_log(NULL, AV_LOG_DEBUG, "Could not open source file: %s, %d(%s)\n",
               src_filename,
               err_code,
               errors);
        return -1;
    }

    /*retrieve audio stream*/
    if ((err_code = avformat_find_stream_info(fmt_ctx, NULL)) < 0)
    {
        av_strerror(err_code, errors, 1024);
        av_log(NULL, AV_LOG_DEBUG, "failed to find stream information: %s, %d(%s)\n",
               src_filename,
               err_code,
               errors);
        return -1;
    }

    /*dump input information*/
    av_dump_format(fmt_ctx, 0, src_filename, 0); // 打印媒体信息

    /*dump output information*/
    //av_dump_format(ofmt_ctx, 0, dst_filename, 1);

    frame = av_frame_alloc();
    if (!frame)
    {
        av_log(NULL, AV_LOG_DEBUG, "Could not allocate frame\n");
        return AVERROR(ENOMEM);
    }

    /*initialize packet*/
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    /*find best audio stream*/                                                              //        索引好i  对应的编解码qi
    audio_stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0); //
    if (audio_stream_index < 0)
    {
        av_log(NULL, AV_LOG_DEBUG, "Could not find %s stream in input file %s\n",
               av_get_media_type_string(AVMEDIA_TYPE_AUDIO),
               src_filename);
        return AVERROR(EINVAL);
    }

    /*read frames from media file*/
    while (av_read_frame(fmt_ctx, &pkt) >= 0)
    {
        if (pkt.stream_index == audio_stream_index)
        {

            len = fwrite(pkt.data, 1, pkt.size, dst_fd);
            if (len != pkt.size)
            {
                av_log(NULL, AV_LOG_DEBUG, "warning, length of writed data isn't equal pkt.size(%d, %d)\n",
                       len,
                       pkt.size);
            }
        }
    }

    /*close input media file*/
    avformat_close_input(&fmt_ctx);
    if (dst_fd)
    {
        fclose(dst_fd);
    }

    return 0;
}
