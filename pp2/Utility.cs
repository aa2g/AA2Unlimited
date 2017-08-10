using System;
using System.Collections.Generic;
using System.Text;
using System.Globalization;
using System.IO;
using System.Configuration;
using System.Reflection;

namespace SB3Utility
{
	public static partial class Utility
	{
		public static Encoding EncodingShiftJIS => Encoding.GetEncoding("Shift-JIS");
        public static int MaxBufSize => 0x400000; //4096;

        public static int EstBufSize(long filesize)
        {
            return (int)Math.Min(filesize, MaxBufSize);
        }

        public static string GetDestFile(DirectoryInfo dir, string prefix, string ext)
		{
			string dest = dir.FullName + @"\" + prefix;
			int destIdx = 0;
			while (File.Exists(dest + destIdx + ext))
			{
				destIdx++;
			}
			dest += destIdx + ext;
			return dest;
		}

        public static void WriteBytes(this Stream stream, byte[] bytes)
        {
            stream.Write(bytes, 0, bytes.Length);
        }
	}
}
