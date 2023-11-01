#include <SPI.h>//引用SPI头文件
#include <Ethernet.h>  //引用W5100头文件
#include <DHT_U.h> //引用DHT11头文件

// 设定MAC地址、IP地址
// IP地址需要参考你的本地网络设置
#define DHTTYPE DHT11   // DHT 11 (AM2302)
#define DHTPIN PIN2     // DHT 引脚

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };//mac地址
IPAddress ip(10,21,214,36);//IP地址

//----------------------------------

// 初始化Ethernet库
// HTTP默认端口为80

EthernetServer server(80);  //设置Arduino的网页服务器
// 温湿度传感器实例
DHT_Unified dht(DHTPIN, DHTTYPE);

// 设置参数的程序区段，只会执行一次
void setup()
{

    //------------------------------------------------------
    dht.begin();
    Serial.begin(9600); //启动串行通信来观察Arduino运行情况
// 开始ethernet连接，并作为服务器初始化
    Ethernet.begin(mac, ip);  //启动网络功能，设置MAC和IP地址
    server.begin();  //启动网页服务器功能
    Serial.print("server is at ");  //显示Arduino自己的IP
    Serial.println(Ethernet.localIP());
}

float Temperature, Humidity;

// 得到温湿度值并打印到串口和屏幕
void get_temperature_humidity() {

// 得到温度并输出值
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature)) {
        Serial.println(F("Error reading temperature!"));
    } else {
        Temperature = event.temperature;
        Serial.print(F("Temperature:  "));
        Serial.print(Temperature);
        Serial.println(F(" C\n"));
    }

// 得到湿度并输出值
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) {
        Serial.println(F("Error reading humidity!"));
    } else {
        Humidity = event.relative_humidity;
        Serial.print(F("Humidity:     "));
        Serial.print(Humidity);
        Serial.println(F("%"));
    }
}

// 程序会重复执行
void loop()
{

    // 监听客户端传来的数据
    get_temperature_humidity();
    delay(100);
   EthernetClient client = server.available();
    if (client)
    {
       Serial.println("new client");
       //一个HTTP的连接请求，以空行结尾
       boolean currentLineIsBlank = true;
       while (client.connected())
       {
          if (client.available())
          {
            char c = client.read();
            Serial.write(c);

// 如果收到空白行，说明http请求结束，并发送响应消息

           if (c == '\n' && currentLineIsBlank)
            {
             // 标准的HTTP响应标头信息
             client.println("HTTP/1.1 200 OK");
             client.println("Content-Type: text/html");
             client.println("Connection: close");  // 在反应后将关闭连接
             client.println("Refresh: 5");  // 每5秒更新一次网页
             client.println(); //响应标头的后面需要一个空行
             client.println("");  //类型定义，说明此为HTML信息
             client.println("");
             client.println(""); //网页的标题
             //网页内容信息
             client.print("Temperature = ");
             client.print(Temperature); //温度数据
             client.print("C  ");
             client.println("");
             client.print("Humidity = ");
             client.print(Humidity);  //湿度数据
             client.print("%  ");
             client.print("");
             client.println("");
             break;  //跳出while循环，避免浏览器持续处于接收状态
            }
            if(c == '\n')

// 已经开始一个新行
               currentLineIsBlank = true;
            else if (c != '\r')
               currentLineIsBlank = false;

          }
       }

       delay(1);  //停留一些时间让浏览器接收Arduino传送的数据

       client.stop();  //关闭连接
       Serial.println("client disconnected");//串口打印client断开连接。
    }
    delay(500);
}