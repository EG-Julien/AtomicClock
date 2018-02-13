#include "decodeurDCF1.h"
#include "decodeurDCF77.h"

void ISR_DCF1() //Routine d'interruption
{
  static bool dispo = true;

  if (dispo)
  {
    dispo = false;
    decodeurDCF77.traiterSignal(digitalRead(decodeurDCF1.pin()), millis());
    dispo = true;
  }
}

void decodeurDCF1_c::begin(uint8_t pin)
{
  _pin = pin & 127;
  pinMode(_pin, INPUT);
}

const uint8_t decodeurDCF1_c::pin()
{
  return _pin & 127;
}

void decodeurDCF1_c::start()
{
  int interruption = digitalPinToInterrupt(_pin & 127);

  if (interruption != -1)
  {
    attachInterrupt(interruption, ISR_DCF1, CHANGE);
    _pin |= 128;
  }
}

void decodeurDCF1_c::stop()
{
  if (_pin > 127)
  {
    detachInterrupt(digitalPinToInterrupt(_pin & 127));
    _pin &= 127;
  }
}

const bool decodeurDCF1_c::started()
{
  return _pin > 127;
}

const uint32_t decodeurDCF1_c::millis_derniere_trame_ok() {return decodeurDCF77.millis_derniere_trame_ok();}
const uint8_t  decodeurDCF1_c::annee()                    {return decodeurDCF77.annee();                   }
const uint8_t  decodeurDCF1_c::mois()                     {return decodeurDCF77.mois();                    }
const uint8_t  decodeurDCF1_c::jour()                     {return decodeurDCF77.jour();                    }
const uint8_t  decodeurDCF1_c::joursem()                  {return decodeurDCF77.joursem();                 }
const uint8_t  decodeurDCF1_c::heure()                    {return decodeurDCF77.heure();                   }
const uint8_t  decodeurDCF1_c::minute()                   {return decodeurDCF77.minute();                  }
const bool     decodeurDCF1_c::heure_ete()                {return decodeurDCF77.heure_ete();               }
const uint8_t  decodeurDCF1_c::longueur_trame_en_cours()  {return decodeurDCF77.longueur_trame_en_cours(); }
const uint8_t  decodeurDCF1_c::bit_trame(uint8_t rang)    {return decodeurDCF77.bit_trame(rang);           }
const uint8_t  decodeurDCF1_c::qualiteReception()         {return decodeurDCF77.qualiteReception(millis());}

decodeurDCF1_c decodeurDCF1;
