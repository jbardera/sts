program RPC_PHONE_PRG
{
	version RPC_PHONE_VERS
	{
	  string STS_M1(string)=1;
	  string STS_M3(string)=2;
		void SEND(string)=3;
	}=1;
}=0x31000000;

