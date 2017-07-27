// smtp/pop3
#include "mail.h"
//facade of function send()
void Send(SOCKET& s, string data) 
{
    if( send(s, data.c_str(), data.length(), 0) == SOCKET_ERROR ) 
    {
        cerr<<"send data \""<<data<<"\" error"<<endl;
    }
}
//facade of function recv()
void Recv(SOCKET& s, char* buf, int len) 
{
    memset(buf, 0, len);
    if( recv(s, buf, len, 0) == SOCKET_ERROR ) 
    {
        cerr<<"error, while receiving data"<<endl;
    }
}
string Base64Encode(const string& src) 
{
    int i, j, srcLen = src.length();
    string dst(srcLen / 3 * 4 + 4, 0);
    for(i = 0, j= 0; i <=srcLen - 3; i+=3, j+=4) 
    {
        dst[j] = (src[i] & 0xFC) >> 2;
        dst[j+1] = ((src[i] & 0x03) << 4) + ((src[i+1] & 0xF0) >> 4);
        dst[j+2] = ((src[i+1] & 0x0F) << 2) + ((src[i+2] & 0xC0) >> 6);
        dst[j+3] = src[i+2] & 0x3F;
    }
    if( srcLen % 3 == 1 ) 
    {
        dst[j] = (src[i] & 0xFC) >> 2;
        dst[j+1] = ((src[i] & 0x03) << 4);
        dst[j+2] = 64;
        dst[j+3] = 64;
        j += 4;
    }
    else if( srcLen % 3 == 2 ) 
    {
        dst[j] = (src[i] & 0xFC) >> 2;
        dst[j+1] = ((src[i] & 0x03) << 4) + ((src[i+1] & 0xF0) >> 4);
        dst[j+2] = ((src[i+1] & 0x0F) << 2);
        dst[j+3] = 64;
        j+=4;
    }
    string result;
    static unsigned char *base64 = (unsigned char*)("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=");
    for(i = 0; i < j; ++i) 
    { //map 6 bit value to base64 ASCII character
        result += base64[(int)dst[i]];
    }
    return result;
}


bool SendEmail(const string& smtpServer, const string& username, const string& pw, const string& to, const string& data) 
{
    hostent *ph = gethostbyname(smtpServer.c_str());
    if( ph == NULL ) 
    {
        cerr<<"no host: "<<smtpServer<<endl;
        return false;
    }
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(25); //port of SMTP 
    memcpy(&sin.sin_addr.s_addr,ph->h_addr_list[0], ph->h_length);
    //connect to the mail server
    SOCKET s = socket(PF_INET, SOCK_STREAM, 0);
    if( connect(s, (sockaddr*)&sin, sizeof(sin)) ) 
    {
        cerr<<"failed to connect the smtp mail server"<<endl;
        close(s);
        return false;
    }
    
    char recvBuffer[1024];
    Recv(s, recvBuffer, sizeof(recvBuffer)); //wait for greeting message
    //cout<<"recv:"<<recvBuffer<<endl;
    //recv:220 163.com Anti-spam GT for Coremail System (163com[20141201])

    Send(s, "HELO " + smtpServer + "\r\n"); 
    Recv(s, recvBuffer, sizeof(recvBuffer)); //should recv "250 OK"
    //cout<<"recv:"<<recvBuffer<<endl;
    //recv:250 OK

    //start to log in 
    Send(s, (string)"auth login\r\n");
    Recv(s, recvBuffer, sizeof(recvBuffer)); //should recv "334 username:"(This is the decode message)
    //cout<<"recv:"<<recvBuffer<<endl;
    //recv:334 dXNlcm5hbWU6
    string sendstr1 = Base64Encode(username);
    Send(s, sendstr1 + "\r\n");
    Recv(s, recvBuffer, sizeof(recvBuffer));
    //cout<<"recv:"<<recvBuffer<<endl;
    //recv:334 UGFzc3dvcmQ6
    if( string(recvBuffer).substr(0, 3) != "334" ) 
    {
        cout<<"username is error!"<<endl;
        close(s);
        return false;
    }
    string sendstr2 = Base64Encode(pw);
    Send(s, sendstr2 + "\r\n");
    Recv(s, recvBuffer, sizeof(recvBuffer));
    //cout<<"recv:"<<recvBuffer<<endl;
    //recv:235 Authentication successful
    if( string(recvBuffer).substr(0, 3) != "235") 
    {
        cout<<"password error"<<endl;
        close(s);
        return false;
    }

    //Set sender
    Send(s, "mail from:<" + username + ">\r\n");
    Recv(s, recvBuffer, sizeof(recvBuffer)); //should recv "250 Mail OK"
    //cout<<"recv:"<<recvBuffer<<endl;
    //recv:250 Mail OK
    
    //set receiver
    Send(s, "rcpt to:<" + to + ">\r\n");
    Recv(s, recvBuffer, sizeof(recvBuffer)); //should recv "250 Mail OK"
    //cout<<"recv:"<<recvBuffer<<endl;
    //recv:250 Mail OK
    
    //send data
    Send(s, (string)"data\r\n");
    Recv(s, recvBuffer, sizeof(recvBuffer)); //should recv "354 End data with <CR><LF>.<CR><LF>"
    //cout<<"recv:"<<recvBuffer<<endl;
    //recv:354 End data with <CR><LF>.<CR><LF>
    
    Send(s, "to:" + to + "\r\n" + "subject:局域网聊天程序服务器\r\n\r\n" + data + "\r\n.\r\n");
    Recv(s, recvBuffer, sizeof(recvBuffer));
    //cout<<"recv:"<<recvBuffer<<endl;
    //recv:250 Mail OK queued as smtp8,DMCowADHnRZhoQBZOx7HAg--.10352S2 1493213538
    
    Send(s, (string)"quit\r\n");
    Recv(s, recvBuffer, sizeof(recvBuffer));
    //cout<<"recv:"<<recvBuffer<<endl;
    //recv:221 Bye
    
    //closesocket(s);
    close(s);
    return true;
}
//设置邮箱的smtp服务器地址
void CSendEmail::setSmtpServer(string server)
{
    smtpServer = server;
}
//设置邮箱的用户名
void CSendEmail::setAuthName(string name)
{
    username = name;
}
//设置邮箱的密码
void CSendEmail::setAuthPwd(string pwd)
{
    this->pwd = pwd;
}
//设置目的邮件地址
void CSendEmail::setDestEmail(string email)
{
    destEmail = email;
}
//设置发送邮件的文本信息
void CSendEmail::setEmailText(string text)
{
    sendText = text;
}
//发送邮件
bool CSendEmail::sendEmail()
{
    return SendEmail(smtpServer, username, pwd, destEmail, sendText);
}