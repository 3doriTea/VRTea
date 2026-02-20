using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServerLib
{
	/// <summary>
	/// ログ書き出すやつのインタフェース
	/// </summary>
	public interface ILogger
	{
		public void WriteLine(string content);
		public void Write(string content);
		public void Error(string content);
	}
}
