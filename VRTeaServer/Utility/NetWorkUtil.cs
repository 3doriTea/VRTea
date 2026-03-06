using System.Net.Sockets;

namespace VRTeaServer.Utility
{
	internal static class NetWorkUtil
	{
		/// <summary>
		/// 指定したバイト数を読み切るまで待機する
		/// </summary>
		/// <param name="stream">ns</param>
		/// <param name="buffer">バッファ</param>
		/// <param name="cts">キャンセルトークン</param>
		/// <returns>読み取り成功 true / false</returns>
		public static async Task<bool> ReadExactlyAsync(NetworkStream stream, byte[] buffer, CancellationTokenSource cts)
		{
			int totalRead = 0;

			while (totalRead < buffer.Length)
			{
				int read = 0;
				try
				{
					read = await stream.ReadAsync(buffer, totalRead, buffer.Length - totalRead, cts.Token);
				}
				catch (OperationCanceledException)
				{
					return false;  // キャンセル発動で失敗
				}

				if (read <= 0)
				{
					return false;  // 切断判定として失敗
				}

				totalRead += read;
			}
			return true;
		}
	}
}
