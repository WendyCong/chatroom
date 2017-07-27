#ifndef SRC_UTILS_AES_ENCRYPTOR_H
#define SRC_UTILS_AES_ENCRYPTOR_H

#include <string>
using namespace std;

class AES;

class AesEncryptor
{
public:
    AesEncryptor(unsigned char* key);
    ~AesEncryptor(void);

    string EncryptString(string strInfor);
    string DecryptString(string strMessage);

    void EncryptTxtFile(const char* inputFileName, const char* outputFileName);
    void DecryptTxtFile(const char* inputFileName, const char* outputFileName);

private:
    void Byte2Hex(const unsigned char* src, int len, char* dest);
    void Hex2Byte(const char* src, int len, unsigned char* dest);
    int  Char2Int(char c);

private:
    AES* m_pEncryptor;
};

#endif        // SRC_UTILS_AES_ENCRYPTOR_H