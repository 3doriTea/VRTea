using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServer.CLI
{
	/// <summary>
	/// 説明を表示するだけの質問
	/// </summary>
	internal class AskDescription : AskBase
	{
		private string _description;  // 説明内容
		public AskDescription(string description)
		{
			_description = description;
		}
		public override void Ask()
		{
			Console.Write($"{_description} ");
		}
	}
}
