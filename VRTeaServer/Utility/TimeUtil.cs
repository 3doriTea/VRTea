namespace VRTeaServer.Utility
{
	/// <summary>
	/// 時間操作の便利系詰め合わせ
	/// </summary>
	internal static class TimeUtil
	{
		/// <summary>
		/// 秒数をミリ秒整数に変換
		/// </summary>
		/// <param name="seconds">秒数(小数点数)</param>
		/// <returns>ミリ秒(整数)</returns>
		public static int ToMilliSec(float seconds)
		{
			return (int)(seconds * 1000.0f);
		}
	}
}
