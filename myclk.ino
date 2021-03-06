#define BLINKER_BLE

#include <DS3231.h>
#include <Wire.h>
#include <LedControl.h>
#include <Blinker.h>

int Buzzer = 4;
int DIN = 5;
int CS = 6;
int CLK = 7;

int secnum = 0;
int sec8num = 0;
int inte = 3;
int dispmode=0;
int maintimer;
bool Century = false;
bool h12;
bool PM;
bool isShutdown = false;
bool isBeep = false;
bool tab[5] = {false};
byte year, month, date, DoW, hour, minute, second;
byte heart1[8] = {0x00, 0x66, 0xFF, 0xFF, 0x7E, 0x3C, 0x18, 0x00};
byte heart2[8] = {0x00, 0x00, 0x24, 0x7E, 0x3C, 0x18, 0x00, 0x00};
byte numb[][7] = {
	{0x18, 0x24, 0x24, 0x24, 0x24, 0x24, 0x18}, /*"0",0*/
	{0x08, 0x18, 0x08, 0x08, 0x08, 0x08, 0x18}, /*"1",1*/
	{0x18, 0x24, 0x04, 0x08, 0x10, 0x20, 0x3C}, /*"2",2*/
	{0x18, 0x24, 0x04, 0x18, 0x04, 0x24, 0x18}, /*"3",3*/
	{0x08, 0x18, 0x18, 0x28, 0x28, 0x3C, 0x08}, /*"4",4*/
	{0x3c, 0x20, 0x20, 0x38, 0x04, 0x04, 0x38}, /*"5",5*/
	{0x18, 0x24, 0x20, 0x38, 0x24, 0x24, 0x18}, /*"6",6*/
	{0x3C, 0x24, 0x04, 0x08, 0x10, 0x10, 0x10}, /*"7",7*/
	{0x18, 0x24, 0x24, 0x18, 0x24, 0x24, 0x18}, /*"8",8*/
	{0x18, 0x24, 0x24, 0x1C, 0x04, 0x24, 0x18}, /*"9",9*/
};
byte alph[][7] = {
	{0x30, 0x48, 0x48, 0x48, 0x78, 0x48, 0x48}, /*"A",0*/
	{0x70, 0x48, 0x48, 0x70, 0x48, 0x48, 0x70}, /*"B",1*/
	{0x30, 0x48, 0x40, 0x40, 0x40, 0x48, 0x30}, /*"C",2*/
	{0x70, 0x48, 0x48, 0x48, 0x48, 0x48, 0x70}, /*"D",3*/
	{0x78, 0x40, 0x40, 0x70, 0x40, 0x40, 0x78}, /*"E",4*/
	{0x78, 0x40, 0x40, 0x70, 0x40, 0x40, 0x40}, /*"F",5*/
	{0x30, 0x48, 0x40, 0x58, 0x48, 0x48, 0x30}, /*"G",6*/
	{0x48, 0x48, 0x48, 0x78, 0x48, 0x48, 0x48}, /*"H",7*/
	{0x70, 0x20, 0x20, 0x20, 0x20, 0x20, 0x70}, /*"I",8*/
	{0x08, 0x08, 0x08, 0x08, 0x48, 0x48, 0x30}, /*"J",9*/
	{0x48, 0x48, 0x50, 0x60, 0x50, 0x48, 0x48}, /*"K",10*/
	{0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x78}, /*"L",11*/
	{0x48, 0x78, 0x78, 0x48, 0x48, 0x48, 0x48}, /*"M",12*/
	{0x48, 0x68, 0x68, 0x58, 0x58, 0x48, 0x48}, /*"N",13*/
	{0x78, 0x48, 0x48, 0x48, 0x48, 0x48, 0x78}, /*"O",14*/
	{0x70, 0x48, 0x48, 0x70, 0x40, 0x40, 0x40}, /*"P",15*/
	{0x30, 0x48, 0x48, 0x48, 0x68, 0x58, 0x38}, /*"Q",16*/
	{0x70, 0x48, 0x48, 0x70, 0x60, 0x50, 0x48}, /*"R",17*/
	{0x30, 0x48, 0x40, 0x30, 0x08, 0x48, 0x30}, /*"S",18*/
	{0x78, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20}, /*"T",19*/
	{0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x30}, /*"U",20*/
	{0x48, 0x48, 0x48, 0x48, 0x30, 0x30, 0x30}, /*"V",21*/
	{0x48, 0x48, 0x48, 0x48, 0x78, 0x78, 0x48}, /*"W",22*/
	{0x48, 0x48, 0x30, 0x30, 0x30, 0x48, 0x48}, /*"X",23*/
	{0x88, 0x88, 0x50, 0x20, 0x20, 0x20, 0x20}, /*"Y",24*/
	{0x78, 0x08, 0x10, 0x30, 0x20, 0x40, 0x78}, /*"Z",25*/
};
byte showSec[9] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00};

DS3231 Clock;
LedControl lc = LedControl(DIN, CLK, CS, 4);
BlinkerButton btn_shutdown("btn-shutdown");
BlinkerButton btn_reset("btn-reset");
BlinkerButton btn_beep("btn-beep");
BlinkerSlider ran_inte("ran-inte");
BlinkerTab Tab_state("tab-ned");

void (*resetFunc)(void) = 0;

void beepBegin()
{
	digitalWrite(Buzzer, HIGH);
	isBeep = true;
}

void beepEnd(){
	digitalWrite(Buzzer, LOW);
	isBeep = false;
}

void printByte(int num, byte character[])
{
	int i = 0;
	for (i = 1; i < 8; i++)
	{
		lc.setRow(num, i, character[i - 1]);
	}
}

void uiUpdate()
{
	ran_inte.print(inte);
	if (isShutdown)
		btn_shutdown.color("red");
	else
		btn_shutdown.color("green");
	btn_shutdown.print();
}
void dispHour(int dsn = 0)
{
	hour = Clock.getHour(h12, PM);
	printByte(dsn, numb[hour / 10]);
	printByte(dsn + 1, numb[hour % 10]);
}

void dispMins(int dsn = 2)
{
	minute = Clock.getMinute();
	printByte(dsn, numb[minute / 10]);
	printByte(dsn + 1, numb[minute % 10]);
}

void dispYear()
{
	year = Clock.getYear();
	printByte(0, numb[2]);
	printByte(1, numb[0]);
	printByte(2, numb[year / 10]);
	printByte(3, numb[year % 10]);
	Serial.print(year);
}

void btn_shutdown_callback(const String &state)
{
	BLINKER_LOG("btn_shutdown_callback: ", state);
	if (state == "tap")
	{
		isShutdown = !isShutdown;
		BLINKER_LOG("shutdown state: ", isShutdown);

		for (int i = 0; i < 4; i++)
		{
			lc.shutdown(i, isShutdown);
		}
		if (isShutdown)
			btn_shutdown.color("red");
		else
			btn_shutdown.color("green");

		btn_shutdown.print();
	}
}



void btn_reset_callback(const String &state)
{

	if (state == "tap" && !isShutdown)
	{
		resetFunc();
		BLINKER_LOG("btn_reset state: ", state);
	}
}

void btn_beep_callback(const String &state){
	if(!isBeep)
		beepBegin();
	else
		beepEnd();
}

void ran_inte_callback(int32_t value)
{
	//digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
	BLINKER_LOG("ran_inte_callback: ", value);
	inte = value;
	for (int i = 0; i < 4; i++)
	{
		lc.setIntensity(i, value);
	}
}

void tab_state_callback(uint8_t tab_set)
{
	BLINKER_LOG("tab_state_callback: ", tab_set);

	switch (tab_set)
	{
	case BLINKER_CMD_TAB_0:
		tab[0] = true;
		dispmode = 0;
		BLINKER_LOG("tab 0 set");
		break;
	case BLINKER_CMD_TAB_1:
		tab[1] = true;
		dispmode = 1;
		BLINKER_LOG("tab 1 set");
		break;
	case BLINKER_CMD_TAB_2:
		tab[2] = true;
		dispmode = 2;
		BLINKER_LOG("tab 2 set");
		break;
	case BLINKER_CMD_TAB_3:
		tab[3] = true;
		dispmode = 3;
		BLINKER_LOG("tab 3 set");
		break;
	case BLINKER_CMD_TAB_4:
		tab[4] = true;
		dispmode = 4;
		BLINKER_LOG("tab 4 set");
		break;
	default:
		break;
	}
}

void tab_state_feedback()
{
	for (uint8_t num = 0; num < 5; num++)
	{
		if (tab[num])
		{
			Tab_state.tab(num);
			tab[num] = false;
		}
	}
	Tab_state.print();
}

void dataRead(const String &data)
{
	BLINKER_LOG("Blinker readString: ", data);
	uiUpdate();
}

void prtSec()
{

	if (sec8num == 8)
	{
		lc.setRow(secnum, 0, showSec[sec8num]);
		sec8num = 0;
		secnum++;
		if (secnum > 3)
			secnum = 0;
	}
	lc.setRow(secnum, 0, showSec[sec8num]);
	sec8num++;
}

void setup()
{
	pinMode(Buzzer, OUTPUT);
	Serial.begin(115200);
	BLINKER_DEBUG.stream(Serial);
	maintimer = 0;
	for (int i = 0; i < 4; i++)
	{
		lc.shutdown(i, false); //启动时，MAX72XX处于省电模式
		lc.setIntensity(i, 2); //将亮度设置为最大值
		lc.clearDisplay(i);	   //清除显示
	}
	Wire.begin();
	Blinker.begin(3,2);
	Blinker.attachData(dataRead);
	btn_shutdown.attach(btn_shutdown_callback);
	btn_reset.attach(btn_reset_callback);
	btn_beep.attach(btn_beep_callback);
	ran_inte.attach(ran_inte_callback);
	Tab_state.attach(tab_state_callback, tab_state_feedback);
}

void loop()
{
	Blinker.run();
	dispMins();
	dispHour();
	//dispYear();
	prtSec();
	delay(200);
	//uiUpdate();
	maintimer++;
	if(maintimer==60)
		maintimer = 0;
}
