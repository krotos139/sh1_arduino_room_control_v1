/*
Author: Iuri Iakovlev <krotos139@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

  (Это свободная программа: вы можете перераспространять ее и/или изменять
   ее на условиях Стандартной общественной лицензии GNU в том виде, в каком
   она была опубликована Фондом свободного программного обеспечения; либо
   версии 3 лицензии, либо (по вашему выбору) любой более поздней версии.

   Эта программа распространяется в надежде, что она будет полезной,
   но БЕЗО ВСЯКИХ ГАРАНТИЙ; даже без неявной гарантии ТОВАРНОГО ВИДА
   или ПРИГОДНОСТИ ДЛЯ ОПРЕДЕЛЕННЫХ ЦЕЛЕЙ. Подробнее см. в Стандартной
   общественной лицензии GNU.

   Вы должны были получить копию Стандартной общественной лицензии GNU
   вместе с этой программой. Если это не так, см.
   <http://www.gnu.org/licenses/>.)
*/

#include "DHT.h"
#include "IRremote.h"
#include "SimpleModbusSlave.h"

#define ROOM_CONTROLLER_Z1 0x30

// ===============================================================
#define DHT_S1_PIN 12    // пин для датчика DHT22
// ===============================================================
#define IRMOTION_PIN A2    // пин для датчика движения
// ===============================================================
#define MQ2_PIN A1    // пин для датчика газа
// ===============================================================
#define CURRENT_PIN A0    // пин для тока
// ===============================================================
#define RELAY_0_PIN 3    // пин для реле
#define RELAY_1_PIN 4    // пин для реле
#define RELAY_2_PIN 5    // пин для реле
#define RELAY_3_PIN 6    // пин для реле
#define RELAY_4_PIN 7    // пин для реле
#define RELAY_5_PIN A3    // пин для реле
#define RELAY_6_PIN A4    // пин для реле
#define RELAY_7_PIN A5    // пин для реле
// ===============================================================
#define GRBLED_PIN_R 9    // пин для канала R
#define GRBLED_PIN_G 10    // пин для канала G
#define GRBLED_PIN_B 11    // пин для канала B
// ===============================================================
// State ID's
enum 
{
  STID_OK,
  STID_ERROR,
  STID_LOADING,
  STID_RELAY_WAITLOCUR_ERROR,
  STID_DHT22_ERROR,
  STID_MQ2_ERROR,
  STID_MQ2_CRITICAL_LEVEL,
  STID_MQ2_OVERRIDE,
  STID_CURRENT_CRITICAL_LEVEL,
  STID_CURRENT_OVERRIDE,
};
// registers NUM for modbus slave
enum 
{
  DEVICE_ID,
  STATE_ID,
  
  RELAY_0_VAL,
  RELAY_1_VAL,
  RELAY_2_VAL,
  RELAY_3_VAL,
  RELAY_4_VAL,
  RELAY_5_VAL,
  RELAY_6_VAL,
  RELAY_7_VAL,
  
  RGBLED_R_VAL,
  RGBLED_G_VAL,
  RGBLED_B_VAL,
  
  TEMPERATURE_VAL,
  TEMPERATURE_FRAC_VAL,
  HUMIDITY_VAL,
  HUMIDITY_FRAC_VAL,
  IR_MOTION_VAL,
  MQ2_SENSOR_VAL,
  PL_CURRENT_VAL,
  
  IR_MODE,
  IR_DH,
  IR_DL,
  IR_SIZE,
  
  HOLDING_REGS_SIZE // leave this one
};
unsigned int holdingRegs[HOLDING_REGS_SIZE]; // 16 register array
// ===============================================================
// SETTING
int relay_current_level = 5;
int mq2_alarm = 200;
int current_alarm = 200;
int mb_slave_id = 1;
int mb_device_id = 0xA101;
// ===============================================================
float humidity = 0, temperature = 0;
int irmotion=0;
int mq2_value=0;
int current_value=0;
int relay_wait_counter=0;

dht dht_s1;

IRsend irsend;

// ========================СТАРТУЕМ=============================
void setup(){

  Serial.begin(9600); 
  while (!Serial) ;
//  Serial.println("Room Z1 v 0.2"); // Тестовые строки для отображения в мониторе порта

  // Init modbus
  holdingRegs[DEVICE_ID] = mb_device_id;
  holdingRegs[STATE_ID] = STID_LOADING;
  modbus_configure((HardwareSerial*)&Serial, 9600, SERIAL_8N2, mb_slave_id, TXEN_NULL, HOLDING_REGS_SIZE, holdingRegs);  
  // RGBLED
  pinMode(GRBLED_PIN_R, OUTPUT);
  pinMode(GRBLED_PIN_G, OUTPUT);
  pinMode(GRBLED_PIN_B, OUTPUT);
  // IR MOTION
  pinMode(IRMOTION_PIN, INPUT);
  // GAS SENSOR
  pinMode(MQ2_PIN, INPUT);
  // RELAY
  pinMode(RELAY_0_PIN, OUTPUT);
  pinMode(RELAY_1_PIN, OUTPUT);
  pinMode(RELAY_2_PIN, OUTPUT);
  pinMode(RELAY_3_PIN, OUTPUT);
  pinMode(RELAY_4_PIN, OUTPUT);
  pinMode(RELAY_5_PIN, OUTPUT);
  pinMode(RELAY_6_PIN, OUTPUT);
  pinMode(RELAY_7_PIN, OUTPUT);
}

void loop(){
  // Update holdingRegs
  modbus_update();
  
  holdingRegs[STATE_ID] = STID_OK;
  // Input
  Z1_input_dht();
  Z1_input_irmotion();
  Z1_input_mq2();
  Z1_input_current();
  // Output
  Z1_output_rgbled();
  Z1_output_relay();
  
  Z1_ir_transmitter();
  
 // tv.send(tv.c_power);
}

void Z1_output_rgbled() {
  analogWrite(GRBLED_PIN_R, 255-holdingRegs[RGBLED_R_VAL]);
  analogWrite(GRBLED_PIN_G, 255-holdingRegs[RGBLED_G_VAL]);
  analogWrite(GRBLED_PIN_B, 255-holdingRegs[RGBLED_B_VAL]);
}

void Z1_output_relay() {
  if (current_value<relay_current_level) {
    digitalWrite(RELAY_0_PIN, holdingRegs[RELAY_0_VAL]);
    digitalWrite(RELAY_1_PIN, holdingRegs[RELAY_1_VAL]);
    digitalWrite(RELAY_2_PIN, holdingRegs[RELAY_2_VAL]);
    digitalWrite(RELAY_3_PIN, holdingRegs[RELAY_3_VAL]);
    digitalWrite(RELAY_4_PIN, holdingRegs[RELAY_4_VAL]);
    digitalWrite(RELAY_5_PIN, holdingRegs[RELAY_5_VAL]);
    digitalWrite(RELAY_6_PIN, holdingRegs[RELAY_6_VAL]);
    digitalWrite(RELAY_7_PIN, holdingRegs[RELAY_7_VAL]);
    relay_wait_counter = 0;
  } else {
    if (relay_wait_counter > 1000)
      holdingRegs[STATE_ID] = STID_RELAY_WAITLOCUR_ERROR;
    else
      relay_wait_counter += 1;
  }
}

void Z1_input_dht() {
  // DHT22 (Temp)
  if (dht_s1.read22(DHT_S1_PIN) == DHTLIB_OK) {
    humidity = dht_s1.humidity;
    temperature = dht_s1.temperature;
  } else {
    holdingRegs[STATE_ID] = STID_DHT22_ERROR;
  }
  holdingRegs[TEMPERATURE_VAL] = temperature;
  holdingRegs[TEMPERATURE_FRAC_VAL] = (int)(temperature*100) % 100;
  holdingRegs[HUMIDITY_VAL] = humidity;
  holdingRegs[HUMIDITY_FRAC_VAL] = (int)(humidity*100) % 100;
}

void Z1_input_irmotion() {
  int val;
  val = digitalRead(IRMOTION_PIN);
  if ((irmotion>0) && (val==0)) irmotion -= 1;
  if ((irmotion<100) && (val==1)) irmotion += 1;
  
  holdingRegs[IR_MOTION_VAL] = irmotion;
}

void Z1_input_mq2() {
  mq2_value = analogRead(MQ2_PIN);
  if (mq2_value == 0) holdingRegs[STATE_ID] = STID_MQ2_ERROR;
  if (mq2_value >= mq2_alarm) holdingRegs[STATE_ID] = STID_MQ2_CRITICAL_LEVEL;
  if (mq2_value >= 1023) holdingRegs[STATE_ID] = STID_MQ2_OVERRIDE;
  holdingRegs[MQ2_SENSOR_VAL] = mq2_value;
}

void Z1_input_current() {
  current_value = analogRead(CURRENT_PIN);
  if (current_value >= current_alarm) holdingRegs[STATE_ID] = STID_CURRENT_CRITICAL_LEVEL;
  if (current_value >= 1023) holdingRegs[STATE_ID] = STID_CURRENT_OVERRIDE;
  holdingRegs[PL_CURRENT_VAL] = current_value;
}

void Z1_ir_transmitter() {
  unsigned long data;
  int nbits;
  
  data = (holdingRegs[IR_DH] << 16) + holdingRegs[IR_DL];
  nbits = holdingRegs[IR_SIZE];
  
  switch (holdingRegs[IR_MODE]) {
    case NEC: irsend.sendNEC(data, nbits);
      break;
    case SONY: irsend.sendSony(data, nbits);
      break;
    case RC5: irsend.sendRC5(data, nbits);
      break;
    case RC6: irsend.sendRC6(data, nbits);
      break;
    case DISH: irsend.sendDISH(data, nbits);
      break;
    case SHARP: irsend.sendSharp(data, nbits);
      break;
    case JVC: irsend.sendJVC(data, nbits, 1);
      break;
    case SAMSUNG: irsend.sendSAMSUNG(data, nbits);
      break;
    default:
    case 0:
      break;
  }
  holdingRegs[IR_MODE] = 0;
}

/*
void Z1_SerialOutput() {
  Serial.print("T= "); 
  Serial.print(temperature);
  Serial.print(" *C \t");
  Serial.print("Humidity= "); 
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Motion= "); 
  Serial.print(irmotion);
  Serial.print(" %\t");
  Serial.print("MQ= "); 
  Serial.print(mq_value);
  Serial.print(" \t");
  Serial.print("CUR= "); 
  Serial.print(current);
  Serial.print(" \t");
  Serial.print("\n");
}
*/

