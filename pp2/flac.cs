using System.IO;
using System;
using System.Runtime.InteropServices;
public class LibFLAC
{
    public static byte[] encodeBuffer(long [] pcm, int nchan)
    {
        int nsamp = pcm.Length / nchan;
        var temp = Path.GetTempFileName();
        IntPtr encoder = FLAC__stream_encoder_new();
        int status = FLAC__stream_encoder_init_file(encoder, temp, IntPtr.Zero, IntPtr.Zero);
        if (status != 0) throw new Exception("Encoder: INIT FILE failed");
        FLAC__stream_encoder_set_channels(encoder, (uint)nchan);
        FLAC__stream_encoder_set_bits_per_sample(encoder, 16);
        FLAC__stream_encoder_set_sample_rate(encoder, 44100);
        FLAC__stream_encoder_set_compression_level(encoder, 5);
        FLAC__stream_encoder_set_total_samples_estimate(encoder, nsamp);
        FLAC__stream_encoder_set_total_samples_estimate(encoder, nsamp);
        FLAC__stream_encoder_process_interleaved(encoder, pcm, (uint)nsamp);
        FLAC__stream_encoder_finish(encoder);
        FLAC__stream_encoder_delete(encoder);
        var res = File.ReadAllBytes(temp);
        //File.Delete(temp);
        return res;
    }

    [DllImport("libFLAC", CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr FLAC__stream_encoder_new();

    [DllImport("libFLAC", CallingConvention = CallingConvention.Cdecl)]
    public static extern void FLAC__stream_encoder_delete(IntPtr encoder);

    [DllImport("libFLAC", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool FLAC__stream_encoder_set_channels(IntPtr encoder, uint value);

    [DllImport("libFLAC", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool FLAC__stream_encoder_set_bits_per_sample(IntPtr encoder, uint value);

    [DllImport("libFLAC", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool FLAC__stream_encoder_set_sample_rate(IntPtr encoder, uint value);

    [DllImport("libFLAC", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool FLAC__stream_encoder_set_compression_level(IntPtr encoder, uint value);

    [DllImport("libFLAC", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool FLAC__stream_encoder_finish(IntPtr encoder);

    [DllImport("libFLAC", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool FLAC__stream_encoder_set_total_samples_estimate(IntPtr encoder, Int64 val);

    [DllImport("libFLAC", CallingConvention = CallingConvention.Cdecl)]
    public static extern int FLAC__stream_encoder_init_file(IntPtr encoder, string filename, IntPtr progress_callback, IntPtr client_data);

    [DllImport("libFLAC", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool FLAC__stream_encoder_process_interleaved(IntPtr encoder, long[] buffer, uint samples);
}