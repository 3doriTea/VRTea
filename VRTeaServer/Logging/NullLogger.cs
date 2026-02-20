using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServer.Logging
{
	/// <summary>
	/// nullオブジェクトのロガー
	/// </summary>
	internal class NullLogger : ILogger
	{
		public void Error(string content)
		{
			throw new NotImplementedException();
		}

		public void Write(string content)
		{
			throw new NotImplementedException();
		}

		public void WriteLine(string content)
		{
			throw new NotImplementedException();
		}
	}
}
