#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

const int RCLK = 12;//锁存端口 RCLK ==>D6
const int SCLK = 13;//时钟端口 SCLK ==>D7
const int DIO = 15;//数据端口 DIO ==>D8
 
unsigned char Num[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};
//0,1,2,3,4,5,6,7,8,9, L, O, A, D,-
//0,1,2,3,4,5,6,7,8,9,10,11,12,13,14
unsigned char loadcode[]={0xc7,0xa3,0x88,0xa1,0xbf};
//L O A D -
//0 1 2 3 4
unsigned char errorcode[]={0x86,0xaf};
unsigned char Point[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0xff};
//7,6,5,4,3,2,1,0
//8,8,8,8,8,8,8,8
//8,全亮

//---------------修改此处""内的信息--------------------
const char* ssid     = "wlan_sl";       //WiFi名
const char* password = "5516721849";   //WiFi密码
String biliuid       = "7966";      //bilibili UID
//----------------------------------------------------
DynamicJsonDocument jsonBuffer(200);
WiFiClient client;

void setup()
{
  pinMode(RCLK,OUTPUT);
  pinMode(SCLK,OUTPUT);
  pinMode(DIO,OUTPUT);//三个语句全部设定Arduino管脚为输出
  
  Serial.begin(115200);//串口通信Init，这里主要为了以后开发，顺手习惯加了，本Demo删掉即可
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
 
void SendByte(unsigned char dat)//发送Btye，也是写入的核心部分
{
  uint8_t i;//定义一个循环变量i
  for(i=0;i<8;i++)
  {
    digitalWrite(SCLK,LOW);//首先根据手册，将时钟输出Low，然后才可以输入数据信号
    if(dat & 0x80)//这里是一个LSB
      digitalWrite(DIO,HIGH);
    else 
      digitalWrite(DIO,LOW);
    dat<<=1;
    digitalWrite(SCLK,HIGH);
  }
}
 
void Send2Byte(unsigned char dat1,unsigned char dat2)
{
  SendByte(dat1);
  SendByte(dat2);
}

void Out595()
{
  digitalWrite(RCLK,LOW);
  delay(0);//屏幕刷新时间ms
  digitalWrite(RCLK,HIGH);
}
//以上为8位数码管74HC595驱动
//以下为执行状态
void errorlog()
{
  Send2Byte(errorcode[0],Point[5]);//E
  Out595();
    Send2Byte(errorcode[1],Point[4]);//r
  Out595();
    Send2Byte(errorcode[1],Point[3]);//r
  Out595();
    Send2Byte(loadcode[1],Point[2]);//o
  Out595();
    Send2Byte(errorcode[1],Point[1]);//r
  Out595();
}
void load()
{
  Send2Byte(loadcode[0],Point[5]);//L
  Out595();
  Send2Byte(loadcode[1],Point[4]);//o
  Out595();
  Send2Byte(loadcode[2],Point[3]);//A
  Out595();
  Send2Byte(loadcode[3],Point[2]);//d
  Out595();
}

//以下为数字显示
void displayNumber(int number)//数显驱动
{
  if(number<0||number>99999999)
  {
    errorlog();
   }
   else
   {
    int x=1;
    int tmp = number;
    while(tmp>=10)
    {
      x++;
      tmp=tmp/10;
      }
      for(int i=0;i<x;i++)
      {
        int character = number%10;
        Send2Byte(Num[character],Point[i]);
        Out595();
        number/=10;
      }
   }
}


 
void loop()
{
    if (WiFi.status() == WL_CONNECTED) //此处感谢av30129522视频作者的开源代码
  {
    HTTPClient http;
    http.begin("http://api.bilibili.com/x/relation/stat?vmid=" + biliuid);
    auto httpCode = http.GET();
    Serial.println(httpCode);
    if (httpCode > 0) {
      String resBuff = http.getString();
      DeserializationError error = deserializeJson(jsonBuffer, resBuff);
      if (error) {
        Serial.println("json error");
        while (1);
      }
      JsonObject root = jsonBuffer.as<JsonObject>();
      long code = root["code"];
      Serial.println(code);
      long fans = root["data"]["follower"];
      Serial.println(fans);
      for(int i=60000;i>=0;i--)//应为74HC595的动态扫描，只能写一个函数来循环延迟了，单位ms,1000ms=1s,默认循环为1分钟,也就是1分钟更新一次
      {
        displayNumber(fans);
      }
      delay(0);
    }
  }

//  load();
}
