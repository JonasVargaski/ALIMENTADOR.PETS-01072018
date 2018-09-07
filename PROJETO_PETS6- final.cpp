#include <EEPROM.h>
#include <LiquidCrystal.h>

#define AUTO true
#define MANUAL false

#define idle_time "00:00:00"


#define BALANCA A0
#define MOTOR 5

#define B_SET 4
#define B_MAIS 3
#define B_MENU 2


LiquidCrystal lcd(8, 9, 10, 11, 12, 13);


byte hour =0,minute=0,second=0,screen=0,count=0,count2=0,flag_screen,time_exit,button_pressed=0,qtd_meal,_hour,_minute,_second,timeout_motor=0;
bool debug=false, flag_bmenu=false,flag_bset=false,flag_bmais=false,b_mais_press=false,b_set_press=false,b_menu_press=false,blink=false,ajust=false,flag_motor_ON=false;;
bool _sys_status=false,time_button=false,mode=false;
int load_food=230,quantity_food,pet_weight,qtd_food,ajust_posit=0;
char line1[16];
char line2[16];
char current_hour[9];
char time1[9];
char time2[9];
char time3[9];
char time4[9];
char time5[9];

//*****************************************************************************************************************************
// Protótipo das funcçoes
void refreshDisplay();
void readButtons();
boolean buttonPress();
void control();
void setClock();
void mainScreen();
void petParameters();
void quantityFood();
void setTime1();
void setTime2();
void setTime3();
void setTime4();
void setTime5();
void blinkDisplay(char start, char end);
void writeEeprom16(unsigned int data,unsigned char adress);
unsigned int readEeprom16(unsigned char adress);
void readParameters();


//*****************************************************************************************************************************


void setup() {

  // Configuração dos Pinos
  pinMode(7,OUTPUT);

  pinMode(BALANCA,INPUT);
  pinMode(B_MENU,INPUT);
  pinMode(B_SET,INPUT);
  pinMode(B_MAIS,INPUT);
  pinMode(MOTOR,OUTPUT);
  lcd.begin(16, 2);
  Serial.begin(9600);
  readParameters();
  
//*****************************************************************************************************************************
// Configuração do timer1
  TCCR1A = 0;                        //confira timer para operação normal
  TCCR1B = 0;                        //limpa registrador
  TCCR1B |= (1<<CS10)|(1 << CS12);   // configura prescaler para 1024
  TCNT1 = 0xC2F7;                    // incia timer com valor para que estouro ocorra em 1 segundo
                                     // 65536-(16MHz/1024/1Hz) = 49911 = 0xC2F7
  TIMSK1 |= (1 << TOIE1);           // habilita a interrupção do TIMER1
//*****************************************************************************************************************************
// Configuração do Timer2  
  TCCR2A = 0x00;   //Timer operando em modo normal
  TCCR2B = 0x07;   //Prescaler 1:1024
  TCNT2  = 0x00;    //10 ms overflow again
  TIMSK2 = 0x01;   //Habilita interrupção do Timer2
//*****************************************************************************************************************************

}//end setup
//*****************************************************************************************************************************
//interrupção do TIMER1
ISR(TIMER1_OVF_vect)                             
{
  TCNT1 = 0xC2F7;                                 // Renicia o registrador do TIMER1
  second++;
  time_exit++;
  timeout_motor++;

  if(time_exit>9){
    time_exit = 0;
  }
 if(second>59){
   second=0;
   minute++;
 }
 if(minute>59){
  minute=0;
  hour++;
 }
 if(hour>23){
  hour=0;
 }
 sprintf(current_hour,"%02d:%02d:%02d",hour,minute,second);
}//end ISR
//*****************************************************************************************************************************
// Interrupção Timer2
ISR(TIMER2_OVF_vect)                              
{
    if(count>5){
    count=0;
    readButtons();
    }
    if(count2>35){
      blink=!blink;
      time_button=!time_button;
      count2=0;
    }
    count++;
    count2++;
    TCNT2=0x00;                                    // Renicia o registrador do TIMER1
}//end ISR
//*****************************************************************************************************************************
//Loop

void loop() {
  
   refreshDisplay();
   control();

 switch(screen){
      case 0: setClock();
      break;
      case 1: mainScreen();
      break;
      case 2: petParameters();
      break;
      case 3: quantityFood();
      break;
      case 4 :setTime1();
      break;
      case 5: setTime2();
      break;
      case 6: setTime3();
      break;
      case 7: setTime4();
      break;
      case 8: setTime5();
      break;
      default: screen=1;
      break;
  }
}//end loop

void setClock(){
  ajust=true;
  sprintf(line1,"  SETUP CLOCK");
  sprintf(line2,current_hour);
if(ajust_posit == 1){
  if(buttonPress(B_MAIS)){
    hour++;
  }
}
if(ajust_posit == 2){
  if(buttonPress(B_MAIS)){
     minute++;
  if(minute>59) minute=0; 
  }
}
if(ajust_posit == 3){
   if(buttonPress(B_MAIS)){
      second++;
      if(second>59)second=0;
    }
  }
if(buttonPress(B_SET)){
  ajust_posit ++;
  if(ajust_posit>3) ajust_posit=0;
}
 if(buttonPress(B_MENU)){
    screen++;
  }
  if(ajust_posit == 1){
 blinkDisplay(0,1);
  }
  if(ajust_posit == 2){
 blinkDisplay(3,4);
  }
  if(ajust_posit == 3){
 blinkDisplay(6,7);
  }
} 

void mainScreen(){
  if(buttonPress(B_SET)){
    _sys_status=!_sys_status;
    EEPROM.write(4,_sys_status);
  }
  if(buttonPress(B_MENU)){
    screen++;
  }
  if(buttonPress(B_MAIS) && !_sys_status){
    digitalWrite(MOTOR,!digitalRead(MOTOR));
  }
  sprintf(line1,"CLOCK: %s",current_hour);
  sprintf(line2,"QTD: %04dg  %s",load_food,_sys_status?"ON ":"OFF"); 
}

void petParameters(){
if(buttonPress(B_MENU)){
  screen++;
  EEPROM.write(1,mode);
  writeEeprom16(pet_weight,2);
}
if(buttonPress(B_SET)){
  ajust=true;
  ajust_posit++;  
  if(ajust_posit>2){
    ajust_posit=0;
  }
 }
  if(ajust_posit==2){
 if(buttonPress(B_MAIS)){
  mode=!mode;
 }
  }
  if(ajust_posit == 1 && pet_weight>=1000){
  if(buttonPress(B_MAIS)){
    pet_weight = (pet_weight+110);
      if(pet_weight > 9999) pet_weight =0;
  }
} 
  if(ajust_posit == 1){  
if(buttonPress(B_MAIS)){
  pet_weight=pet_weight+10;
  }
} 
sprintf(line1, "PET WEIGHT/MODE");
sprintf(line2,"%04d %s | %s",pet_weight,pet_weight<=1000?"gr":"kg",mode?"AUTO":"MANU");
if(ajust_posit==1){
    blinkDisplay(0,3);
  }
  if(ajust_posit==2){
    blinkDisplay(10,13);
  }
if(pet_weight>1000){
  line2[1]='.';
}
}

void quantityFood(){
  if(buttonPress(B_MENU)){
  screen++;
  writeEeprom16(qtd_food,6);
  }
  if(buttonPress(B_SET)){
    ajust=true;      
    ajust_posit++;   
    if(ajust_posit >1 | mode==true) ajust_posit=0;
  }
  if(buttonPress(B_MAIS)){
    if(ajust_posit==1){
      qtd_food+=10;
    }
    if(qtd_food>=1000){
      qtd_food=0;
    }
   }
  sprintf(line1,"QUANTITY FOOD");
  sprintf(line2,"%03dg per %d MEAL",qtd_food,qtd_meal);

  if(ajust_posit == 1){
    blinkDisplay(0,2);
  }
}

void setTime1(){
if(buttonPress(B_MENU)){
  screen++;
   }
if(buttonPress(B_SET)){
  ajust=true;
  ajust_posit++;
  if(ajust_posit>5) ajust_posit=0;
}
if(ajust_posit==0){
   _hour= EEPROM.read(10);
  _minute = EEPROM.read(11);
  _second = EEPROM.read(12);
  ajust_posit++;
}
if(ajust_posit==2){
  if(buttonPress(B_MAIS)){
    _hour++;
    if(_hour>23) _hour=0;
  }
}
if(ajust_posit==3){
  if(buttonPress(B_MAIS)){
    _minute++;
    if(_minute>59) _minute=0; 
  }
}
if(ajust_posit==4){
  if(buttonPress(B_MAIS)){
    _second++;
    if(_second>59)_second=0;
  }
}
sprintf(line1,"FOOD TIME 1");
sprintf(line2,"%02d:%02d:%02d",_hour,_minute,_second);
  if(ajust_posit == 2){
   blinkDisplay(0,1);
  }
  if(ajust_posit == 3){
   blinkDisplay(3,4);
  }
  if(ajust_posit == 4){
   blinkDisplay(6,7);
  }
  if(ajust_posit==5){
  EEPROM.write(10,_hour);
  EEPROM.write(11,_minute);
  EEPROM.write(12,_second);
  sprintf(time1,"%02d:%02d:%02d",_hour,_minute,_second);
  ajust_posit=1;
  }
}

void setTime2(){
if(buttonPress(B_MENU)){
  screen++;
   }
if(buttonPress(B_SET)){
  ajust=true;
  ajust_posit++;
  if(ajust_posit>5) ajust_posit=0;
}
if(ajust_posit==0){
   _hour= EEPROM.read(13);
  _minute = EEPROM.read(14);
  _second = EEPROM.read(15);
  ajust_posit++;
}
if(ajust_posit==2){
  if(buttonPress(B_MAIS)){
    _hour++;
    if(_hour>23) _hour=0;
  }
}
if(ajust_posit==3){
  if(buttonPress(B_MAIS)){
    _minute++;
    if(_minute>59) _minute=0; 
  }
}
if(ajust_posit==4){
  if(buttonPress(B_MAIS)){
    _second++;
    if(_second>59)_second=0;
  }
}
sprintf(line1,"FOOD TIME 2");
sprintf(line2,"%02d:%02d:%02d",_hour,_minute,_second);
  if(ajust_posit == 2){
   blinkDisplay(0,1);
  }
  if(ajust_posit == 3){
   blinkDisplay(3,4);
  }
  if(ajust_posit == 4){
   blinkDisplay(6,7);
  }
  if(ajust_posit==5){
  EEPROM.write(13,_hour);
  EEPROM.write(14,_minute);
  EEPROM.write(15,_second);
  sprintf(time2,"%02d:%02d:%02d",_hour,_minute,_second);
  ajust_posit=1;
  }
}

void setTime3(){
if(buttonPress(B_MENU)){
  screen++;
   }
if(buttonPress(B_SET)){
  ajust=true;
  ajust_posit++;
  if(ajust_posit>5) ajust_posit=0;
}
if(ajust_posit==0){
   _hour= EEPROM.read(16);
  _minute = EEPROM.read(17);
  _second = EEPROM.read(18);
  ajust_posit++;
}
if(ajust_posit==2){
  if(buttonPress(B_MAIS)){
    _hour++;
    if(_hour>23) _hour=0;
  }
}
if(ajust_posit==3){
  if(buttonPress(B_MAIS)){
    _minute++;
    if(_minute>59) _minute=0; 
  }
}
if(ajust_posit==4){
  if(buttonPress(B_MAIS)){
    _second++;
    if(_second>59)_second=0;
  }
}
sprintf(line1,"FOOD TIME 3");
sprintf(line2,"%02d:%02d:%02d",_hour,_minute,_second);
  if(ajust_posit == 2){
   blinkDisplay(0,1);
  }
  if(ajust_posit == 3){
   blinkDisplay(3,4);
  }
  if(ajust_posit == 4){
   blinkDisplay(6,7);
  }
  if(ajust_posit==5){
  EEPROM.write(16,_hour);
  EEPROM.write(17,_minute);
  EEPROM.write(18,_second);
  sprintf(time3,"%02d:%02d:%02d",_hour,_minute,_second);
  ajust_posit=1;
  }
}

void setTime4(){
if(buttonPress(B_MENU)){
  screen++;
   }
if(buttonPress(B_SET)){
  ajust=true;
  ajust_posit++;
  if(ajust_posit>5) ajust_posit=0;
}
if(ajust_posit==0){
   _hour= EEPROM.read(19);
  _minute = EEPROM.read(20);
  _second = EEPROM.read(21);
  ajust_posit++;
}
if(ajust_posit==2){
  if(buttonPress(B_MAIS)){
    _hour++;
    if(_hour>23) _hour=0;
  }
}
if(ajust_posit==3){
  if(buttonPress(B_MAIS)){
    _minute++;
    if(_minute>59) _minute=0; 
  }
}
if(ajust_posit==4){
  if(buttonPress(B_MAIS)){
    _second++;
    if(_second>59)_second=0;
  }
}
sprintf(line1,"FOOD TIME 4");
sprintf(line2,"%02d:%02d:%02d",_hour,_minute,_second);
  if(ajust_posit == 2){
   blinkDisplay(0,1);
  }
  if(ajust_posit == 3){
   blinkDisplay(3,4);
  }
  if(ajust_posit == 4){
   blinkDisplay(6,7);
  }
  if(ajust_posit==5){
  EEPROM.write(19,_hour);
  EEPROM.write(20,_minute);
  EEPROM.write(21,_second);
  sprintf(time4,"%02d:%02d:%02d",_hour,_minute,_second);
  ajust_posit=1;
  }
}

void setTime5(){
if(buttonPress(B_MENU)){
  screen++;
   }
if(buttonPress(B_SET)){
  ajust=true;
  ajust_posit++;
  if(ajust_posit>5) ajust_posit=0;
}
if(ajust_posit==0){
   _hour= EEPROM.read(22);
  _minute = EEPROM.read(23);
  _second = EEPROM.read(24);
  ajust_posit++;
}
if(ajust_posit==2){
  if(buttonPress(B_MAIS)){
    _hour++;
    if(_hour>23) _hour=0;
  }
}
if(ajust_posit==3){
  if(buttonPress(B_MAIS)){
    _minute++;
    if(_minute>59) _minute=0; 
  }
}
if(ajust_posit==4){
  if(buttonPress(B_MAIS)){
    _second++;
    if(_second>59)_second=0;
  }
}
sprintf(line1,"FOOD TIME 5");
sprintf(line2,"%02d:%02d:%02d",_hour,_minute,_second);
  if(ajust_posit == 2){
   blinkDisplay(0,1);
  }
  if(ajust_posit == 3){
   blinkDisplay(3,4);
  }
  if(ajust_posit == 4){
   blinkDisplay(6,7);
  }
  if(ajust_posit==5){
  EEPROM.write(22,_hour);
  EEPROM.write(23,_minute);
  EEPROM.write(24,_second);
  sprintf(time5,"%02d:%02d:%02d",_hour,_minute,_second);
  ajust_posit=1;
  }
}
//*************************************************************************************************************

void readButtons(){

 if(!digitalRead(B_MENU)) flag_bmenu = true;
  if(digitalRead(B_MENU) && flag_bmenu){ 
    b_menu_press = true;
    flag_bmenu=false;
    time_exit=0;
    }
  if(!digitalRead(B_MAIS)) flag_bmais = true;
  if(digitalRead(B_MAIS) && flag_bmais){ 
    b_mais_press=true;
    flag_bmais=false;
    time_exit=0;
    }
  if(!digitalRead(B_SET)) flag_bset = true;
  if(digitalRead(B_SET) && flag_bset){
    b_set_press=true; 
    flag_bset=false;
    time_exit=0;
    }
    if(flag_bmais && time_button==true){
      button_pressed++;
      time_exit=0;
    }
}

boolean buttonPress(byte button){
  if(button == B_MENU && b_menu_press){
    b_menu_press=false;
    return true;
  }
  if(button == B_MAIS && b_mais_press){
    b_mais_press=false;
    button_pressed=0;
    return true;
  }
  if(button == B_SET && b_set_press){
    b_set_press=false;
    return true;
  }
  if(button == B_MAIS && button_pressed>4){
    delay(25);
    blink=false;
    return true;    
  }
  return false;
}

void refreshDisplay(){
if(time_exit>=8 && screen!= 0 && screen != 1){
  screen = 1;
  time_exit = 0;
}
if(flag_screen!= screen){
  flag_screen = screen;
  ajust_posit=0;
  ajust=false;
  _hour=0;_minute=0;_second=0;
  sprintf(line1," ");
  sprintf(line2," ");
  lcd.clear();
  }
  delay(10);
  lcd.setCursor(0,0);
  delay(5);
  lcd.print(line1);
  delay(20);
  lcd.setCursor(0,1);
  delay(5);
  lcd.print(line2);
  delay(20); 
}

void blinkDisplay(char start, char end){
  if(blink && ajust){
    for(char i=start;i<=end;i++){
      line2[i]=' ';
    }
  }
}

void writeEeprom16(unsigned int data,unsigned char adress){
EEPROM.write(adress,data);
data>>=8;
EEPROM.write(adress+1,data);
} 

 unsigned int readEeprom16(unsigned char adress){
 unsigned int data;
 data = EEPROM.read(adress+1);
 data<<=8;
 data|=EEPROM.read(adress);
 return data;
 }

 void readParameters(){
  quantity_food = EEPROM.read(0);
  mode = EEPROM.read(1);
  pet_weight = readEeprom16(2);
  _sys_status= EEPROM.read(4);
  qtd_food = readEeprom16(6);
 
  _hour= EEPROM.read(10);
  _minute = EEPROM.read(11);
  _second = EEPROM.read(12);
  delay(20);
  sprintf(time1,"%02d:%02d:%02d",_hour,_minute,_second);
   _hour= EEPROM.read(13);
  _minute = EEPROM.read(14);
  _second = EEPROM.read(15);
  delay(20);
  sprintf(time2,"%02d:%02d:%02d",_hour,_minute,_second);
   _hour= EEPROM.read(16);
  _minute = EEPROM.read(17);
  _second = EEPROM.read(18);
  delay(20);
  sprintf(time3,"%02d:%02d:%02d",_hour,_minute,_second);
   _hour= EEPROM.read(19);
  _minute = EEPROM.read(20);
  _second = EEPROM.read(21);
  delay(20);
  sprintf(time4,"%02d:%02d:%02d",_hour,_minute,_second);
   _hour= EEPROM.read(22);
  _minute = EEPROM.read(23);
  _second = EEPROM.read(24);
  delay(20);
  sprintf(time5,"%02d:%02d:%02d",_hour,_minute,_second);
 }

void control(){

unsigned int _load_food=0;
  for(char i=0;i<10;i++){
    _load_food+= analogRead(BALANCA);
  }
  load_food = (_load_food/10); // APLICAR FORMULA DE CONVERSAO PARA GRAMAS

 qtd_meal =0; 	
 if(strcmp(time1,idle_time)!=0) qtd_meal++;
 if(strcmp(time2,idle_time)!=0) qtd_meal++;
 if(strcmp(time3,idle_time)!=0) qtd_meal++;
 if(strcmp(time4,idle_time)!=0) qtd_meal++;
 if(strcmp(time5,idle_time)!=0) qtd_meal++;

  if(mode){
   qtd_food = ((pet_weight*0.5)/qtd_meal); // 5% do peso do cachorro, p  porte medio; resultado em gramas.
 }


if(_sys_status){
  if(strcmp(current_hour,time1)==0 && strcmp(time1,idle_time)!=0){
    flag_motor_ON = true;
    timeout_motor = 0;
  }
  else if(strcmp(current_hour,time2)==0 && strcmp(time2,idle_time)!=0){
    flag_motor_ON = true;
    timeout_motor = 0;
  }
  else if(strcmp(current_hour,time3)==0 && strcmp(time3,idle_time)!=0){
    flag_motor_ON = true;
    timeout_motor = 0;
  }
  else if(strcmp(current_hour,time4)==0 && strcmp(time4,idle_time)!=0){
    flag_motor_ON = true;
    timeout_motor = 0;
  }
  else if(strcmp(current_hour,time5)==0 && strcmp(time5,idle_time)!=0){
    flag_motor_ON = true;
    timeout_motor = 0;
    }
    if(flag_motor_ON && timeout_motor < 20 && load_food <= qtd_food){
    digitalWrite(MOTOR,HIGH);
    }else {
        digitalWrite(MOTOR,LOW);
        timeout_motor = 0;
        flag_motor_ON = false;
      } 
  }
}
