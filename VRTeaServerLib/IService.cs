using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServerLib
{
	/// <summary>
	/// サーバーサービスに共通するインタフェース
	/// </summary>
	public interface IService
	{
		/// <summary>
		/// 処理を開始する
		/// </summary>
		/// <param name="cts">キャンセルトークンソース</param>
		/// <returns>非同期タスク</returns>
		public Task Start(CancellationTokenSource cts);
	}
}
