#include "tunerDCF77.h"

const uint8_t TAILLE_BUFFER_MESURE_DCF77 = PERIODE_MESURE_DCF77 + 1;

tunerDCF77_c::tunerDCF77_c()
{
  this->qualite = 0;
  this->ref_chrono = 0;
  this->ref_blanc = 0;
  this->ref_trou = 0;
  this->ref_trou -= 60000UL;

  for (uint8_t i=0; i<TAILLE_BUFFER_MESURE_DCF77; i++)
  {
    this->nb_blancs[i] = 0;
    this->nb_pics[i] = 0;
  }
}

void tunerDCF77_c::enregistrerFrontDescendant(uint32_t millis_front)
{
  this->glisser(millis_front);
  this->ref_blanc = millis_front;
}

void tunerDCF77_c::enregistrerFrontMontant(uint32_t millis_front)
{
  this->glisser(millis_front);

  if (this->nb_pics[PERIODE_MESURE_DCF77] < 255)
  {
    this->nb_pics[PERIODE_MESURE_DCF77]++;
  }

  uint32_t duree_blanc = millis_front - this->ref_blanc;

  if (duree_blanc >= DUREE_MINIMUM_NIVEAU_BAS_DCF77)
  {
    this->nb_blancs[PERIODE_MESURE_DCF77]++;
  }
}

const uint8_t tunerDCF77_c::qualiteReception(uint32_t millis_maintenant)
{
  if (millis_maintenant - this->ref_chrono > 5000)
  {
    return 0;
  }
  return this->qualite;
}

void tunerDCF77_c::glisser(uint32_t jusqua)
{
  uint32_t delai;
  uint8_t combien;

  delai = jusqua - this->ref_chrono;

  if (delai >= 1000) //Un glissement des compteurs est nécessaire
  {
    delai /= 1000;

    if (delai < TAILLE_BUFFER_MESURE_DCF77)
    {
      combien = delai;
      delai *= 1000;
      this->ref_chrono += delai;
    }
    else
    {
      combien = TAILLE_BUFFER_MESURE_DCF77;
      this->ref_chrono = jusqua;
      this->ref_trou = jusqua - 60000UL;
    }

    this->glisser(combien);
  }
}

void tunerDCF77_c::glisser(uint8_t combien)
{
  uint8_t i = 0;
  uint8_t j = combien;

  while (j < TAILLE_BUFFER_MESURE_DCF77)
  {
    this->nb_blancs[i] = this->nb_blancs[j];
    this->nb_pics[i++] = this->nb_pics[j++];
  }

  while (i < TAILLE_BUFFER_MESURE_DCF77)
  {
    this->nb_blancs[i] = 0;
    this->nb_pics[i++] = 0;
  }

  this->comblerTrou();
  this->qualite = this->calculerQualite();
}

void tunerDCF77_c::comblerTrou()
{
//simule un front montant une fois par minute
//pour ne pas fausser la stat à cause de la seconde 59 du signal DCF77

  uint8_t i, i_trou, nb_trous;
  uint32_t delai;

  nb_trous = 0;
  for (i=0; i<PERIODE_MESURE_DCF77 ;i++)
  {
    if (this->nb_pics[i] == 0 && this->nb_blancs[i] == 0)
    {
      nb_trous++;
      i_trou = i;
    }
  }

  if (nb_trous == 1 && i_trou > 0)
  {
    delai = this->ref_chrono - this->ref_trou;
    if (delai >= 60000UL)
    {
      this->nb_pics[i_trou] = 1;
      this->nb_blancs[i_trou] = 1;
      this->ref_trou += 60000UL;
    }
  }
}

uint8_t tunerDCF77_c::calculerQualite()
{
  const uint8_t NIVEAU_DCF77_MAX = (PERIODE_MESURE_DCF77 * 2) + 1;
  uint8_t i, nb;
  uint16_t nbre;

  nb = 0;
  for (i=0; i<PERIODE_MESURE_DCF77; i++)
  {
    if (this->nb_pics[i])
  {
    nb++;
  }
  }
  if (nb <PERIODE_MESURE_DCF77) //signal insuffisant : pas assez d'impulsions sur la période
  {
    return nb;
  }

  nb = 0;
  for (i=0; i<PERIODE_MESURE_DCF77; i++)
  {
    if (this->nb_blancs[i])
  {
    nb++;
  }
  }
  if (nb <PERIODE_MESURE_DCF77) //signal avec des parasites sur les niveaux bas
  {
    return NIVEAU_DCF77_MAX - nb;
  }

  nbre = 0;
  for (i=0; i<PERIODE_MESURE_DCF77; i++)
  {
    nbre += this->nb_pics[i];
  }
  if (nbre > PERIODE_MESURE_DCF77) //signal avec niveaux bas ok mais quelques parasites sur les niveaux haut
  {
    return PERIODE_MESURE_DCF77 + 1;
  }

  return PERIODE_MESURE_DCF77; //signal a priori nickel
}

tunerDCF77_c tunerDCF77;
