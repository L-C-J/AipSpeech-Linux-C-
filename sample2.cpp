#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "curl/include/curl/curl.h"
#include "curl/include/curl/easy.h"
#include "json-cpp/include/json.h"
#include "base64.h"

#define MAX_BUFFER_SIZE 512
#define MAX_BODY_SIZE 1000000
char* FileName;
unsigned char ToHex(unsigned char x) 
{ 
    return  x > 9 ? x + 55 : x + 48; 
}

unsigned char FromHex(unsigned char x) 
{ 
    unsigned char y;
    if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
    else if (x >= '0' && x <= '9') y = x - '0';
    else assert(0);
    return y;
}

std::string UrlEncode(const std::string& str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        if (isalnum((unsigned char)str[i]) || 
            (str[i] == '-') ||
            (str[i] == '_') || 
            (str[i] == '.') || 
            (str[i] == '~'))
            strTemp += str[i];
        else if (str[i] == ' ')
            strTemp += "+";
        else
        {
            strTemp += '%';
            strTemp += ToHex((unsigned char)str[i] >> 4);
            strTemp += ToHex((unsigned char)str[i] % 16);
        }
    }
    return strTemp;
}

std::string UrlDecode(const std::string& str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        if (str[i] == '+') strTemp += ' ';
        else if (str[i] == '%')
        {
            assert(i + 2 < length);
            unsigned char high = FromHex((unsigned char)str[++i]);
            unsigned char low = FromHex((unsigned char)str[++i]);
            strTemp += high*16 + low;
        }
        else strTemp += str[i];
    }
    return strTemp;
}
// static size_t writefunc(void *ptr, size_t size, size_t nmemb, char **result)
// {
    // size_t result_len = size * nmemb;
    // *result = (char *)realloc(*result, result_len + 1);
    // if (*result == NULL)
    // {
        // printf("realloc failure!\n");
        // return 1;
    // }
    // memcpy(*result, ptr, result_len);
    // (*result)[result_len] = '\0';
    // printf("result:%s\n", *result);
	// printf("result_len:%d\n",result_len);
    // return result_len;
// }
static size_t writefunc(void *ptr, size_t size, size_t nmemb, void *result)
{
    FILE *fp;
    size_t result_len = size * nmemb;

    fp = fopen(FileName, "ab");
    if(fp==NULL)
    {
		printf("open file failed\n");
    } 
    if((fwrite(ptr,result_len,1,fp)<0))
		printf("the recieve is empty\n");

    fclose(fp);

    std::cout<<"call back is recieve"<<std::endl;

    return result_len;
}

int main (int argc,char* argv[])
{
    Json::Reader reader;
    Json::Value root;	
    if (argc != 3)//2
    {
        printf("Usage: %s \"Text\" \"filename.mp3\"\n", argv[0]);
        return -1;
    }
	FileName = argv[2];
	
    //put your own params here
    char *cuid = "Your own AppID";
    char *apiKey = "Your own API Key";
    char *secretKey = "Your own Secret Key";

    std::string token;
    char host[MAX_BUFFER_SIZE];
    snprintf(host, sizeof(host), 
            "https://openapi.baidu.com/oauth/2.0/token?grant_type=client_credentials&client_id=%s&client_secret=%s", 
            apiKey, secretKey);
    FILE* fpp = NULL;
    char cmd[MAX_BUFFER_SIZE];
    char* result = (char*)malloc(MAX_BUFFER_SIZE);
    char* curl_cmd = "curl -s ";
    char* yinhao = "\"";
    strcpy(cmd, curl_cmd);
    strcat(cmd, yinhao);
    strcat(cmd, host);
    strcat(cmd, yinhao);
    fpp = popen(cmd, "r");
    fgets(result, MAX_BUFFER_SIZE, fpp);
    pclose(fpp);
	printf("result:%s\n",result);
    if (result != NULL) 
    {

        if (reader.parse(result, root, false)) 
        {
            //token = root.get("access_token","").asString();
			//token = root["access_token"].asString();
        }
		printf("token:");	
		std::cout << root["access_token"].asString();
		printf("\n");
    }

// tex	必填	合成的文本，使用UTF-8编码。小于512个中文字或者英文数字。（文本在百度服务器内转换为GBK后，长度必须小于1024字节）
// tok	必填	开放平台获取到的开发者access_token（见上面的“鉴权认证机制”段落）
// cuid	必填	用户唯一标识，用来区分用户，计算UV值。建议填写能区分用户的机器 MAC 地址或 IMEI 码，长度为60字符以内
// ctp	必填	客户端类型选择，web端填写固定值1
// lan	必填	固定值zh。语言选择,目前只有中英文混合模式，填写固定值zh
// spd	选填	语速，取值0-9，默认为5中语速
// pit	选填	音调，取值0-9，默认为5中语调
// vol	选填	音量，取值0-15，默认为5中音量
// per	选填	发音人选择, 0为普通女声，1为普通男生，3为情感合成-度逍遥，4为情感合成-度丫丫，默认为普通女声
 
	CURL *curl;
    CURLcode res;
    //char *resultBuf = NULL;
	
	CURLcode return_code;
    return_code = curl_global_init(CURL_GLOBAL_ALL);
    if (CURLE_OK != return_code)
    {
        std::cerr << "init libcurl failed." << std::endl;
        return -1;
    }
	
	std::string strurl("http://tsn.baidu.com//text2audio?tex=");
	std::string strtext = UrlEncode(argv[1]);
	strurl += strtext;
	strurl += "&lan=zh&cuid=";
	strurl += cuid;
	strurl += "&ctp=1";
	strurl += "&tok=";
	strurl += root["access_token"].asString();


	printf("strurl:");	
	std::cout << strurl;
	printf("\n");

    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, strurl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    //curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resultBuf);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);
    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        printf("perform curl error:%d.\n", res);
        return 1;
    }

    curl_easy_cleanup(curl);
	curl_global_cleanup();

	
    return 0;
}