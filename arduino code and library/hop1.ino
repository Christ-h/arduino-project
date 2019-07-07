#include <stdlib.h>
#include <SoftwareSerial.h>
#include <dht.h>
#include <Wire.h> //IIC库
#include <math.h>
dht DHT;
int BH1750address = 0x23;//芯片地址为16位23
byte buff[2];

const int pin = 4;  // 将把 DHT11的data pin连到arduino
char target1[] = "OK";
char target2[] = ">";
char target3[] = "Error";
#define SSID "ssid"      //wifi名
#define PASS "password" //wifi密码
#define IP "184.106.153.149" // 连接thingspeak.com服务器
String GET = "GET /update?key=W5Y89S5XZES4R5RU"; //输入前面记下的API
SoftwareSerial monitor(10, 11); // 定义D10 D11为软串口RX, TX

//初始化-----------------------------------------
void setup()
{
  
  Wire.begin();
  monitor.begin(115200);
  Serial.begin(115200);
  sendDebug("AT");        //指令测试
  //delay(5000);

  if (Serial.find(target1))  //接收指令正常则返回OK
  {
    monitor.println("RECEIVED: OKReceive");
    connectWiFi();
  }
  else
  {
    monitor.println("RECEIVED: ErrorReceive");
  }
}

//主循环-----------------------------------------
void loop()
{
  DHT.read11(pin);  // 读取 DHT11 传感器
  //Serial.print(DHT.humidity,1);    //串口打印湿度
  //Serial.print(",\t");
  //Serial.println(DHT.temperature,1);   //打印温度
  float tempH = DHT.humidity;
  float tempT = DHT.temperature;
  char buffer[10];
  String temph = dtostrf(tempH, 4, 1, buffer);
  String tempt = dtostrf(tempT, 4, 1, buffer);

  int i;
  String lux;
  uint16_t val=0;
  BH1750_Init(BH1750address);
  delay(1000);
  if(2==BH1750_Read(BH1750address))
  {
   val=((buff[0]<<8)|buff[1])/1.2;
   lux=val;
  }
  
  if (Serial.available() > 0)
  {
    delay(500);
    monitor.println("GOOD");
  }
  else
  {
    delay(500);
    monitor.println("LOOP_ERROR");
  }
  updateTemp(temph, tempt, lux);
  delay(60000);
}

//更新温湿度、光照度--------------------------------------
void updateTemp(String temph, String tempt, String lux)
{
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += IP;
  cmd += "\",80";
  sendDebug(cmd);                         //发送指令，连接服务器
  delay(2000);
  if (Serial.find(target3))
  {
    monitor.print("RECEIVED: ErrorTCP");
    return;
  }

  cmd = GET + "&field1=" + tempt + "&field2=" + temph + "&field3=" + lux +"\r\n";       //发送温湿度和光照度
  Serial.print("AT+CIPSEND=");
  Serial.println(cmd.length());
  //delay(100000);
  if (Serial.find(target2))
  {
    monitor.print(">");
    monitor.print(cmd);
    Serial.println(cmd);
  }
  else
  {
    sendDebug("AT+CIPCLOSE");
  }
  if (Serial.find(target1))
  {
    monitor.println("RECEIVED: OKSEND");
  }
  else
  {
    monitor.println("RECEIVED: ErrorSEND");
  }
}
//调试WIFi-------------------------------------------
void sendDebug(String cmd)
{
  monitor.print("SEND: ");
  monitor.println(cmd);
  Serial.println(cmd);
  if (Serial.available() > 0)
  {
    delay(5000);
  }
}
//连接WiFi----------------------------------------------
boolean connectWiFi()
{
  Serial.println("AT+CIPMUX=0");
  Serial.println("AT+CWMODE=1");
  delay(2000);
  String cmd = "AT+CWJAP=\"";
  cmd += SSID;
  cmd += "\",\"";
  cmd += PASS;
  cmd += "\"";
  sendDebug(cmd);
  delay(5000);
  if (Serial.find(target1))
  {
    monitor.println("RECEIVED: OKWIFI");
    return true;
  } else
  {
    monitor.println("RECEIVED: ErrorWIFI");
    return false;
  }
}
//读GY30---------------------------------------------
int BH1750_Read(int address) 
{
  int i=0;
  Wire.beginTransmission(address);
  Wire.requestFrom(address, 2);
  while(Wire.available()) 

  {
    buff[i] = Wire.read();  // 读IIC数据
    i++;
  }
  Wire.endTransmission();  
  return i;
}
//GY30初始化--------------------------------------------
void BH1750_Init(int address) 
{
  Wire.beginTransmission(address);
  Wire.write(0x10);
  Wire.endTransmission();
}
