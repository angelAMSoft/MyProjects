#include <iostream>
#include <ctime>
#include <Windows.h>
#include "base32.h"
#include "sha1.h"
#include "hmac.h"

#include <string>


time_t lastTime;
std::string lastCode;

typedef unsigned char byte;

void calcKey(std::string & text, time_t tiks)
{
	tiks /= 30;
	int base32len = text.size() - 1;
	int secretLen = (base32len * 5 + 7) / 8;
	uint8_t secret[100];
	if ((secretLen = base32_decode((const uint8_t *)text.c_str(), secret, secretLen))<1) {
		return;
	}
	time_t chlg=tiks;
    uint8_t challenge[8];
    for (int j=7;j>=0;j--) {
        challenge[j]=(byte)((int)chlg&0xff);
        chlg >>= 8;
    }
	uint8_t hash[SHA1_DIGEST_LENGTH];
    hmac_sha1(secret, secretLen, challenge, 8, hash, SHA1_DIGEST_LENGTH);
	int offset = hash[SHA1_DIGEST_LENGTH - 1] & 0xF;

  // Compute the truncated hash in a byte-order independent loop.
	unsigned int truncatedHash = 0;
	for (int i = 0; i < 4; ++i) {
		truncatedHash <<= 8;
		truncatedHash  |= hash[offset + i];
	}

  // Truncate to a smaller number of digits.
  truncatedHash &= 0x7FFFFFFF;
  truncatedHash %= 1000000;
  std::string tmpcode = std::to_string((long long)truncatedHash);
  int len = tmpcode.size();
  if(len < 6){
	 std::string tmp(6 - len, '0');
	 lastCode = tmp;
	 lastCode += tmpcode;
  }
  else
	  lastCode = tmpcode;  
}

int Start(char* secret)
{
	time_t timer;
	time(&timer);
	long remainder = timer % 30;
	remainder = 30 - remainder;
	if (lastTime != timer){
		calcKey(std::string(secret), timer);	
		lastTime = timer;
		std::cout << lastCode << std::endl;
		Sleep(1000);
	}
	return 0;
}


int main(int argc, char** argv)
{

	if(argc < 2){
		printf("Usage: auth <secret>");
		return -1;
	}

	while(true){
		Start(argv[1]);
	}
	return 0;
}

