using System;
using System.Collections.Generic;
using System.IO;
using System.Security.Cryptography;
using System.Text;
using System.Linq;

namespace SB3Utility
{
	public static class ppHeader
	{
        public static byte DecryptHeaderBytes(byte buf)
        {
            return DecryptHeaderBytes(new byte[] { buf })[0];
        }

        public static byte[] DecryptHeaderBytes(byte[] buf)
        {
            byte[] table = new byte[]
            {
                0xFA, 0x49, 0x7B, 0x1C, // var48
				0xF9, 0x4D, 0x83, 0x0A,
                0x3A, 0xE3, 0x87, 0xC2, // var24
				0xBD, 0x1E, 0xA6, 0xFE
            };

            byte var28;
            for (int var4 = 0; var4 < buf.Length; var4++)
            {
                var28 = (byte)(var4 & 0x7);
                table[var28] += table[8 + var28];
                buf[var4] ^= table[var28];
            }

            return buf;
        }

        const byte FirstByte = 0x01;
		const int Version = 0x6C;
		readonly static byte[] ppVersionBytes = Encoding.ASCII.GetBytes("[PPVER]\0");

		public static uint HeaderSize(int numFiles)
		{
			return (uint)((288 * numFiles) + 9 + 12);
		}

		public static List<IWriteFile> ReadHeader(string path)
		{
			List<IWriteFile> subfiles = null;
			using (BinaryReader reader = new BinaryReader(File.OpenRead(path)))
            {
                if (!VerifyHeader(path))
                    throw new InvalidDataException("The supplied file is not a PP archive.");

                byte[] versionHeader = reader.ReadBytes(8);
				int version = BitConverter.ToInt32(DecryptHeaderBytes(reader.ReadBytes(4)), 0);

				DecryptHeaderBytes(reader.ReadBytes(1));  // first byte
				int numFiles = BitConverter.ToInt32(DecryptHeaderBytes(reader.ReadBytes(4)), 0);

				byte[] buf = DecryptHeaderBytes(reader.ReadBytes(numFiles * 288));

				subfiles = new List<IWriteFile>(numFiles);
				for (int i = 0; i < numFiles; i++)
				{
					int offset = i * 288;
					ppSubfile subfile = new ppSubfile(path);
					subfile.Name = Utility.EncodingShiftJIS.GetString(buf, offset, 260).TrimEnd(new char[] { '\0' });
					subfile.size = BitConverter.ToUInt32(buf, offset + 260);
					subfile.offset = BitConverter.ToUInt32(buf, offset + 264);

					Metadata metadata = new Metadata();
					metadata.LastBytes = new byte[20];
					System.Array.Copy(buf, offset + 268, metadata.LastBytes, 0, 20);
					subfile.Metadata = metadata;

					subfiles.Add(subfile);
				}
			}
			return subfiles;
		}

		public static void WriteHeader(Stream stream, List<IWriteFile> files, uint[] sizes, object[] metadata)
		{
			byte[] headerBuf = new byte[HeaderSize(files.Count)];
			BinaryWriter writer = new BinaryWriter(new MemoryStream(headerBuf));

			writer.Write(ppVersionBytes);
			writer.Write(DecryptHeaderBytes(BitConverter.GetBytes(Version)));
			
			writer.Write(DecryptHeaderBytes(new byte[] { FirstByte }));
			writer.Write(DecryptHeaderBytes(BitConverter.GetBytes(files.Count)));

			byte[] fileHeaderBuf = new byte[288 * files.Count];
			uint fileOffset = (uint)headerBuf.Length;
			for (int i = 0; i < files.Count; i++)
			{
				int idx = i * 288;
				Utility.EncodingShiftJIS.GetBytes(files[i].Name).CopyTo(fileHeaderBuf, idx);
				BitConverter.GetBytes(sizes[i]).CopyTo(fileHeaderBuf, idx + 260);
				BitConverter.GetBytes(fileOffset).CopyTo(fileHeaderBuf, idx + 264);

				Metadata wakeariMetadata = (Metadata)metadata[i];
				System.Array.Copy(wakeariMetadata.LastBytes, 0, fileHeaderBuf, idx + 268, 20);
				BitConverter.GetBytes(sizes[i]).CopyTo(fileHeaderBuf, idx + 284);

				fileOffset += sizes[i];
			}

			writer.Write(DecryptHeaderBytes(fileHeaderBuf));
			writer.Write(DecryptHeaderBytes(BitConverter.GetBytes(headerBuf.Length)));
			writer.Flush();
			stream.Write(headerBuf, 0, headerBuf.Length);
		}

        public static void WriteRLEHeader(Stream stream, MetaIWriteList data)
        {
            byte[] headerBuf = new byte[HeaderSize(data.Count)];
            BinaryWriter writer = new BinaryWriter(new MemoryStream(headerBuf));

            writer.Write(ppVersionBytes);
            writer.Write(DecryptHeaderBytes(BitConverter.GetBytes(Version)));

            writer.Write(DecryptHeaderBytes(new byte[] { FirstByte }));
            writer.Write(DecryptHeaderBytes(BitConverter.GetBytes(data.Count)));

            List<uint> offsets = new List<uint>();

            byte[] fileHeaderBuf = new byte[288 * data.Count];
            uint fileOffset = (uint)headerBuf.Length;
            for (int i = 0; i < data.Count; i++)
            {
                IWriteFile subfile = data[i].WriteFile;
                uint hash = data[i].Hash;
                uint size = data[i].Size;
                object metadata = data[i].Metadata;

                bool collision = data.GetRange(0, i)
                                    .Any(x => x.Hash == hash);

                uint currentOffset = fileOffset;

                if (collision)
                {
                    int index = data.IndexOf(data.GetRange(0, i)
                                        .First(x => x.Hash == hash));

                    size = data[index].Size;

                    currentOffset = offsets[index];
                }

                offsets.Add(currentOffset);

                int idx = i * 288;
                Utility.EncodingShiftJIS.GetBytes(subfile.Name).CopyTo(fileHeaderBuf, idx);
                BitConverter.GetBytes(size).CopyTo(fileHeaderBuf, idx + 260);
                BitConverter.GetBytes(currentOffset).CopyTo(fileHeaderBuf, idx + 264);

                Metadata wakeariMetadata = (Metadata)metadata;
                System.Array.Copy(wakeariMetadata.LastBytes, 0, fileHeaderBuf, idx + 268, 20);
                BitConverter.GetBytes(size).CopyTo(fileHeaderBuf, idx + 284);

                if (!collision)
                    fileOffset += size;
            }

            writer.Write(DecryptHeaderBytes(fileHeaderBuf));
            writer.Write(DecryptHeaderBytes(BitConverter.GetBytes(headerBuf.Length)));
            writer.Flush();
            stream.Write(headerBuf, 0, headerBuf.Length);
        }

        private static bool ByteArrayCompare(this byte[] a1, byte[] a2)
        {
            if (a1.Length != a2.Length)
                return false;

            for (int i = 0; i < a1.Length; i++)
                if (a1[i] != a2[i])
                    return false;

            return true;
        }

        public static bool VerifyHeader(string path)
        {
            using (BinaryReader reader = new BinaryReader(File.OpenRead(path)))
            {
                byte[] versionHeader = reader.ReadBytes(8);
                if (!ppVersionBytes.ByteArrayCompare(versionHeader))
                    return false;
                
                int version = BitConverter.ToInt32(DecryptHeaderBytes(reader.ReadBytes(4)), 0);
                /*if (Version != version)
                    return false;*/

                byte first = DecryptHeaderBytes(reader.ReadByte());
                if (FirstByte != first)
                    return false;

                int numFiles = BitConverter.ToInt32(DecryptHeaderBytes(reader.ReadBytes(4)), 0);
                if (numFiles < 0)
                    return false;

                if (numFiles * 288 > reader.BaseStream.Length - reader.BaseStream.Position)
                    return false;

                byte[] buf = DecryptHeaderBytes(reader.ReadBytes(numFiles * 288));

                var subfiles = new List<IWriteFile>(numFiles);
                for (int i = 0; i < numFiles; i++)
                {
                    int offset = i * 288;
                    uint size = BitConverter.ToUInt32(buf, offset + 260);
                    uint poffset = BitConverter.ToUInt32(buf, offset + 264);

                    if (poffset < 0 ||
                        poffset + size > reader.BaseStream.Length)
                        return false;
                }

            }
            return true;
        }

		public struct Metadata
		{
			public byte[] LastBytes { get; set; }
		}
	}
}
