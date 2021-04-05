#include "GyverButton.h"
#include "si5351mcu.h"  
#include "EEPROM.h"
#include <LiquidCrystal_74HC595.h>

Si5351mcu Si;
LiquidCrystal_74HC595 lcd(11, 13, 12, 1, 3, 4, 5, 6, 7);

GButton up(3, HIGH_PULL, NORM_OPEN);
GButton down(5, HIGH_PULL, NORM_OPEN);
GButton func(4, HIGH_PULL, NORM_OPEN);

//int vollevel = analogRead(A0) / 100;

enum Mode  // Перечисление режимов 
{ 
  SetupFreq,
  Menu,
  Sql,
  InterFreq,
  Power,
  QuartzCall,
  MenuStep
};
Mode mode = SetupFreq;
int const n = 3;
long steps [n] = {1000, 5000, 10000};  // Шаг перестройки 
int stepIndex;      
long F1 = 27200000;    // Частота по умолчанию при запуске синтезатора 
long F2 = F1 - 465000; // 2 выход синтезатора для передатчика: -частота ПЧ !ВЫБОР ПЧ ПЕРЕНЕСТИ В МЕНЮ!
uint32_t TimerTime;
int const NumMenu = 7; 
int CurretMenu = 2;
const char *NameMenu[NumMenu] = {
  "FREQ",
  "MENU",
  "SQL",
  "IF",
  "POW",
  "QzSi",
  "STEP"
  }; 
bool trigButton = false;
int MenuTimeDelay = 5000;
int levelSql;

void setup() {  
  Serial.begin(9600);
  pinMode(12, OUTPUT);
  
  lcd.begin(16, 2);
  
  levelSql = EEPROM.read(0);
  stepIndex = EEPROM.read(4);
  
  Si.init();            // Настройки Si5351
  Si.correction(-1250);
  Si.setPower(0, SIOUT_4mA);
  Si.setPower(1, SIOUT_4mA);
  Si.reset();
    
  up.setDebounce(50); // Настройки кнопок
  up.setTimeout(100);
  up.setStepTimeout(200);
  
  down.setDebounce(50);
  down.setTimeout(100);
  down.setStepTimeout(200);
  
  func.setDebounce(80);
  func.setTimeout(500);
  func.setClickTimeout(300);


}
void setupFreq() {    // Функция главного экрана 
  
  F1 = constrain(F1, 26000000, 28000000);  // Граници работы синтезатора !ПЕРЕНЕСТИ В МЕНЮ!
  
  Si.setFreq(0, F1);
  Si.setFreq(1, F2);
  
  if (up.isPress() || up.isStep()) { // Кнопки с функцией пролистывания
    F1 = F1 + steps[stepIndex];
  }
  if (down.isPress() || down.isStep()) {
    F1 = F1 - steps[stepIndex];
  }
   if (func.isSingle()) {         // Однкратное нажатие изменяет шаг перестройки !ПРИ НАЖАТИИ НА ШАГ НА ЭКРАНЕ ПОКАЗЫВАЕТ ТЕКУЩИЙ
    if (stepIndex < n-1)         // ШАГ ПЕРЕСТРОЙКИ В ТАКОМ ВИДЕ: 1К, 5K, 10K!
      stepIndex = stepIndex + 1; // Зацикленное изменение шага перестройки с помощью массива 
    else 
      stepIndex = 0; 
  }
   lcd.clear();
   lcd.setCursor(12, 0);
   lcd.print(F1 - 20000000);
   
} 
void menu() {       // Функция меню

  if (up.isPress() || up.isStep()) {
    trigButton = true;
    if (CurretMenu < NumMenu - 1)
     CurretMenu = CurretMenu + 1;
    else 
     CurretMenu = 2;  
    //   Serial.println(NameMenu[CurretMenu]);
  }
  if (down.isPress() || down.isStep()) {
    trigButton = true;
    if (CurretMenu != 2)
     CurretMenu < NumMenu - 1,
     CurretMenu = CurretMenu - 1;
    else
     CurretMenu = NumMenu - 1;
    //  Serial.println(NameMenu[CurretMenu]);
  }
      
   if (func.isHold()) {
    TimerTime = millis();
    mode = CurretMenu;
     }  

  if (trigButton) {
   trigButton = false;
   TimerTime = millis();
  }
  if (millis() - TimerTime > MenuTimeDelay) {  // Таймер где millis() - кол-во мс прошедших с момента запуска МК(макс занчение 50суток) 
    TimerTime = millis();             // Перезапись в переменную времени, текущие значение времени millis()
    mode = SetupFreq;
  //  Serial.println("Reset mode menu");
  }
  lcd.clear();
  lcd.setCursor(12, 0),
  lcd.print(NameMenu[CurretMenu]);
}
void sql() {
  
 levelSql = constrain(levelSql, 0, 9);
 
 if (up.isPress()){
  levelSql ++;
  trigButton = true;
 }
 if (down.isPress()) {
  levelSql --;
  trigButton = true;
 }
 
 if (trigButton) {
    trigButton = false;
 // Serial.println(levelSql);
 }
 if (func.isSingle()) {
  TimerTime = millis();
  mode = 1;
 }
 lcd.clear();
 lcd.setCursor(12, 0);
 lcd.print(levelSql);
 EEPROM.update(0, levelSql); // Запись значения в EEPROM
}
void power() {
  
}
void quartzCall() {
  
}
void interFreq() {
 
}
void menustep() {
  
  if (up.isPress()) {         
    if (stepIndex < n-1)         
      stepIndex = stepIndex + 1;
    else 
      stepIndex = 0; 
  }
   if (down.isPress()) {         
    if (stepIndex != 0)         
      stepIndex = stepIndex - 1; 
    else 
      stepIndex = 2;
   }
 if (func.isSingle()) {
  TimerTime = millis();
  mode = 1;
 }
 lcd.clear();
 lcd.setCursor(12, 0);
 lcd.print((steps[stepIndex]) / 1000);
 lcd.setCursor(15, 0);
 lcd.print("K");
 EEPROM.update(4, stepIndex); // Запись значения в EEPROM
}
void loop() {
  
  up.tick();
  down.tick();
  func.tick();
   
  if (func.isDouble()) { // Двойное нажатие на кнопку - включение режима меню
    mode = Menu;
    TimerTime = millis();
  }
  switch (mode) 
  {
  case SetupFreq:
    setupFreq();   // Основная информация при работе (частота)
    break;
  case Menu:
    menu();       // Вход в главное меню настройки радиостанции 
    break;
  case Sql:
    sql();       // Настройки шумоподавителя
    break;
  case Power:
    power();       // Настройки выходной мощности  
    break;
  case QuartzCall:
    quartzCall();       // Калибровка опорного кварца синтезатора
    break;
  case InterFreq:
    interFreq();       // Установка промежуточной частоты приемо-передающего тракта трансивера
    break;
  case MenuStep:
    menustep();
    break;
  }
 //Serial.println(analogRead(A0) / 100);
  if ((analogRead(A0) / 100) > levelSql) {
    Serial.println(digitalRead(12));
    digitalWrite(12, 1);
  }
 
  if (func.isTriple()) Serial.println("Triple"); 

}
