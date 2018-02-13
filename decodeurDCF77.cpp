#include "decodeurDCF77.h"
#include "tunerDCF77.h"
#include "trameDCF77.h"

/***************************************************************************************************/

decodeurDCF77_c::decodeurDCF77_c()
{
  _millis_derniere_trame_ok = 0;
  _annee = 0;
  _mois = 0;
  _jour = 0;
  _joursem = 0;
  _heure = 0;
  _minute = 0;
  _heure_ete = false;
  ref_synchro_pulsation = 0;
}

const uint32_t decodeurDCF77_c::millis_derniere_trame_ok() {return _millis_derniere_trame_ok;}
const uint8_t  decodeurDCF77_c::annee()                    {return _annee;                   }
const uint8_t  decodeurDCF77_c::mois()                     {return _mois;                    }
const uint8_t  decodeurDCF77_c::jour()                     {return _jour;                    }
const uint8_t  decodeurDCF77_c::joursem()                  {return _joursem;                 }
const uint8_t  decodeurDCF77_c::heure()                    {return _heure;                   }
const uint8_t  decodeurDCF77_c::minute()                   {return _minute;                  }
const bool     decodeurDCF77_c::heure_ete()                {return _heure_ete;               }
const uint8_t  decodeurDCF77_c::longueur_trame_en_cours()  {return trameDCF77.longueur();    }
const uint8_t  decodeurDCF77_c::bit_trame(uint8_t rang)    {return trameDCF77.lireBit(rang); }

const uint8_t  decodeurDCF77_c::qualiteReception(uint32_t millis_maintenant)
{
  return tunerDCF77.qualiteReception(millis_maintenant);
}

/***************************************************************************************************/

bool decodeurDCF77_c::traiterSignal(uint8_t signal, uint32_t millis_signal)
{
  static uint32_t millis_signal_precedent = 0;
  static uint32_t millis_front_montant_pulse = 0;
  static uint32_t millis_niveau_bas = 0;
  static uint8_t  signal_precedent = 0;

  signal &= 1;

  //Ici on ne laisse passer qu'un �chantillon maximum par milliseconde
  if (millis_signal == millis_signal_precedent)
  {
    return false;
  }
  millis_signal_precedent = millis_signal;

//Ici on ne laisse passer que les fronts montants / descendants
  if (signal == (signal_precedent & 1))
  {
    return false;
  }
  signal_precedent ^= 1;

//Ici on alimente le tuner DCF77 pour le suivi de la qualit� du signal
  if (signal)
  {
    tunerDCF77.enregistrerFrontMontant(millis_signal);
  }
  else
  {
    tunerDCF77.enregistrerFrontDescendant(millis_signal);
  }

//Ici on recherche la pulsation des fronts montants � 1 Hz
  const uint8_t coef_filtre_fronts_montants = 30;

  if (signal && (millis_signal - millis_front_montant_pulse > 700))
  {
    millis_front_montant_pulse = millis_signal;

    decalerPulsation(millis_signal);

    uint16_t ecart = ref_synchro_pulsation - millis_signal;

    if (ecart > 0)
    {
      if (ecart < 500)
      {
        ecart *= (coef_filtre_fronts_montants - 1);
        ecart /= coef_filtre_fronts_montants;
        ref_synchro_pulsation = millis_signal + ecart;
      }
      else
      {
        ecart = 1000 - ecart;
        ecart *= (coef_filtre_fronts_montants - 1);
        ecart /= coef_filtre_fronts_montants;
        ref_synchro_pulsation = millis_signal + 1000 - ecart;
      }
    }
  }

//Ici on �limine les fronts montants trop loin de la pulsation
  const uint16_t maxi_avant = 50;
  const uint16_t maxi_apres = 200;

  if (signal)
  {
    decalerPulsation(millis_signal);
    uint16_t ecart = ref_synchro_pulsation - millis_signal;
    if (ecart > maxi_avant && ecart < (1000 - maxi_apres))
    {
      return false;
    }
  }

//Ici on ne laisse passer que les alternances
  if (signal == (signal_precedent >> 1))
  {
    return false;
  }
  signal_precedent ^= 2;

//Ici on d�code les bits de la trame
  bool pas_fini = true;

  if (!signal)
  {
    millis_niveau_bas = millis_signal;
  }
  else
  {
    uint32_t delai1 = millis_signal - millis_niveau_bas;
    if (delai1 > 500)
    {
      decalerPulsation(millis_niveau_bas);
      uint32_t delai2 = ref_synchro_pulsation - millis_niveau_bas;
      if (delai2 > 850)
      {
        trameDCF77.ajouterBit(0);
      }
      else
      {
        trameDCF77.ajouterBit(1);
      }
      if (delai1 > 1500)
      {
        pas_fini = false;
      }
    }
  }
  if (pas_fini)
  {
    return false;
  }

//Ici on termine le travail
  bool resultat_final = trameDCF77.decoder(&_annee, &_mois, &_jour, &_joursem, &_heure, &_minute, &_heure_ete);

  if (resultat_final)
  {
    _millis_derniere_trame_ok = millis_signal;
  }
  trameDCF77.raz();

  return resultat_final;
}

void decodeurDCF77_c::decalerPulsation(uint32_t millis_reference)
{
//D�place la reference de pulsation par bonds de 1000 ms
//pour la placer dans une fourchette de temps comprise entre +0 ms et +999ms
//par rapport � millis_reference

  if (ref_synchro_pulsation != millis_reference)
  {
    uint32_t delai = millis_reference - ref_synchro_pulsation;
    if (delai < 0x80000000) // pulsation avant millis_reference
    {
      if (delai > 30000)
      {
        delai /= 1000;
        delai -= 2;
        delai *= 1000;
        ref_synchro_pulsation += delai;
        delai = millis_reference - ref_synchro_pulsation;
      }
      ref_synchro_pulsation += 1000;
      delai = millis_reference - ref_synchro_pulsation;
      while ((delai < 0x80000000) && (delai > 0))
      {
        ref_synchro_pulsation += 1000;
        delai = millis_reference - ref_synchro_pulsation;
      }
    }
    else   // pulsation apres millis_reference
    {
      delai = ref_synchro_pulsation - millis_reference;
      if (delai > 999)
      {
        if (delai > 30000)
        {
          delai /= 1000;
          delai -= 2;
          delai *= 1000;
          ref_synchro_pulsation -= delai;
          delai = ref_synchro_pulsation - millis_reference;
        }
        ref_synchro_pulsation -= 1000;
        delai = ref_synchro_pulsation - millis_reference;
        while (delai > 999)
        {
          ref_synchro_pulsation -= 1000;
          delai = ref_synchro_pulsation - millis_reference;
        }
      }
    }
  }
}

/***************************************************************************************************/

decodeurDCF77_c decodeurDCF77;
