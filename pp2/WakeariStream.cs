using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Security.Cryptography;

namespace SB3Utility
{
	public class WakeariStream : CryptoStream
	{
		private int lastOffset = 0;

		public byte[] LastBytes { get; protected set; }

		public WakeariStream(Stream stream, ICryptoTransform transform, CryptoStreamMode mode)
			: base(stream, transform, mode)
		{
			LastBytes = new byte[20];
		}

		public override void Write(byte[] buffer, int offset, int count)
		{
			if (count >= 20)
			{
				Array.Copy(buffer, offset + count - 20, LastBytes, 0, 20);
				lastOffset = 20;
			}
			else
			{
				int total = lastOffset + count;
				if (total > 20)
				{
					int shift = total - 20;
					int copy = 20 - count;
					for (int i = 0; i < copy; i++)
					{
						LastBytes[i] = LastBytes[shift + i];
					}

					for (int i = count; i < count; i++)
					{
						LastBytes[i + copy] = buffer[offset + i];
					}
					lastOffset = 20;
				}
				else
				{
					for (int i = 0; i < count; i++)
					{
						LastBytes[i + lastOffset] = buffer[offset + i];
					}
					lastOffset += count;
				}
			}

			base.Write(buffer, offset, count);
		}

		public override void Close()
		{
			LastBytes = null;
			base.Close();
		}
	}
}
