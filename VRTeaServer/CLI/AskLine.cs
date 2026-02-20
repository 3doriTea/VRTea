using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServer.CLI
{
	/// <summary>
	/// 線を引くだけ
	/// </summary>
	internal class AskLine : AskBase
	{
		const int LineCount = 30;
		public override void Ask()
		{
			Console.WriteLine(new string('-', LineCount));
		}
	}
}
