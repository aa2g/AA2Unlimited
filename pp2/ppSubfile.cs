using System;
using System.Collections.Generic;
using System.IO;
using System.Security.Cryptography;

namespace SB3Utility
{
	/// <summary>
	/// If removed from a ppParser, CreateReadStream() is no longer guaranteed to work. The .pp file may have changed,
	/// so you have to transfer the ppSubfile's data when removing.
	/// </summary>
	public class ppSubfile : IReadFile, IWriteFile
	{
		public string ppPath;
		public long offset;
		public uint size;

		public object Metadata { get; set; }

		public ppSubfile(string ppPath)
		{
			this.ppPath = ppPath;
		}

		public string Name { get; set; }

		public void WriteTo(Stream stream)
		{
            //CreateReadStream().CopyTo(stream);
			using (BinaryReader reader = new BinaryReader(CreateReadStream()))
			{
				BinaryWriter writer = new BinaryWriter(stream);
				byte[] buf;
                int bufsize = Utility.EstBufSize(reader.BaseStream.Length);
                if (bufsize > 0)
                {
                    while ((buf = reader.ReadBytes(bufsize)).Length == bufsize)
                    {
					    writer.Write(buf);
				    }
				    writer.Write(buf);
                }
                
			}
		}

		public Stream CreateReadStream()
		{
			var fs = File.Open(ppPath, FileMode.Open, FileAccess.Read, FileShare.Read);
            fs.Seek(offset, SeekOrigin.Begin);
            return ppFormat.ReadStream(new PartialStream(fs, size));
		}

		public override string ToString()
		{
			return this.Name;
		}
	}

	// Represents a subsection of the stream. This forces a CryptoStream to use TransformFinalBlock() at the end of the subsection.
	public class PartialStream : Stream
	{
		public override bool CanRead
		{
			get { return stream.CanRead; }
		}

		public override bool CanSeek
		{
			get { return stream.CanSeek; }
		}

		public override bool CanWrite
		{
			get { return stream.CanWrite; }
		}

		public override void Flush()
		{
			stream.Flush();
		}

		public override long Length
		{
			get { return this.length; }
		}

		public override long Position
		{
			get
			{
				return stream.Position - offset;
			}
			set
			{
				stream.Position = value + offset;
			}
		}

		public override int Read(byte[] buffer, int offset, int count)
		{
			if ((stream.Position + count) > end)
			{
				count = (int)(end - stream.Position);
			}

			if (count < 0)
			{
				return 0;
			}
			else
			{
				return stream.Read(buffer, offset, count);
			}
		}

		public override long Seek(long offset, SeekOrigin origin)
		{
			switch (origin)
            {
                case SeekOrigin.Begin:
                    Position = offset;
                    break;
                case SeekOrigin.Current:
                    Position += offset;
                    break;
                case SeekOrigin.End:
                    Position = length + offset;
                    break;
            }

            return Position;
		}

		public override void SetLength(long value)
		{
			throw new NotImplementedException();
		}

		public override void Write(byte[] buffer, int offset, int count)
		{
			throw new NotImplementedException();
		}

		private Stream stream = null;
		private long offset = 0;
		private long length = 0;
		private long end = 0;

		public PartialStream(Stream stream, long length)
		{
			if ((length + stream.Position) > stream.Length)
			{
				throw new ArgumentOutOfRangeException();
			}

			this.stream = stream;
			this.offset = stream.Position;
			this.length = length;
			this.end = this.offset + this.length;
		}

		public override void Close()
		{
			this.Dispose(true);
			GC.SuppressFinalize(this);
		}

		protected override void Dispose(bool disposing)
		{
			try
			{
				if (disposing)
				{
					this.stream.Close();
				}
			}
			finally
			{
				base.Dispose(disposing);
			}
		}
	}
}
