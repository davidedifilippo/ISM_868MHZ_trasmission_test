## ISM_868MHZ_trasmission_test

Temperature trasmission test on Industrial Scientific and Medical 868 MHZ band 

## Dichiarazioni iniziali

Per chi usa Visual Studio Code:

    #include <Arduino.h>

Bisogna includere la libreria per la gestione dell'interfaccia I2C di comunicazione con il sensore TMP102:

    #include <Wire.h>

che come sappiamo prevede l'utilizzo di due piedini del microcontrollore:

D15 SCL Serial Clock
D14 SDA Serial Data

Il microcontrollore scrive i dati da trasmettere e legge i dati ricevuti tramite il transceiver utilizzando il modulo di comunicazione sincrono ad alta velocità SPI (Serial Peripheral Interface)

    #include <SPI.h>

Il transceiver S2LP viene gestito tramite la libreria:

    #include <S2LP.h>

Mentre il sensore di temperatura mediante la libreria:

    #include "tmp102.h"

Per convertire la temperatura numerica in una stringa di caratteri ASCII abbiamo bisogno della libreria:

    #include <string.h>

Imponiamo l'indirizzo I2C del TMP102

    #define TMP102_I2C_ADDRESS 72

Creo un oggetto di tipo SPI, un oggetto di tipo S2LP e un oggetto di tipo TMP102:

    SPIClass *devSPI;
    S2LP *myS2LP;
    tmp102 my_tmp102;

Il flag receive_packet deve essere dicharato volatile in quanto il suo valore in memoria può cambiare a seguito di un interrupt dal transceiver:

    volatile uint8_t receive_packet = 0;

Il pulsante in questa board è collegato sul piedino PC13:

    const int buttonPin = PC13; // set buttonPin to digital pin PC13 */
    int pushButtonState = LOW;
    
Dichiariamo ora due vettori, uno per l'invio di dati e uno per la ricezione:

    static uint8_t send_buf[FIFO_SIZE] ={'T','M','P',' ','1','0','2',':',' '};
    static uint8_t read_buf[FIFO_SIZE] ={0};

Dichiaro i prototipi delle funzioni che verranno richiamate dal loop:

    void callback_func(void);
    void recv_data(void);
    void blink_led(void);

Dichiaro le variabili accessorie:

    bool tmp102_presente = false;
    float T;

## Fase di Setup

Si dichiara la frequenza della portante da utilizzare nelle trasmissioni:  

    uint32_t s_frequency = 868000000; 

Si dichara la frequenza di clock del chip S2LP:

    uint32_t s_RfXtalFrequency = 50000000;
    
Si dichiara la struttura che conterrà i dati del power amplifier (non presente) 
        
      PAInfo_t paInfo;

Si riserva la memoria in modo dinamico:

     memset(&paInfo, 0, sizeof(PAInfo_t));

Nella board Nucleo L152RE il led è montato sul piedino PA5:
  
    pinMode(LED_BUILTIN, OUTPUT);

Non sappiamo se il pulsante a riposo è nello stato alto o basso, scopriamolo:

      pinMode(buttonPin, INPUT);
      pushButtonState = (digitalRead(buttonPin)) ?  LOW : HIGH;

Si resetta il transceiver:

     // Put S2-LP in Shutdown
      pinMode(D7, OUTPUT);
      digitalWrite(D7, HIGH);
  
Si inizializza l'interfaccia di comunicazione SPI con il transceiver:
      
      // Initialize SPI
      devSPI = new SPIClass(D11, D12, D3);
      devSPI->begin();

 Si crea in modo dinamico in memoria di lavoro l'oggetto S2LP e si inizializza:
  
      myS2LP = new S2LP(devSPI, A1, D7, A5, s_frequency, s_RfXtalFrequency, paInfo);
      myS2LP->begin();

Comunico alla libreria la funzione da richiamare quando si riceve un pacchetto dati

    myS2LP->attachS2LPReceive(callback_func);

Inizializzo l'interfaccia di comunicazione I2C e vedo se è montato il sensore TMP102:

     Wire.begin(); // start the I2C library
     bool tmp102_presente = my_tmp102.begin();

Se non è presente lo comunico all'utente:
  
     if (tmp102_presente) 
     Serial.println("Sensore tmp102 presente... ");
     else
      {
       Serial.println("tmp102 non rilevato. Controlla il collegamento del sensore di temperatura D15 -> SCL e D14 -> SDA.");
       //while(1); dovrei fermare tutto in attesa che l'utente sistemi. Se voglio che funzioni sempre non lo faccio......
      }
     

## Fase di Loop

Si trasmette solo se premo il pulsante blu:

    if(digitalRead(buttonPin) == pushButtonState)
    {
      /* Debouncing */
      delay(50);
      /* Wait until the button is released */
      while (digitalRead(buttonPin) == pushButtonState);
      /* Debouncing */
      delay(50);
    
      //Se è presente il sensore TMP102 lo uso altrimenti mi arrrangio
    
      if (tmp102_presente) T = my_tmp102.getTemp();
      else 
      T = random(10,30);
   
      Serial.println(T);
    
      String temp = String(T).c_str(); //Converto la temperatura in una sequenza di caratteri ASCII
    
      int i =  9;  //carico i dati nel buffer di invio lasciando intatto il preambolo di lunghezza 10 caratteri
    
       for (uint8_t c : temp) {
        send_buf[i] =  c;
        i++;
        }

    Se l'invio ha successo la funzione send mi restituisce 0: 

    if(!myS2LP->send(send_buf, (strlen((char *)send_buf) + 1), 0x44, true))
    {
      /* Blink LED */
      blink_led();

      /* Print message */
      Serial.print("Transmitted ");
      Serial.print((strlen((char *)send_buf) + 1));
      Serial.println(" bytes successfully");
    } 
    else
      {
       Serial.println("Error in transmission");
      }
    } // Si chiude finalmente la parte ....se il pulsante è premuto trasmetto.....
    
Se nel frattempo è arrivato un pacchetto: 

    if(receive_packet)
    {
      receive_packet = 0;
      recv_data();
      
    /* Blink LED */
      blink_led();
      }
    
    
## Funzioni di servizio

La funzione di elaborazione dei dati ricevuti:

    void recv_data(void)
    { 
    uint8_t data_size = myS2LP->getRecvPayloadLen();

    myS2LP->read(read_buf, data_size);

    Serial.print("Received packet (size of ");
    Serial.print(data_size);
    Serial.print(" bytes): ");
    Serial.println((char *)read_buf);
    }

Se arriva un pacchetto di dati setto il flag:
    
    void callback_func(void)
    {
      receive_packet = 1;
    }

Se ricevo dati il led lampeggia:

      void blink_led(void)
     {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(10);
      digitalWrite(LED_BUILTIN, LOW);
      delay(10);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(10);
      digitalWrite(LED_BUILTIN, LOW);
    }


