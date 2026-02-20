using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VSCServerLib
{
	/// <summary>
	/// 非同期サービスのインタフェース
	/// </summary>
	public interface IService
	{
		/// <summary>
		/// 非同期タスクを開始する
		/// </summary>
		/// <param name="cts">キャンセルトークンソース</param>
		/// <returns>非同期タスク</returns>
		public Task Start(CancellationTokenSource cts);
	}
}
