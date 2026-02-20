using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServerLib
{
	internal static class TimeUtil
	{
		/// <summary>
		/// 秒をミリ秒に変換する
		/// </summary>
		/// <param name="seconds">秒</param>
		/// <returns>ミリ秒</returns>
		public static int ToMilliSec(float seconds)
		{
			return (int)(seconds * 1000.0f);
		}
	}
}
