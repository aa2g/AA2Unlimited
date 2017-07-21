using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace SB3Utility
{
	public interface IReadFile
	{
		string Name { get; set; }
		Stream CreateReadStream();
	}
}
