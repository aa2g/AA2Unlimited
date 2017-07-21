using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.IO.Compression;
using System.Security.Cryptography;

namespace SB3Utility
{
	public class ppFormat
	{
		protected static ICryptoTransform CryptoTransform()
		{
			return new CryptoTransformOneCode(new byte[] {
				0x4D, 0x2D, 0xBF, 0x6A, 0x5B, 0x4A, 0xCE, 0x9D,
				0xF4, 0xA5, 0x16, 0x87, 0x92, 0x9B, 0x13, 0x03,
				0x8F, 0x92, 0x3C, 0xF0, 0x98, 0x81, 0xDB, 0x8E,
				0x5F, 0xB4, 0x1D, 0x2B, 0x90, 0xC9, 0x65, 0x00 });
		}

        public static Stream ReadStream(Stream stream)
        {
            return new SeekableCryptoStream(stream, CryptoTransform(), CryptoStreamMode.Read);
        }

        public static Stream WriteStream(Stream stream)
        {
            return new WakeariStream(stream, CryptoTransform(), CryptoStreamMode.Write);
        }

        public static ppHeader.Metadata FinishWriteTo(Stream stream)
        {
            ((CryptoStream)stream).FlushFinalBlock();

            ppHeader.Metadata metadata = new ppHeader.Metadata();
            metadata.LastBytes = ((WakeariStream)stream).LastBytes;
            return metadata;
        }
    }

	#region CryptoTransform

	public class CryptoTransformOneCode : ICryptoTransform
	{
		#region ICryptoTransform Members
		public bool CanReuseTransform
		{
			get { return true; }
		}

		public bool CanTransformMultipleBlocks
		{
			get { return true; }
		}

		public int InputBlockSize
		{
			get { return code.Length; }
		}

		public int OutputBlockSize
		{
			get { return code.Length; }
		}

		public unsafe int TransformBlock(byte[] inputBuffer, int inputOffset, int inputCount, byte[] outputBuffer, int outputOffset)
		{
            /*
            v1
            int transformCount = 0;
			while (transformCount < inputCount)
			{
				for (int i = 0; i < code.Length; i++, transformCount++)
				{
					outputBuffer[outputOffset + transformCount] = (byte)(inputBuffer[inputOffset + transformCount] ^ code[i]);
				}
			}
			return transformCount;
            */

            /*
            //v2
            int blocksize = code.Length;
            int transformCount = (int)Math.Ceiling((double)inputCount / blocksize) * blocksize;
            for (int i = 0; i < transformCount; i++)
            {
                outputBuffer[outputOffset + i] = (byte)(inputBuffer[inputOffset + i] ^ code[i% blocksize]);
            }
            return transformCount;
            */


            //v3
            int count = inputCount / 32;
            fixed (byte* pInput = inputBuffer, pOutput = outputBuffer, pCode = code)
            {
                byte* pi = pInput + inputOffset;
                byte* po = pOutput + outputOffset;

                for (int c = 0; c < count; c++)
                    for (int i = 0; i < 32; i++)
                        *(po++) = (byte)(*(pi++) ^ *(pCode + i));
            }
            return inputCount;

        }

		public byte[] TransformFinalBlock(byte[] inputBuffer, int inputOffset, int inputCount)
		{
			byte[] outputBuffer = new byte[inputCount];
			int remainder = inputCount % 4;
			int transformLength = inputCount - remainder;
			for (int i = 0; i < transformLength; i++)
			{
				outputBuffer[i] = (byte)(inputBuffer[inputOffset + i] ^ code[i]);
			}
			Array.Copy(inputBuffer, inputOffset + transformLength, outputBuffer, transformLength, remainder);
			return outputBuffer;
		}
		#endregion

		#region IDisposable Members
		public void Dispose()
		{
			throw new NotImplementedException();
		}
		#endregion

		private byte[] code = null;

		public CryptoTransformOneCode(byte[] code)
		{
			this.code = code;
		}
	}
	#endregion
}
