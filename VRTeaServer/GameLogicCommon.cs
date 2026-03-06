namespace VRTeaServer
{
	/// <summary>
	/// しょっちゅう変わるプレイヤー情報
	/// </summary>
	internal class PlayerStatus
	{
		public float PositionX { get; set; } = 0.0f;
		public float PositionY { get; set; } = 0.0f;
		public float PositionZ { get; set; } = 0.0f;
	}

	/// <summary>
	/// あまり変わらないプレイヤー情報
	/// </summary>
	internal class PlayerData
	{
		public uint Color { get; set; } = 0xffffffff;
		public string Name { get; set; } = string.Empty;
	}
}
