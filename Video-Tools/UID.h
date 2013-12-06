#ifndef I_UID
#define I_UID

/* An UID is a unique identifier that can be an unsigned integer
   or a byte array. Two integer UIDs are identical when their value
   is identical, two byte array UIDs are identical when each byte
   is identical and both arrays have the same length.

   => Integers   : 0x00 0x17 0x23 0x37 == 0x17 0x23 0x37
   => Byte Arrays: 0x00 0x17 0x23 0x37 != 0x17 0x23 0x37
*/

class CUID
{
private:
	int uidType;
	unsigned __int64 uidInt;
	int uidByteArraySize;
	unsigned char* uidByteArray;
	unsigned __int32 uidFlags;
public:
	virtual ~CUID();
	CUID();
	CUID(__int64 uid);
	CUID(unsigned __int64 uid);
	CUID(char* uid, size_t size);
	CUID(unsigned char* uid, size_t size);
	virtual void ReInit();
	unsigned __int32 GetUIDFlags();
	void SetUIDFlags(unsigned __int32 uidFlags);
	bool IsUIDIdentical(CUID* uid2);
	operator unsigned __int64();
	operator unsigned char*();
	operator char*();
};

class CHasUID
{
private:
	CUID* uid;

public:
	CHasUID();
	virtual ~CHasUID();

	virtual void SetUID(CUID* uid);
	virtual CUID* GetUID();
};

void generate_uid(char* pDest, int len);

#endif