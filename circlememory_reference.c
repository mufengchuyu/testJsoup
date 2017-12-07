// 设置为双倍大小，有利于每一圈跑道的变长
unsigned char buf[BUF_SIZE * 2]; 
unsigned char *thief_ptr;
unsigned char *police_ptr;
int racetrack_len;//路径长度
// 判断警察和小偷是否在同一圈内，界线是起跑线
bool InCatchCycle;
void Init()
{
	// 警察和小偷在同一起跑线上
	thief_ptr = police_ptr = buf;
	// 警察和小偷一开始都在同一圈内
	InCatchCycle = true;
}
bool CheckWriteable(int len)
{
	//如果不在同一圈内，小偷就不能超过警察
	if((!InCatchCycle && (police_ptr - thief_ptr < len)) )
		return false;
	return true;
}
int SendMsg(const unsigned char *msg, const int &len)
{
	if(!CheckWriteable(len) || len > BUF_SIZE)
		return 0;
	// 此处要接收消息，无奈，只能使用memcpy
	memcpy(thief_ptr, msg, len);
	// 警察和小偷不在同一圈内
	if(!InCatchCycle || (BUF_SIZE - (thief_ptr - buf) >= len))
	{
		//小偷跑len步长
		thief_ptr += len;
		return 1;
	}
	// 修正这一圈的长度
	racetrack_len = (thief_ptr - WBuf) + len;
	// 跑完一圈，回到终点
	thief_ptr = buf;
	// 小偷这个时候和警察肯定不在同一圈内
	InCatchCycle = false;
	return 1;
}
int ReadMsg()
{
	int len;
	//如果警察和小偷在同一圈内，则计算警察和小偷之间差多远
	if(InCatchCycle) len = thief_ptr - police_ptr;
	//否则则算警察离修正过的终点还差多远
	else len = racetrack_len - (police_ptr - buf);
	if(len <= 0) return 0;
	int n;
	// 因为很多因素，n大多数情况下不等于len
	// 特别是在网络情况下。
	n = get_msg(police_ptr, len);
	//警察跑n步长
	police_ptr += n;
	if(n == len && !InCatchCycle){
		//如果警察跑完一圈，回到起点
		police_ptr = WBuf;
		//这个时候警察和小偷肯定在同一圈了
		InCatchCycle = true;
	}
	return n;
}
