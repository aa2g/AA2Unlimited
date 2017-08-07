using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;

namespace SB3Utility
{
    public class SeekableCryptoStream : CryptoStream
    {
        private Stream _stream;
        public SeekableCryptoStream(Stream stream, ICryptoTransform transform, CryptoStreamMode mode) : base(stream, transform, mode)
        {
            _stream = stream;
        }

        public override long Position
        {
            get
            {
                return _stream.Position;
            }

            set
            {
                _stream.Position = value;
            }
        }

        public override long Length => _stream.Length;

        public override bool CanSeek => _stream.CanSeek;
    }
}
