using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServer.CLI
{
	/// <summary>
	/// 質問を再生するやつ
	/// </summary>
	internal class AskPlayer
	{
		private List<AskBase> _asks;

		public AskPlayer(List<AskBase> asks)
		{
			_asks = asks;
		}

		/// <summary>
		/// 質問を再生する
		/// </summary>
		public void Play()
		{
			foreach (AskBase ask in _asks)
			{
				ask.Ask();
			}
		}
	}
}
