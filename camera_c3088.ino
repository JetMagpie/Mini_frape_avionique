// I2C
#define SYSCLK 16

// PINES DEL 0 AL 7
#define PORTY PORTD
#define PINY PIND
#define DDRY DDRD

// PINES DE CONTROL
#define PCLK 10
#define HREF 11
#define VSYN 12

// REGISTROS DE LA CÁMARA
#define camAddRead 193
#define camAddWrite 192
#define camRegClock 0x11
#define camRegControlA 0x12
#define camDatReset 0x80
#define camDatbarras 0x26
#define camDatEspejo 0x40
#define camDatEspejoNo 0xBF // repasar
#define camDatFrecLo 0x10
#define camDatFrecMed 0x01 //medio AC
#define camDatFrecMed2 0x3F //medio AC
#define camDatFrecUp 0x00

#define camRegControlC 0x14
#define camDatQCIF 0x20


#define TWI_SLA_CAM   0xc0  //Cam C0    sensor 90
#define MAX_ITER        200
#define PAGE_SIZE       8 // before 8

#ifndef UCSRB
# ifdef UCSR1A          /* ATmega128 */
#  define UCSRA UCSR1A
#  define UCSRB UCSR1B
#  define UBRR UBRR1L
#  define UDR UDR1
# else /* ATmega8 */
#  define UCSRA USR
#  define UCSRB UCR
# endif
#endif
#ifndef UBRR
#  define UBRR UBRRL
#endif


// VARIABLES ************************************************************************

byte pixels[8];

// FUNCIONES ************************************************************************



void cameraIni(){
  //DDRY = 0x00;
  //DDRD = (DDRD & 0xE3);
  
  pinMode(PCLK, INPUT);
  pinMode(HREF, INPUT);
  pinMode(VSYN, INPUT);
  for(int i = 2;i<=9;i++) pinMode(i, INPUT);
  
  
  // resetear camara
  //write_register(camRegControlA, camDatReset);
  delay(1000);
  // min frec: 69,25KHz
  write_register(camRegClock, camDatFrecLo);
  // QCIF
  write_register(camRegControlC, camDatQCIF);
  // se muestran barras de color
  //write_register(camRegControlA, 0x26);
  
  
}

void i2c_init(int clk)
{

  /* initialize TWI clock: 100 kHz clock, TWPS = 0 => prescaler = 1 */
#if defined(TWPS0)
  /* has prescaler (mega128 & newer) */
  TWSR = 0;
#endif
  TWBR = (clk*1000000 / 100000UL - 16) / 2;
 
}

int write_register(uint16_t numregister, uint8_t value){
  uint8_t *pvalue;
  int num;
  pvalue = &value;
  
  num = i2c_write_bytes(numregister, 1, pvalue);
  
  if(num!=1) return -1;
  else return 1;
}

int i2c_write_bytes(uint16_t eeaddr, int len, uint8_t *buf){
  int rv, total;

  total = 0;
  do
    {
#if DEBUG
      printf("Calling i2c_write_page(%d, %d, %p)",
             eeaddr, len, buf);
#endif
      rv = i2c_write_page(eeaddr, len, buf);
#if DEBUG
      printf(" => %d\n", rv);
#endif
      if (rv == -1)
        return -1;
      eeaddr += rv;
      len -= rv;
      buf += rv;
      total += rv;
    }
  while (len > 0);

  return total;
}

int write_register(uint16_t numregister, uint8_t value){
  uint8_t *pvalue;
  int num;
  pvalue = &value;
  
  num = i2c_write_bytes(numregister, 1, pvalue);
  
  if(num!=1) return -1;
  else return 1;
}

int i2c_write_page(uint16_t eeaddr, int len, uint8_t *buf)
{
  uint8_t sla, n = 0;
  int rv = 0;
  uint16_t endaddr;

  if (eeaddr + len < (eeaddr | (PAGE_SIZE - 1)))
    endaddr = eeaddr + len;
  else
    endaddr = (eeaddr | (PAGE_SIZE - 1)) + 1;
  len = endaddr - eeaddr;

  /* patch high bits of EEPROM address into SLA */
  sla = TWI_SLA_CAM | (((eeaddr >> 8) & 0x07) << 1);

  restart:
  if (n++ >= MAX_ITER)
    return -1;
  begin:

  /* Note 13 */
  TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN); /* send start condition */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((TW_STATUS))
    {
    case TW_REP_START:          /* OK, but should not happen */
    case TW_START:
      break;

    case TW_MT_ARB_LOST:
      goto begin;

    default:
      return -1;                /* error: not in start condition */
                                /* NB: do /not/ send stop condition */
    }

  /* send SLA+W */
  TWDR = sla | TW_WRITE;
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((TW_STATUS))
    {
    case TW_MT_SLA_ACK:
      break;

    case TW_MT_SLA_NACK:        /* nack during select: device busy writing */
      goto restart;

    case TW_MT_ARB_LOST:        /* re-arbitrate */
      goto begin;

    default:
      goto error;               /* must send stop condition */
    }
  
  TWDR = eeaddr;                /* low 8 bits of addr */
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((TW_STATUS))
    {
    case TW_MT_DATA_ACK:
      break;

    case TW_MT_DATA_NACK:
      goto quit;

    case TW_MT_ARB_LOST:
      goto begin;

    default:
      goto error;               /* must send stop condition */
    }

  for (; len > 0; len--)
    {
      TWDR = *buf++;
      TWCR = _BV(TWINT) | _BV(TWEN); /* start transmission */
      while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
      switch ((TW_STATUS))
        {
        case TW_MT_DATA_NACK:
          goto error;           /* device write protected -- Note [14] */

        case TW_MT_DATA_ACK:
          rv++;
          break;

        default:
          goto error;
        }
    }
  quit:
  TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN); /* send stop condition */

  return rv;

  error:
  rv = -1;
  goto quit;
}

void photo(void){
  // QCIF 176x144
  // CIF  352x292

        // NEW FRAME
          while(digitalRead(VSYN));
        while(!digitalRead(VSYN));
          for(int y = 0;y<40;y++){ //lineas que quiero leer

            // NEW LINE
            
            while(!digitalRead(HREF));
        
              for(int r = 0;r<176;r++){ //longitud de una linea
          
                 // PIXEL OK
                 while(digitalRead(PCLK));
               while(!digitalRead(PCLK));
          
                 // READ 8 PINS AND MAKE A BYTE
                 byte sdd=0;
                 for(int i = 9;i>=2;i--){
                   sdd |= digitalRead(i); //LEO PATILLA
                   if (i>2) sdd <<= 1; //LA DESPLAZO UN BIT A MENOS QUE SEA LA ULTIMA
                 }
                 
                 // SEND DE DATA BY SERIAL PORT
                 Serial.print(sdd, DEC);
                 Serial.print("P");
            }
            while(digitalRead(HREF));
          }
  Serial.println();
}


// SETUP ************************************************************************

void setup(){
  Serial.begin(115200);
  
  i2c_init(SYSCLK);
  cameraIni();
}


// LOOP ************************************************************************

void loop(){
  photo();
  delay(1000);
}