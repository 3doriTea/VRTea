using GroqApiLibrary;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.Json.Nodes;
using System.Threading.Tasks;

namespace VRTeaServer.AI
{
	internal class AIBrain
	{
		private readonly GroqApiClient _groqApi;
		const string ConstantSetting =
			"自分の色を変更するときはメッセージの途中で##から始まり##で終わるマークの中に16進数カラーコードを入れて感情表現して。例「こんにちは。##33ff33##お元気ですか。」";

		public string SystemRole { get; set; } = string.Join("",
			ConstantSetting,
			"日本語でしゃべる",
			"年齢は38歳",
			"趣味はお手玉",
			"一人称は「俺っち」",
			"語尾は「ピカ」",
			"名前は「足車輪」");
		public string AiModel { get; set; } = "openai/gpt-oss-20b";//"llama3-8b-8192", // または "llama3-70b-8192"

		public AIBrain(string apiKey)
		{
			_groqApi = new(apiKey);
		}

		public async Task<string?> Ask(string content)
		{
			var request = new JsonObject
			{
				["model"] = AiModel,
				["messages"] = new JsonArray
				{
					new JsonObject
					{
						["role"] = "system",
						["content"] = SystemRole,
					},
					new JsonObject
					{
						["role"] = "user",
						["content"] = $"{content}",
					}
				},
				["temperature"] = 0.5,
				["max_tokens"] = 1024
			};

			try
			{
				var result = await _groqApi.CreateChatCompletionAsync(request);
				var responseText = result?["choices"]?[0]?["message"]?["content"]?.ToString();

				return responseText!;
			}
			catch (Exception ex)
			{
				Console.WriteLine($"Error: {ex}");
				return null;
			}
		}
	}
}
