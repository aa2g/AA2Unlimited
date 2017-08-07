using System;
using System.Runtime.InteropServices;

internal class ZSTD
{
    [DllImport("libzstd.dll", CallingConvention = CallingConvention.Cdecl)]
    internal static extern IntPtr ZSTD_createCCtx();

    [DllImport("libzstd.dll", CallingConvention = CallingConvention.Cdecl)]
    internal static extern IntPtr ZSTD_freeCCtx(IntPtr ctx);


    [DllImport("libzstd.dll", CallingConvention = CallingConvention.Cdecl)]
    internal static extern IntPtr ZSTD_CCtx_setParameter(IntPtr ctx, IntPtr param, uint value);

    [DllImport("libzstd.dll", CallingConvention = CallingConvention.Cdecl)]
    internal static extern IntPtr ZSTD_compressBound(IntPtr srcSize);

    [DllImport("libzstd.dll", CallingConvention = CallingConvention.Cdecl)]
    internal static extern IntPtr ZSTD_compress(IntPtr dst, IntPtr dcap, byte[] src, IntPtr scap, int level);

    [DllImport("libzstd.dll", CallingConvention = CallingConvention.Cdecl)]
    internal static extern IntPtr ZSTD_compress_generic_simpleArgs(
        IntPtr ctx,
        IntPtr dst,
        IntPtr dcap,
        out IntPtr dpos,
        byte[] src,
        IntPtr ssize,
        out IntPtr spos,
        IntPtr endop);
    //, IntPtr dcap, byte[] src, IntPtr scap, int level);

}
