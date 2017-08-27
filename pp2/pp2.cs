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

public class Flags
{
    public static ushort OPUS = 0x400;
    public static ushort ZSTD = 0x800;
    public static ushort ALONE = 0x1000;
    public static ushort LINK = 0x8000;
}

public class DirEntry : IReadFile
{
    public string Name { get; set; }
    public string fp;
    public DirEntry(string name, string realpath)
    {
        Name = name;
        fp = realpath;
    }
    public Stream CreateReadStream()
    {
        return new FileInfo(fp).OpenRead();
    }
}


// Describes one file to be packed in a chunk (probably with other files)
// This can be easily interfac-ied, needs to give only .GetData(), .GetMeta() & .naked
public class ChunkFile
{
    const int MAXSIZE = 16 * 1024 * 1024;
    // helper fields
    public bool done, naked; // no zstd, and no chunk grouping
    public IReadFile pp;

    // serialized data in meta()
    public uint chunk_idx;
    public uint chunk_off;
    public uint orig_size;
    public long md5;        // first 64bits only
    public ushort chpos;      // type and flags
    public ushort flags;      // type and flags
    public string name;

    // serializes file metadata for packed file header
    public void meta(Stream outf)
    {
        outf.Write(BitConverter.GetBytes(chunk_idx), 0, 4);
        outf.Write(BitConverter.GetBytes(chunk_off), 0, 4);
        outf.Write(BitConverter.GetBytes(orig_size), 0, 4);
        outf.Write(BitConverter.GetBytes(chpos), 0, 2);
        outf.Write(BitConverter.GetBytes(flags), 0, 2);
    }

    public void metaname(Stream outf)
    {
        outf.Write(BitConverter.GetBytes(md5), 0, 8);
        outf.Write(BitConverter.GetBytes(name.Length * 2), 0, 2);
        outf.Write(Encoding.Unicode.GetBytes(name), 0, name.Length * 2);
    }

    public ChunkFile(IReadFile _pp, string parent, uint size)
    {
        pp = _pp;
        name = (parent + "/" + pp.Name);

        if (name.ToLower().EndsWith(".wav"))
        {
            var tmp = new byte[44];
            pp.CreateReadStream().Read(tmp, 0, 44);
            if (checkWav(tmp) != 0)
            {
                naked = true;
                flags |= Flags.OPUS;
            }
        }
        if (!naked)
        {
            flags |= Flags.ZSTD;
        }
        orig_size = size;
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
        cb.lastf = this;
        ushort chp = cb.chpos;
        chpos = chp;
        chunk_idx = cb.idx;
        chunk_off = (uint)cb.buffer.Length;
        if (chpos == 0)
            Debug.Assert(chunk_off == 0);
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


    int checkWav(byte[] wav)
    {
        int nchan = wav[22];
        int isr = wav[24] | (((int)wav[25]) << 8);
        int bps = wav[28] | (((int)wav[29]) << 8) | (((int)wav[30]) << 16);
        if ((isr == 0) || ((nchan != 1) && (nchan != 2)))
        {
            Console.WriteLine(name + " appears to be corrupted! " + orig_size);
            return 0;
        }
        int width = bps / isr / nchan;
        if (width != 2)
            return 0;
        return isr;
    }
    unsafe void encodeWavToOpus(byte[] wav, Stream os)
    {
        int nchan = wav[22];
        int isr = checkWav(wav);
        int pos = 44;
        int total = wav.Length - pos;
        var packet = new byte[65536];
        IntPtr error;
        //int bstep = 2880;

        int[][] hifi = new int[][]
        {
            //          f  sr  nchan kbps   osr    60/120 step
            new int [] {1, 22050, 1, 40000, 24000},
            new int [] {1, 22050, 2, 64000, 24000},
            new int [] {2, 44100, 1, 48000, 48000},
            new int [] {2, 44100, 2, 64000, 48000}
        };

        int[][] lofi = new int[][]
        {
            //          f  sr  nchan kbps   osr    60/120 step
            new int [] {1, 22050, 1, 32000, 24000},
            new int [] {1, 22050, 2, 40000, 24000},
            new int [] {2, 44100, 1, 40000, 48000},
            new int [] {2, 44100, 2, 48000, 48000},

            new int [] {5, 24000, 1, 32000, 24000},
            new int [] {5, 24000, 2, 40000, 24000},
            new int [] {6, 48000, 1, 48000, 48000},
            new int [] {6, 48000, 2, 64000, 48000},
        };

        var paras = lofi;

        IntPtr ctx = (IntPtr)0;

        int step = 0;
        int framelen = 1920;
        foreach (var para in paras)
        {
            int flg = para[0];
            int sr = para[1];
            int nch = para[2];
            int kbps = para[3];
            int osr = para[4];
            if (sr == isr && nch == nchan)
            {
                if (Path.GetFileName(name).StartsWith("bgm"))
                {
                    kbps = 96 * 1024;
                }
                ctx = OPUS.opus_encoder_create(osr, nchan, (int)Application.Voip, out error);
                if ((long)ctx == 0L)
                    throw new Exception("opus_encoder_create failed " + isr + ", " + nchan + " for "+name);

                flags |= (ushort)flg;
                flags |= (ushort)(nchan << 4);
                int  ret;
                
                ret = OPUS.opus_encoder_ctl(ctx, Ctl.OPUS_SET_COMPLEXITY_REQUEST, (IntPtr)10);
                ret = OPUS.opus_encoder_ctl(ctx, Ctl.OPUS_SET_BITRATE_REQUEST, (IntPtr)kbps);
                //ret = OPUS.opus_encoder_ctl(ctx, Ctl.OPUS_SET_VBR_CONSTRAINT_REQUEST, (IntPtr)0);
                break;
            }
        }
        if ((long)ctx == 0L)
            throw new Exception("Failed to setup for " + isr + ", " + nchan + " for " + name);

        step = framelen * 2 * nchan;

        var wavbuf = new byte[step];

        long packed = 0;
        int rsum, ravg;
        rsum = ravg = 0;
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
            Array.Copy(wav, i + 44, wavbuf, 0, wavlen);

            int ret;
            fixed (byte* ppacket = packet)
            {
                ret = OPUS.opus_encode(ctx, wavbuf, framelen, new IntPtr(ppacket), packet.Length);
            }
            if (ret < 0)
            {
                throw new Exception("Encoding failed - " + ((Errors)ret).ToString() + " for " + name);
            }
            if (ret > 65534)
                throw new Exception("Frame overflow " + ret);
            rsum += ret;
            ravg++;
            packed += ret;
            os.WriteByte((byte)ret);
            os.WriteByte((byte)(ret >> 8));
            os.Write(packet, 0, ret);
            if ((i % (step*32)) == 0)
                Console.Write("w");
        }
       // Console.WriteLine("average packet " + (rsum / ravg));
        Console.Write(packed * 100 / wav.Length);
        OPUS.opus_encoder_destroy(ctx);
    }
}

// This actually builds sequence of chunks as well as its metadata.
public class ChunkBuilder
{
    public MemoryStream buffer; // we build the chunk there on the go
    public uint at; // where the chunk is in the file
    public uint bufulen; // buffer unpacked length
    public uint idx; // index of this chunk
    public ushort chpos;

    long last;
    public ChunkFile lastf, currf;

    long totalu, totalc;
    long tosize;

    List<ChunkFile> files;
    Dictionary<String, uint> links;

    Dictionary<long, uint> hash;
    public MemoryStream table;

    public ChunkBuilder()
    {
        files = new List<ChunkFile>();
        hash = new Dictionary<long, uint>();
        table = new MemoryStream();
        buffer = new MemoryStream();
        links = new Dictionary<string, uint>();
    }

    public void AddChunkFile(ChunkFile chf)
    {
        if (chf.orig_size == 0)
        {
            links[chf.name] = 0xffffffff;
            return;
        }

        uint link;
        // link to earlier hash if it's a dupe
        if (hash.TryGetValue(chf.md5, out link))
        {
            links[chf.name] = link;
            return;
        }
        hash[chf.md5] = (uint)files.Count();
        files.Add(chf);
        tosize += chf.orig_size;
    }

    public void EncodeChunks(Stream os)
    {
        foreach (ChunkFile chf in files)
        {
            AppendFile(os, chf);
        }
        FlushZstd(os);
        table.Write(BitConverter.GetBytes(at), 0, 4);
    }

    public void AppendFile(Stream os, ChunkFile chf)
    {
        //Console.WriteLine("append file " + chf.name + " buffer " + buffer.Length);
        currf = chf;
        if (chf.done)
        {
            Console.Write("!");
        }
        if (chf.naked)
            FlushZstd(os);
        Console.Write(Path.GetExtension(chf.name).ToUpper()[1]);
        chf.GetData(this, os);
        chpos++;
/*        if (buffer.Length == 37207)
        {
            Console.WriteLine("debug");
        }*/
        totalu += chf.orig_size;
        chf.done = true;
        if (chf.naked)
            FlushStore(os);
    }

    public bool FlushOnOverflow(int size, int ms, Stream os)
    {
        if (buffer.Length + size > ms)
        {
            FlushZstd(os);
            return true;
        }
        return false;
    }


    unsafe MemoryStream zstdPack(byte[] input)
    {
        var cbuf = new byte[(int)ZSTD.ZSTD_compressBound((IntPtr)input.Length)];
        int clen;
        fixed (byte* pcbuf = cbuf)
        {
            clen = (int)ZSTD.ZSTD_compress(new IntPtr(pcbuf), (IntPtr)cbuf.Length, input, (IntPtr)input.Length, 3);
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

        res.Write(cbuf, 0, (int)dpos);
        return res;
    }

    public void FlushZstd(Stream os)
    {
        if (buffer.Length == 0)
            return;
        bufulen = (uint)buffer.Length;
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
        //Console.WriteLine("chunk at " + at);
        table.Write(BitConverter.GetBytes(at), 0, 4);
        at += (uint)buffer.Length;
        os.Write(buffer.ToArray(), 0, (int)buffer.Length);
        totalc += buffer.Length;
        buffer = new MemoryStream();
        idx++;
        var now = DateTimeOffset.Now.ToLocalTime().Ticks;
        if (now > last)
        {
            last = now + 10 * 1000000;
            Console.WriteLine("");
            Console.WriteLine(totalu / 1024 / 1024 + "/" + tosize / 1024 / 1024 + "MB, " + totalu * 100 / (tosize+1) +
                "% Done, ratio " + totalc * 100 / (totalu+1) + "%");
        }
        if (chpos == 1)
        {
            //Console.WriteLine("flaggin " + lastf.name + " as alone!, current is " + currf.name + " buffer is " + buffer.Length);
            lastf.flags |= Flags.ALONE;
        }

        // some sanity
        if ((lastf.flags & Flags.OPUS) != 0)
        {
            Debug.Assert((lastf.flags & Flags.ALONE) != 0);
            Debug.Assert(chpos == 1);
        }
        chpos = 0;
        Debug.Assert(buffer.Length == 0);
    }

    public void WriteMetadata(Stream os)
    {
        var pak = new MemoryStream();

        // version
        pak.Write(BitConverter.GetBytes((int)0), 0, 4);
        pak.Write(BitConverter.GetBytes((int)(table.Length/4)), 0, 4);
        pak.Write(BitConverter.GetBytes((int)files.Count()), 0, 4);
        pak.Write(BitConverter.GetBytes((int)(files.Count() + links.Count())), 0, 4);

        // dump chunk table
        pak.Write(table.ToArray(), 0, (int)table.Length);

        // dump file meta
        //int co = 0;
        foreach (ChunkFile chf in files)
        {
/*            Console.WriteLine("writing file no " + co++ +
                ", chunk=" + chf.chunk_idx +
                ", chpos=" + chf.chpos  +
                ", choff=" + chf.chunk_off +
                ", orig=" + chf.orig_size +
                ", name=" + chf.name
                );*/
            chf.meta(pak);
        }

        // dump file names
        foreach (ChunkFile chf in files)
        {
            chf.metaname(pak);
        }

        // dump file name links
        foreach (var link in links)
        {
            pak.Write(BitConverter.GetBytes((long)link.Value), 0, 8);
            pak.Write(BitConverter.GetBytes((ushort)((link.Key.Length * 2)|0x8000)), 0, 2);
            pak.Write(Encoding.Unicode.GetBytes(link.Key), 0, link.Key.Length * 2);
        }

        var zpak = zstdPack(pak.ToArray());
        //zpak = pak;
        os.Write(zpak.ToArray(), 0, (int)zpak.Length);
        os.Write(BitConverter.GetBytes((uint)zpak.Length), 0, 4);
    }
}


class pp2
{
    static void process_pp_files(ChunkBuilder chb, IEnumerable<string> dirs, Regex r)
    {
        foreach (var dd in dirs)
        {
            foreach (string fn in Directory.GetFiles(dd, "*.pp"))
            {
                Console.Write("Opening " + fn);
                if (fn.EndsWith("base.pp"))
                    continue;
                ppParser pp = new ppParser(fn);
                int nmatch = 0;
                foreach (ppSubfile file in pp.Subfiles)
                {
                    //Console.WriteLine(ext);
                    if (r.Match(file.Name).Success)
                    {                       
                        chb.AddChunkFile(new ChunkFile(file, Path.GetFileName(file.ppPath), file.size));
                        nmatch++;
                    }
                }
                Console.WriteLine("..." + nmatch + " files matched");
                //if (nfm-- == 0) break;
            }
        }
    }

    static void process_unpacked_tree(ChunkBuilder chb, IEnumerable<string> dirs, Regex r)
    {
        
        foreach (var dd in dirs)
        {
            Console.WriteLine("Scan " + dd);
            foreach (string dirn in Directory.GetDirectories(dd))
            {
                var pdirn = Path.GetFileName(dirn);
                Console.WriteLine("Opening " + pdirn);
                if (!pdirn.StartsWith("jg2"))
                    continue;
                pdirn += ".pp";

                foreach (var f in Directory.GetFiles(dirn))
                {
                    uint size = (uint)new System.IO.FileInfo(f).Length;
                    var plain = Path.GetFileName(f);
                    if (r.Match(plain).Success)
                    {
                        chb.AddChunkFile(new ChunkFile(new DirEntry(Path.GetFileName(f), f), pdirn, size));
                    }
                }
            }
        }
    }

    static void process_textures(ChunkBuilder chb, String bdir, Regex r)
    {
        foreach (var f in Directory.GetFiles(bdir, "*.*", SearchOption.AllDirectories))
        {
            var fn = f.Substring(bdir.Length + 1).Replace("\\", "/");
            var fi = new System.IO.FileInfo(f);
            if ((fi.Attributes & FileAttributes.Directory) == FileAttributes.Directory)
                continue;
            uint size = (uint)fi.Length;
            if (r.Match(fn).Success)
            {
//                Console.WriteLine();

                chb.AddChunkFile(new ChunkFile(new DirEntry(fn, f), "texture", size));
            }
        }
    }

    static void Main(string[] args)
    {
        if (args.Length < 3)
        {
            Console.WriteLine("usage: pp2 output.pp2 regex path... path...");
            return;
        }
        var chb = new ChunkBuilder();
        Regex r = null;
        r = new Regex(args[1], RegexOptions.IgnoreCase);
        //int nfm = 16;
        var trail = Path.GetFileName(args[2]);
        Console.WriteLine(trail);
        if (trail == "AA2_MAKE" || trail == "AA2_PLAY")
        {
            process_unpacked_tree(chb, args.Skip(2), r);
        }
        else if (trail == "texture")
        {
            process_textures(chb, args[2], r);
        }
        else
        {

            process_pp_files(chb, args.Skip(2), r);
        }

        var outf = new FileStream(args[0], FileMode.Create, FileAccess.Write);
        chb.EncodeChunks(outf);
        chb.WriteMetadata(outf);
        // store length of metadata as last integer, so reader can locate
        // the metadata header
        outf.Close();
    }
}
