using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServer.CLI
{
	internal class AskReady : AskBase
	{
		public bool Ready = false;
		public override void Ask()
		{
			Ready = AskYesNo($"Ready? (Y/n)");
		}
	}
}
