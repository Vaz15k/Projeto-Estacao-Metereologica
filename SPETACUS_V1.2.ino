// Keven Eduardo Vaz Bilibio
// Github: @Vaz_15K
// 24/01\2023

String versao  = "SPETACUS V1.2";

// Changelog:
//              Mais Limpeza do codigo e melhorias
//              Substituição do sensor e da lib AM2315;
//              Novo sensor SHT15 com sua respectiva lib SHT1x;

//OBS: Usado Arduino Mega 2560.

//grau   tensão (5.1 Koh)  tensao (10 koh)
//0 1.69  2.51
//45  1 1.68
//90  0.72  1.25
//135 0.55  1
//180 0.45  0.85
//225 0.38  0.71
//270 0.33  0.62
//315 0.28  0.56

#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>
#include <SHT1x.h>
#include <Wire.h>

#define SEALEVELPRESSURE_HPA (1013.25)

LiquidCrystal lcd(8, 9, 4, 5, 6, 7); //(12, 11, 5, 4, 3, 2);//

String AP = ""; //Colocar o nome da rede entre as aspas
String PASS = ""; // Colocar a senha da rede entre as aspas

String API = "";      // Use aqui a chave de autenticação da sua nuvem
String HOST = "api.thingspeak.com";   // O Host da sua nuvem
String PORT = "80";                   // Pota da sua Nuvem

//String field = "field1";
int countTrueCommand;
int countTimeCommand;
bool found = false;
String strteste = " ";

long ti_laco = 0;
long dt_laco = 0;
long dt_nuvem_max = 30000; // manda pra nuvem
long conta_mss_nuvem = 0;
int nu = 10; //faz a leitura de dados  10 vezes antes de enviar pra nuvem/armazenar.
long dt_laco_max = dt_nuvem_max / nu;
long ti_nuvem = 0;
long dt_nuvem = 0;
long dt_fecha = 0;
long ti_fecha = 0;
long dt_fecha_max = 0 ;
int Ffecha = 0;

// Variaveis de medição
// obs: Variaveis de pressao ficaram aqui no codigo, porem nao possui um sensor responsavel por medir isso.

float aTemp[9];
float aUmi[9];
float aPre[9];
int ct_a = 0; /// conta array indice
float temperature, humidity;
float Temp_i = temperature; float Temp_acu = 0; float Temp_med;
float Temp_max = 0; float Temp_min = 100;
float Umi_i = humidity; float Umi_acu = 0; float Umi_med;
float Umi_max = 0; float Umi_min = 100;
float Pre_i; float Pre_acu = 0; float Pre_med;
float Pre_max = 0; float Pre_min = 5000;

//// VARIÁVEIS DO GANGORRA////
const int REED = 31;              //The reed switch outputs to digital pin 31
// Variáveis:
int val = 0;                     //Current value of reed switch
int old_val = 0;                 //Old value of reed switch
int pu_ct = 0;                   // conta pulso da gagorra
float pu_mm = 4;                 // constante de calibração pulso por mm
float mm = 0;
float mmho = 0;                  // intensidade de chuva (mm/h)

//Variaveis do Sensor de Temp/Hum
#define dataPin  20
#define clockPin 21
SHT1x sht1x(dataPin, clockPin);

// VARIÁVEIS DA ANEMOMETRO///////
const float pi = 3.14159265;     // Numero pi
float PPS = 0;             // Tempo de medida(miliseconds)
int radius = 147;              // Raio do anemometro(mm)
unsigned int ct_vento = 0; // magnet counter for sensor
unsigned int RPM = 0;          // Revolutions per minute
float ve_vento_i  = 0;         //  km/h
float ve_vento_acu = 0;
float ve_vento_med = 0;
float ve_vento_max = 0;
float ve_vento_min = 300;

// VARIÁVEIS DA BIRUTA
int bir_pin = 8; /// pino analógico da biruta 
float bir_volt_i = 0;
float bir_volt_acu = 0;
float bir_volt_max = 0;
float bir_volt_min = 0;
float bir_volt_med = 0;
int vento_dir = 0;

///VARIÁVEIS DO ARMAZENAMENTO
int CS_pin = 4; //Define pino para modulo SD
File file;
long fileSize = 0; //Variavel para monitorar tamanho do arquivo
String arq_log = " ";
int ct_algo = 0;

//VARIAVEIS PARA MANIPULACAO DE DADOS DE COMANDO DO PC (HARD SERIAL 0)///
String inputString0 = "";         // variavel tipo string to hold incoming data
boolean stringComplete0 = false;  // whether the string is complete
String inputString1 = "";         // variavel tipo string to hold incoming data on GPS port
boolean stringComplete1 = false;  // whether the GPS string is complete
String inputString2 = "";         // variavel tipo string to hold incoming data on GPS port
boolean stringComplete2 = false;  // whether the GPS string is complete
String inputString3 = "";         // variavel tipo string to hold incoming data on GPS port
boolean stringComplete3 = false; // whether the GPS string is complete
String gpsSeg;
String RMCout;
float  LatRo;
float  LonRo;
float  VelRo;
String TimeRo;
String DateRo;
String DateCo;
float LatCo;
float LonCo;
String TimeCo;
String TagCo;
float VelCo;

void setup() {
  
  Serial.begin(9600);
  Serial1.begin(115200);   //baud rate padrão do ESP8266
  Serial3.begin(4800);
 
  pinMode (REED, INPUT_PULLUP); //This activates the internal pull up resistor

  attachInterrupt(0, addcount, RISING);
  pinMode(2, INPUT);
  digitalWrite(2, HIGH);
  Serial.println("BUENAS...");
  lcd.begin(16, 2);
  lcd.print("BUENAS..");
  lcd.setCursor(0, 1);
  lcd.print(versao);
  
  Serial.println(versao);
  pinMode(2, INPUT);
  digitalWrite(2, HIGH);
  
  Serial.print("verificando cartão...");
  if (!SD.begin(CS_pin))
  {
    Serial.println("Cartao deu  pau");
    //    lcd.clear();
    //    lcd.setCursor(0, 0);
    //    lcd.print("Cartao deu pau");
    delay(5000);
  }
  else {
    Serial.println("Cartao Joinha");
    //    lcd.clear();
    //    lcd.setCursor(0, 0);
    //    lcd.print("Cartao beleza");

    delay(5000);
  }

  File raiz;
  raiz = SD.open("/");

  printDirectory(raiz, 0);
  Serial.println(ct_algo);
  arq_log = "LOG" + String(ct_algo) + ".TXT";
  Serial.println(arq_log);
  Serial.println("Inicia WIFI...");
  sendCommand("AT", 5, "OK");
  sendCommand("AT+CWMODE=1", 5, "OK");
  sendCommand("AT+CWJAP=\"" + AP + "\",\"" + PASS + "\"", 20, "OK");
  Serial.println("Iniciando coleta...");
  ti_laco = millis();
  ti_nuvem = millis();
  lcd.clear();
}

void loop() {
  if (stringComplete3)
  { //GPS string
    //Ti = millis();
    // Serial.println(inputString1);
    lcd.setCursor(0, 0);
    lcd.print(gpsSeg); 
    String pre = "";
    for (int i = 0; i < 6; i++) {
      pre += inputString3.charAt(i);
    }
    if (pre ==  "$GPRMC") {
      RMCout = ParseGPRMC(inputString3);
      //Serial.println(inputString3);
      //Serial3.println(inputString1);

    }
    else if (pre ==  "$GPGGA") {
      //Serial.println(inputString1);
      //Serial3.println(inputString1);
    }
    inputString3 = "";
    stringComplete3 = false;
  }

// Le os valores do Sensor
  Temp_i = sht1x.readTemperatureC();
  Umi_i = sht1x.readHumidity();

  val = digitalRead(REED);                   //Read the status of the Reed swtich
  if ((val == LOW) && (old_val == HIGH)) {   //Check to see if the status has changed
    delay(300);                              // Delay put in to deal with any "bouncing" in the switch.
    pu_ct = pu_ct + 1;                       //pulso da gangorra
    mm = float(pu_ct) / pu_mm;
    old_val = val;                           //Make the old value equal to the current value
  }
  else {
    old_val = val;                           //If the status hasn't changed then do nothing
  }
  dt_laco = millis() - ti_laco;
  dt_nuvem = millis() - ti_nuvem;

  if (dt_laco > dt_laco_max)
  {
    PPS = float(ct_vento) / float(dt_laco_max / 1000);
    RPM = PPS * 60;
    


    bir_volt_i = analogRead(bir_pin) * (5.0 / 1023.0);

    ve_vento_i = (((4 * pi * radius * RPM) / 60) / 1000) * 3.6; // Calcula a velocidade do vento em Km/h
    Umi_acu = Umi_acu + Umi_i;
    Pre_acu = Pre_acu + Pre_i;
    Temp_acu = Temp_acu + Temp_i;
    ve_vento_acu = ve_vento_acu + ve_vento_i;
    bir_volt_acu = bir_volt_acu + bir_volt_i;

    // min max para direcao do vento
    if (bir_volt_i > bir_volt_max)
    {
      bir_volt_max = bir_volt_i;
    }
    if (bir_volt_i < ve_vento_min)
    {
      bir_volt_min = bir_volt_i;
    }

    // min max para velocidade do vento
    if (ve_vento_i > ve_vento_max)
    {
      ve_vento_max = ve_vento_i;
    }
    if (ve_vento_i < ve_vento_min)
    {
      ve_vento_min = ve_vento_i;
    }

    // MIN MAX  para temperatura
    if (Temp_i > Temp_max) {
      Temp_max = Temp_i;
    }
    
    if (Temp_i < Temp_min) {
      
      Temp_min = Temp_i;
    }
    
    /// MIN MAX para Pre
    if (Pre_i > Pre_max) {
      Pre_max = Pre_i;
    }
    
    if (Pre_i < Pre_min){
      Pre_min = Pre_i;
    }

    // MIN MAX para Umi
    if (Umi_i > Umi_max) {
      Umi_max = Umi_i;
      }
      
    if (Umi_i < Umi_min) {
      Umi_min = Umi_i;
    }
    
    Serial.print(Temp_i); Serial.print(";"); Serial.print(Umi_i); Serial.print(";"); Serial.print(Pre_i); Serial.print(";");
    Serial.print(pu_ct); Serial.print(";"); Serial.print(mm); Serial.print(";"); Serial.print("vento: "); Serial.print(RPM); Serial.print(";");
    Serial.print(ve_vento_i); Serial.print(";"); Serial.print(bir_volt_i); Serial.print(";");  Serial.print(ct_a); Serial.print(";"); Serial.println(dt_laco);
    
    lcd.setCursor(0, 1);
    lcd.print(Temp_i);lcd.print(";"); lcd.print(Umi_i);lcd.print(";");lcd.print(ve_vento_i);
    ct_vento = 0;
    ct_a++;
    ti_laco = millis();
  }

  if (dt_nuvem > dt_nuvem_max) {
    
    conta_mss_nuvem  = conta_mss_nuvem + 1;
    Temp_med = Temp_acu / (ct_a);
    Umi_med = Umi_acu / (ct_a);
    Pre_med = Pre_acu / (ct_a);
    ve_vento_med = ve_vento_acu / (ct_a);
    bir_volt_med = bir_volt_acu / ct_a;
    mmho = (mm / (dt_nuvem / 1000)) * 3600; //calcula intensidade me mm/hora
    //    Serial.print("$;");Serial.print(conta_mss_nuvem);Serial.print(";");
    //    Serial.print(Temp_med); Serial.print(";"); Serial.print(Temp_min); Serial.print(";"); Serial.print(Temp_max); Serial.print(";");
    //    Serial.print(Umi_med); Serial.print(";"); Serial.print(Umi_min); Serial.print(";"); Serial.print(Umi_max); Serial.print(";");
    //    Serial.print(Pre_med); Serial.print(";"); Serial.print(Pre_min); Serial.print(";"); Serial.print(Pre_max); Serial.print(";");
    //    Serial.print(ve_vento_med); Serial.print(";"); Serial.print(ve_vento_min); Serial.print(";"); Serial.print(ve_vento_max); Serial.print(";");
    //    Serial.print(bir_volt_med); Serial.print(";"); Serial.print(bir_volt_min); Serial.print(";"); Serial.print(bir_volt_max); Serial.print(";");
    //    Serial.print(pu_ct); Serial.print(";"); Serial.print(mm); Serial.print(";"); Serial.print(dt_nuvem); Serial.print(";"); Serial.println(mmho);
  Serial.println(DateCo); Serial.println(TimeCo); 
      lcd.setCursor(4, 0);
      lcd.print("Nuvem..");
   
    strteste = "$," + DateCo + "," + TimeCo + "," + String(conta_mss_nuvem) + "," + String(Temp_med, 1) + "," + String(Temp_min, 1) + "," + String(Temp_max, 1) + "," +
               String(Umi_med, 1) + "," +         String(Umi_min, 1) + "," +  String(Umi_max, 1) + "," +
               String(ve_vento_med, 1) + "," +    String(ve_vento_min, 1) + "," + String(ve_vento_max, 1) + "," +
               String(bir_volt_med, 2) + "," + //String(bir_volt_min, 2) + "," + String(bir_volt_max, 2) + "," +
               String(Pre_med, 1) + "," +  String(pu_ct)  + "," +  String(mm, 2) + "," +  String(dt_nuvem) + "," +  String(mmho, 1);

    ct_a = 0;
    pu_ct = 0;
    mm = 0;
    mmho = 0;
    Temp_acu = 0;
    Umi_acu  = 0;
    Pre_acu = 0;
    bir_volt_acu = 0;
    ve_vento_acu = 0;
    ve_vento_max = 0;
    ve_vento_min = 300;
    Temp_max = 0;
    Temp_min = 100;
    Umi_max = 0;
    Umi_min = 100;

    Serial.println(strteste);
    grava_cartao_SD(strteste);
    String strData1 = "GET /update?api_key=" + API + "&field1=" + String(strteste);//+"&field2="+String(strteste);
    sendCommand("AT+CIPMUX=1", 5, "OK");
    sendCommand("AT+CIPSTART=0,\"TCP\",\"" + HOST + "\"," + PORT, 15, "OK");
    sendCommand("AT+CIPSEND=0," + String(strData1.length() + 4), 4, ">");
    //esp8266.println(strData1);
    Serial1.println(strData1);
    //    //Serial.println(strData1);
    ti_fecha = millis();
    ti_nuvem = millis();
    Ffecha  = 1;
    delay(10);
    //    countTrueCommand++;
    //    sendCommand("AT+CIPCLOSE=0", 5, "OK");
  }
  if (Ffecha == 1)
  {
    dt_fecha = millis() - ti_fecha;
    if (dt_fecha > dt_fecha_max)
    {
      countTrueCommand++;
      sendCommand("AT+CIPCLOSE=0", 5, "OK");
      lcd.clear();
      Ffecha = 0;
    }
  }
}

void sendCommand(String command, int maxTime, char readReplay[]) {
  //  Serial.print(countTrueCommand);
  //  Serial.print(". at command => ");
  //  Serial.print(command);
  //  Serial.print(" ");
  
  while (countTimeCommand < (maxTime * 2)) {
    //esp8266.println(command);//at+cipsend
    Serial1.println(command);
    if (Serial1.find(readReplay)) {
      found = true;
      break;
    }
    countTimeCommand++;
  }
  
  if (found == true) {
    // Serial.println("OYI");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  
  if (found == false) {
    //  Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  found = false;
}

  float StrToFloat(String str) {
  char carray[str.length() + 1];
  str.toCharArray(carray, sizeof(carray));
  return atof(carray);
}

void addcount() {
  ct_vento++;
}


void printDirectory(File dir, int numTabs) {
  while (true) {
    
    File entry =  dir.openNextFile();
    ct_algo = ct_algo + 1;
    Serial.println(ct_algo);
    if (! entry) {
      // no more files
      break;
    }
    
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    
    Serial.print(entry.name());
    
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}

void grava_cartao_SD(String uid_card){
  //Abre arquivo no SD para gravacao
  file = SD.open(arq_log, FILE_WRITE);
  file.println(uid_card);
  //Fecha arquivo
  file.close();
}

void serialEvent3() {
  while (Serial3.available()) {
    // get the new byte:
    char inChar = (char)Serial3.read();
    // add it to the inputString:
    inputString3 += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete3 = true;
      //Serial.println(inputString1);
    }
  }
}

//Processa GPS string GPRMC
  String ParseGPRMC(String mss) {
  String gpstime = "";
  String gpsdate = "";
  String gpsstatus = "";
  String lat = "";
  String latd = "";
  String lon = "";
  String lond = "";
  String velo = "";
  String out = "";
  int len;
  int par = 1;
  len = mss.length();
  //Serial.println(mss);
  for (int i = 7; i < len; i++) {
    if (mss.charAt(i) != ','  && par == 1) {
      gpstime += mss.charAt(i);
    }
    
    else if (mss.charAt(i) != ',' && par == 2) {
      gpsstatus += mss.charAt(i);
    }
    
    else if (mss.charAt(i) != ',' && par == 3) {
      lat += mss.charAt(i);
    }
    
    else if (mss.charAt(i) != ',' && par == 4) {
      latd += mss.charAt(i);
    }
    
    else if (mss.charAt(i) != ',' && par == 5) {
      lon += mss.charAt(i);
    }
    
    else if (mss.charAt(i) != ',' && par == 6) {
      lond += mss.charAt(i);
    }
    
    else if (mss.charAt(i) != ',' && par == 7) {
      velo += mss.charAt(i);
    }
    
    else if (mss.charAt(i) != ',' && par == 9) {
      gpsdate += mss.charAt(i);
    }
    
    else if (mss.charAt(i) == ',') {
      par ++;
    }
  }
  
  int horalen =  gpstime.length();
  int k = horalen - 2;
  gpsSeg = "";
  for (k=4; k < 6; k ++) {
    gpsSeg = gpsSeg + gpstime[k];
  }
  
  //Serial.print("velocidade Router");
  // Serial.println(velo);
  LatCo = -lat2latdd(lat);
  LonCo = -lon2londd(lon);
  DateCo = gpsdate;
  TimeCo = gpstime;
  VelCo = StrToFloat(velo) * 1.8;

  return out;
}

  float lat2latdd(String lat) { 
  String latg = "";
  String latm = "";
  latg += lat.charAt(0);
  latg += lat.charAt(1);

  for (int i = 2; i < lat.length() ; i++) {
    latm += lat.charAt(i);
  }
  
  float flat = StrToFloat(latg);
  float flatm = StrToFloat(latm);
  float flatdd = flat + flatm / 60;
  return flatdd;
}

  float lon2londd(String lon) {
  String lon_g = "";
  String lon_m = "";
  lon_g += lon.charAt(0);
  lon_g += lon.charAt(1);
  lon_g += lon.charAt(2);
  for (int i = 3; i < lon.length() ; i++) {
    lon_m += lon.charAt(i);
  }
  
  float flon = StrToFloat(lon_g);
  float flonm = StrToFloat(lon_m);
  float flondd = flon + flonm / 60;
  return flondd;
}
