using System;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using SB3Utility;
//using ZstdNet;
using System.Text.RegularExpressions;
using OpusCodec;
using System.Security.Cryptography;
using System.Diagnostics;



// Describes one file to be packed in a chunk (probably with other files)
// This can be easily interfac-ied, needs to give only .GetData(), .GetMeta() & .naked
public class ChunkFile
{
    const int MAXSIZE = 16 * 1024 * 1024;
    // helper fields
    public bool done, naked; // no zstd, and no chunk grouping
    public ppSubfile pp;

    // serialized data in meta()
    public long chunk_idx;
    public long chunk_off;
    public long orig_size;
    public long md5;        // first 64bits only
    public long flags;      // type and flags
    public string name;

    // serializes file metadata for packed file header
    public void meta(Stream outf)
    {
        outf.Write(BitConverter.GetBytes(chunk_idx), 0, 8);
        outf.Write(BitConverter.GetBytes(chunk_off), 0, 8);
        outf.Write(BitConverter.GetBytes(orig_size), 0, 8);
        outf.Write(BitConverter.GetBytes(md5), 0, 8);
        outf.Write(BitConverter.GetBytes(flags), 0, 8);
        outf.Write(BitConverter.GetBytes(name.Length), 0, 2);
        outf.Write(Encoding.Unicode.GetBytes(name), 0, name.Length * 2);
    }

    public ChunkFile(ppSubfile _pp)
    {
        pp = _pp;
        name = (Path.GetFileName(pp.ppPath) + "/" + pp.Name).ToLower();
        if (name.EndsWith(".wav"))
            naked = true;
        orig_size = (int)pp.size;
        var hasher = MD5.Create();

        md5 = BitConverter.ToInt64(hasher.ComputeHash(pp.CreateReadStream()), 0);
    }

    // simply return contents of the file (if applicable applying its own compression)
    // idx and off are values where the file will be stored at - so the callee
    // better write it down to talk about it later via meta()
    //
    // if the file wishes to flush current chunk it is in, it can return true
    public void GetData(ChunkBuilder cb, Stream os)
    {
        cb.FlushOnOverflow((int)orig_size, MAXSIZE, os);
        chunk_idx = cb.idx;
        chunk_off = (long)cb.buffer.Length;
        var src = pp.CreateReadStream();
        if (naked)
        {
            var wav = new byte[orig_size];
            src.Read(wav, 0, (int)orig_size);
            encodeWavToOpus(wav, cb.buffer);
            //encodeWavToFLAC(wav, cb.buffer);
        }
        else
        {
            src.CopyTo(cb.buffer);
        }
    }
/*
    void encodeWavToFLAC(byte[] wav, Stream os)
    {
        int nchan = wav[22];
        var pcm = new long[(wav.Length - 44) / 2];

        for (int i = 0; i < pcm.Length; i++)
        {
            //pcm[i] = (int)(short)((wav[44 + i * 2 + 1] << 8) | (short)(wav[44 + i * 2]));
            pcm[i] = (long)(short)((wav[44 + i * 2 + 1] << 8) | (short)(wav[44 + i * 2]));

           // Console.WriteLine(pcm[i] + " x " + wav[i*2+44] + " y " + wav[i*2+1+44]);
        }

        var res = LibFLAC.encodeBuffer(pcm, nchan);
        os.Write(res, 0, res.Length);
    }
    */


    // TODO: make this less hard coded
    unsafe void encodeWavToOpus(byte[] wav, Stream os)
    {
        const int BASERATE = 64000;
        int nchan = wav[22];
        int pos = 44;
        int total = wav.Length - pos;
        var packet = new byte[65536];
        int step = 1920 * 2 * nchan;
        IntPtr error;
        IntPtr ctx = OPUS.opus_encoder_create(48000, nchan, (int)Application.Audio, out error);
        var wavbuf = new byte[step];
        flags |= (long)nchan;
        OPUS.opus_encoder_ctl(ctx, Ctl.OPUS_SET_BITRATE_REQUEST, (IntPtr)(BASERATE * nchan));
        OPUS.opus_encoder_ctl(ctx, Ctl.OPUS_SET_EXPERT_FRAME_DURATION_REQUEST, (IntPtr)5005);
        for (int i = pos; i < total; i += 2)
        {
            var t = wav[i + 1];
            wav[i] = wav[i + 1];
            wav[i + 1] = t;
        }
        long packed = 0;
        for (int i = 0; i < total; i += step)
        {
            int wavlen = total - i;
            if (wavlen > step)
            {
                wavlen = step;
            }
            else
            {
                Array.Clear(wavbuf, wavlen, step - wavlen);
            }
            Array.Copy(wav, i, wavbuf, 0, wavlen);

            int ret;
            fixed (byte* ppacket = packet)
            {
                ret = OPUS.opus_encode(ctx, wavbuf, 1920, new IntPtr(ppacket), packet.Length);
            }
            if (ret < 0)
            {
                throw new Exception("Encoding failed - " + ((Errors)ret).ToString());
            }
            packed += ret;
            os.WriteByte((byte)ret);
            os.WriteByte((byte)(ret >> 8));
            os.Write(packet, 0, ret);
        }
        Console.Write(packed * 100 / wav.Length);
        OPUS.opus_encoder_destroy(ctx);
    }
}

// This actually builds sequence of chunks as well as its metadata.
public class ChunkBuilder
{
    public MemoryStream buffer; // we build the chunk there on the go
    public long at; // where the chunk is in the file
    public long bufulen; // buffer unpacked length
    public int idx; // index of this chunk


    long totalu, totalc;
    long tosize;

    List<ChunkFile> files;
    Dictionary<long, ChunkFile> hash;
    MemoryStream table;

    public ChunkBuilder()
    {
        files = new List<ChunkFile>();
        hash = new Dictionary<long, ChunkFile>();
        table = new MemoryStream();
        buffer = new MemoryStream();
    }

    public void AddChunkFile(ChunkFile chf)
    {
        ChunkFile chtry;
        // link to earlier hash if it's a dupe
        if (!hash.TryGetValue(chf.md5, out chtry))
        {
            hash[chf.md5] = chf;
            chtry = chf;
            Console.Write(".");
            tosize += chf.orig_size;
        } else Console.Write("!");
        files.Add(chtry);
    }

    public void EncodeChunks(Stream os)
    {
        foreach (ChunkFile chf in files)
        {
            AppendFile(os, chf);
        }
    }

    public void AppendFile(Stream os, ChunkFile chf)
    {
        if (chf.done)
        {
            Console.Write("!");
        }
        if (chf.naked)
            FlushZstd(os);
        Console.Write(Path.GetExtension(chf.name).ToUpper()[1]);
        chf.GetData(this, os);
        totalu += chf.orig_size;
        chf.done = true;
        if (chf.naked)
            FlushStore(os);
    }

    public void FlushOnOverflow(int size, int ms, Stream os)
    {
        if (buffer.Length + size > ms)
            FlushZstd(os);
    }


    unsafe MemoryStream zstdPack(byte[] input)
    {
        var cbuf = new byte[(int)ZSTD.ZSTD_compressBound((IntPtr)input.Length)];
        int clen;
        fixed (byte* pcbuf = cbuf)
        {
            clen = (int)ZSTD.ZSTD_compress(new IntPtr(pcbuf), (IntPtr)cbuf.Length, input, (IntPtr)input.Length, 22);
        }
        var res = new MemoryStream();

        res.Write(cbuf, 0, clen);
        return res;
    }

    unsafe MemoryStream zstdPackFancy(byte[] input)
    {
        var cbuf = new byte[(int)ZSTD.ZSTD_compressBound((IntPtr)input.Length)];
        var ctx = ZSTD.ZSTD_createCCtx();
        var dpos = (IntPtr)0;
        var spos = (IntPtr)0;
        ZSTD.ZSTD_CCtx_setParameter(ctx, (IntPtr)100, 999);

        ZSTD.ZSTD_CCtx_setParameter(ctx, (IntPtr)101, 24); //window
        ZSTD.ZSTD_CCtx_setParameter(ctx, (IntPtr)103, 24); // chainlog
        ZSTD.ZSTD_CCtx_setParameter(ctx, (IntPtr)104, 24); //searchlog
        ZSTD.ZSTD_CCtx_setParameter(ctx, (IntPtr)105, 2); //minma

        ZSTD.ZSTD_CCtx_setParameter(ctx, (IntPtr)106, 9999); //tlen
        ZSTD.ZSTD_CCtx_setParameter(ctx, (IntPtr)107, 9);
        ZSTD.ZSTD_CCtx_setParameter(ctx, (IntPtr)402, 9);
        ZSTD.ZSTD_CCtx_setParameter(ctx, (IntPtr)400, 8);


        fixed (byte* pcbuf = cbuf) {
            ZSTD.ZSTD_compress_generic_simpleArgs(ctx, new IntPtr(pcbuf), (IntPtr)cbuf.Length, out dpos, input, (IntPtr)input.Length, out spos, (IntPtr)2);
        }
        ZSTD.ZSTD_freeCCtx(ctx);
        var res = new MemoryStream();

        res.Write(cbuf, 5, (int)dpos);
        return res;
    }

    public void FlushZstd(Stream os)
    {
        if (buffer.Length == 0)
            return;
        bufulen = buffer.Length;
        buffer = zstdPack(buffer.ToArray());
        FlushStore(os);
    }

    public void FlushStore(Stream os)
    {
        if (buffer.Length == 0)
            return;
        FlushForce(os);
    }

    public void FlushForce(Stream os)
    {
        table.Write(BitConverter.GetBytes(at), 0, 8);
        table.Write(BitConverter.GetBytes(bufulen), 0, 8);
        at += buffer.Length;
        os.Write(buffer.ToArray(), 0, (int)buffer.Length);
        totalc += buffer.Length;
        buffer = new MemoryStream();
        idx++;
        Console.WriteLine("");
        Console.WriteLine(totalu / 1024 / 1024 + "/" + tosize / 1024 / 1024 + "MB, " + totalu * 100 / tosize +
            "% Done, ratio " + totalc * 100 / totalu + "%");
    }

    public void WriteMetadata(Stream os)
    {
        var pak = new MemoryStream();
        pak.Write(BitConverter.GetBytes(table.Length), 0, 4);
        pak.Write(table.ToArray(), 0, (int)table.Length);
        pak.Write(BitConverter.GetBytes((int)files.Count()), 0, 4);
        foreach (ChunkFile chf in files)
        {
            chf.meta(pak);
        }
        var zpak = zstdPack(pak.ToArray());
        os.Write(zpak.ToArray(), 0, (int)zpak.Length);
        //os.Write(pak.ToArray(), 0, (int)pak.Length);
        os.Write(BitConverter.GetBytes((long)pak.Length), 0, 8);
        os.Write(BitConverter.GetBytes((long)zpak.Length+8), 0, 8);
        os.WriteByte(1);
    }
}


class pp2
{
    static void Main(string[] args)
    {
        if (args.Length < 3)
        {
            Console.WriteLine("usage: pp2 \\some\\path\\to\\aa2play\\data output.pp2 [maxchunk]");
        }
        var chb = new ChunkBuilder();
        foreach (string fn in Directory.GetFiles(args[0], "*.pp"))
        {
            Console.Write("Opening " + fn);
            if (fn.EndsWith("base.pp"))
                continue;
            ppParser pp = new ppParser(fn);
            foreach (ppSubfile file in pp.Subfiles)
            {
                if (file.Name.EndsWith(".wav"))
                    continue;
                chb.AddChunkFile(new ChunkFile(file));
                //break;
            }
            Console.WriteLine(pp.Subfiles.Count() + " files");
        }

        var outf = new FileStream(args[1], FileMode.Create, FileAccess.Write);
        chb.EncodeChunks(outf);
        chb.FlushZstd(outf);
        chb.WriteMetadata(outf);
        // store length of metadata as last integer, so reader can locate
        // the metadata header
        outf.Close();
    }
}
