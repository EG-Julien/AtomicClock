#include "trameDCF77.h"

trameDCF77_c::trameDCF77_c()
{
  this->_longueur = 0;
}

void trameDCF77_c::raz()
{
  this->_longueur = 0;
}

void trameDCF77_c::ajouterBit(uint8_t bit)
{
  if (this->_longueur < NB_BITS_TRAME_DCF77)
  {
    this->ecrireBit(bit, this->_longueur);
    this->_longueur++;
  }
}

uint8_t trameDCF77_c::lireBit(uint8_t rang)
{
  uint8_t num_octet, num_bit, octet, masque, bit;

  this->localiserBit(rang, &num_octet, &num_bit);

  octet  = this->_valeur[num_octet];
  masque = 1 << num_bit;

  bit = (octet & masque) ? 1 : 0;

  return bit;
}

const uint8_t trameDCF77_c::longueur()
{
  return this->_longueur;
}

const bool trameDCF77_c::estVide()
{
  return (this->_longueur == 0);
}

const bool trameDCF77_c::estComplete()
{
  return (this->_longueur == NB_BITS_TRAME_DCF77);
}

const bool trameDCF77_c::decoder(uint8_t *annee, uint8_t *mois, uint8_t *jour, uint8_t *joursem,
                                 uint8_t *heure, uint8_t *minute, bool *heure_ete)
{
  //Contr�le de longueur
  if (!this->estComplete())
  {
    return false;
  }

  //Contr�les techniques
  if ((this->lireBit(0)  != 0)
   || (this->lireBit(18) == this->lireBit(17))
   || (this->lireBit(20) != 1)
   || (this->lireBit(28) != this->calculerParite(21, 27))
   || (this->lireBit(35) != this->calculerParite(29, 34))
   || (this->lireBit(58) != this->calculerParite(36, 57)))
  {
    return false;
  }

  //Extraction des valeurs
  uint8_t a  = this->extraireValeurDCB(50, 57);
  uint8_t m  = this->extraireValeurDCB(45, 49);
  uint8_t j  = this->extraireValeurDCB(36, 41);
  uint8_t js = this->extraireValeurDCB(42, 44);
  uint8_t hh = this->extraireValeurDCB(29, 34);
  uint8_t mm = this->extraireValeurDCB(21, 27);
  bool   ete = (this->lireBit(17) == 1);

  //Contr�les plages de valeur
  if (a > 99 || m < 1 || m > 12 || j < 1 || j > 31 || js < 1 || js > 7 || hh > 23 || mm > 59)
  {
    return false;
  }

  //Contr�le validit� de la date
  if ((j == 31 && (m == 2 || m == 4 || m == 6 || m == 9 || m == 11))
   || (j == 30 && (m == 2))
   || (j == 29 && m == 2 && ((a & 3) != 0)))
  {
    return false;
  }

  //Contr�le validit� du flag heures d'�t� pour chaque mois hors mars et octobre
  if ((ete && (m < 3 || m > 10))
   || ((!ete) && m > 3 && m < 10))
  {
    return false;
  }

  //Contr�le validit� du flag heures d'�t� en mars
  if (m == 3)
  {
    uint8_t dernier_dimanche = 31 - ((5 + a + (a >> 2)) % 7);
    if ((j < dernier_dimanche && ete)
     || (j > dernier_dimanche && !ete))
    {
      return false;
    }
    if (j == dernier_dimanche)
    {
      if ((ete && hh < 3)
       || (!ete && hh > 1))
      {
        return false;
      }
    }
  }

  //Contr�le validit� du flag heures d'�t� en octobre
  if (m == 10)
  {
    uint8_t dernier_dimanche = 31 - ((2 + a + (a >> 2)) % 7);
    if ((j < dernier_dimanche && !ete)
     || (j > dernier_dimanche && ete))
    {
      return false;
    }
    if (j == dernier_dimanche)
    {
      if ((ete && hh > 2)
       || (!ete && hh < 2))
      {
        return false;
      }
    }
  }

  //Contr�le validit� du jour de la semaine
  uint16_t jours = ((uint16_t)a)*365 + ((a+3)>>2);
  switch (m)
  {
    case  2 : jours +=  31; break;
    case  3 : jours +=  59; break;
    case  4 : jours +=  90; break;
    case  5 : jours += 120; break;
    case  6 : jours += 151; break;
    case  7 : jours += 181; break;
    case  8 : jours += 212; break;
    case  9 : jours += 243; break;
    case 10 : jours += 273; break;
    case 11 : jours += 304; break;
    case 12 : jours += 334; break;
  }
  if ((m < 3) || (a&3))
  {
    jours += j - 1;
  }
  else // le 29/02 est pass�
  {
    jours += j;
  }
  if ((js % 7) != ((jours + 6) % 7))
  {
    return false;
  }

  //Tout est OK !
  *annee = a;
  *mois = m;
  *jour = j;
  *joursem = js;
  *heure = hh;
  *minute = mm;
  *heure_ete = ete;

  return true;
}

void trameDCF77_c::localiserBit(uint8_t rang, uint8_t *num_octet, uint8_t *num_bit)
{
  if (rang >= NB_BITS_TRAME_DCF77)
  {
    rang = NB_BITS_TRAME_DCF77 - 1;
  }

  *num_octet = rang >> 3;
  *num_bit   = rang &  7;
}

void trameDCF77_c::ecrireBit(uint8_t bit, uint8_t rang)
{
  uint8_t num_octet, num_bit, masque;

  this->localiserBit(rang, &num_octet, &num_bit);

  masque = 1 << num_bit;

  if (bit)
  {
    _valeur[num_octet] |= masque;
  }
  else
  {
    _valeur[num_octet] &= ~masque;
  }
}

uint8_t trameDCF77_c::calculerParite(uint8_t debut, uint8_t fin)
{
  uint8_t parite = 0;

  for (uint8_t i = debut; i <= fin; i++)
  {
    parite ^= this->lireBit(i);
  }

  return parite;
}

uint8_t trameDCF77_c::extraireValeurDCB(uint8_t debut, uint8_t fin)
{
  uint8_t resultat = 0;

  uint8_t poids = 1;
  for (uint8_t i = debut; i <= fin; i++)
  {
    if (this->lireBit(i))
    {
      resultat += poids;
    }
    if (poids == 8)
    {
      poids = 10;
    }
    else
    {
      poids <<= 1;
    }
  }

  return resultat;
}

trameDCF77_c trameDCF77;
