using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace SB3Utility
{
	public interface IWriteFile
	{
		string Name { get; set; }
		void WriteTo(Stream stream);
	}
}
