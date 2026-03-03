using VRTeaServer.CLI;
using VRTeaServer.Service;

namespace VRTeaServer
{
	internal class Program
	{
		const string LogDirectory = "./";
		static void Main(string[] args)
		{
			//var input = Console.ReadLine();
			//Console.WriteLine(input);
			//File.WriteAllText("./LOOOOOG.txt", input);
			//return;

			var askIPAddress = new AskIPAddress();
			var askGamePortNumber = new AskPortNumber();
			var askWebPortNumber = new AskPortNumber();
			var askAPIKey = new AskAPIKey();
			var askReady = new AskReady();

			var askPlayer = new AskPlayer(
			[
				new AskLine(),
				askIPAddress,
				new AskDescription("Game"),
				askGamePortNumber,
				new AskDescription("Web"),
				askWebPortNumber,
				askAPIKey,
				askReady,
			]);

			while (!askReady.Ready)
			{
				askPlayer.Play();
			}

			Console.Write($"Booting...");
			Console.Write($"\"{askIPAddress.IPAddress}:{askGamePortNumber.PortNumber}\"...");
			Console.WriteLine($"Ok!");

			var sessionManager = new SessionManager();

			var cts = new CancellationTokenSource();
			var servicePlayer = new ServicePlayer(
			[
				new LoggerService(new DirectoryInfo(LogDirectory)),
				new TerminateService(cts),
				new BotService(
					askIPAddress.IPAddress,
					askGamePortNumber.PortNumber,
					askAPIKey.Key),
				new GameTcpService(
					sessionManager,
					askIPAddress.IPAddress,
					askGamePortNumber.PortNumber),
				new GameUdpService(
					sessionManager,
					askIPAddress.IPAddress,
					askGamePortNumber.PortNumber),
				new WebTcpService(
					sessionManager,
					askIPAddress.IPAddress,
					askWebPortNumber.PortNumber),
				//new Reaper(sessionManager),
				new GameLogicService(sessionManager),
			]);

			servicePlayer.Play(cts);

			Console.WriteLine("---Closing server...Ok!---");
		}
	}
}
