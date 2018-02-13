#ifndef tunerDCF77_h
#define tunerDCF77_h

#include <stdint.h>

const uint8_t PERIODE_MESURE_DCF77 = 5; //secondes glissantes

const uint32_t DUREE_MINIMUM_NIVEAU_BAS_DCF77 = 700; //millisecondes

class tunerDCF77_c
{
  public :
    tunerDCF77_c();

    void enregistrerFrontMontant(uint32_t millis_front);
    void enregistrerFrontDescendant(uint32_t millis_front);

    const uint8_t qualiteReception(uint32_t millis_maintenant);
  //valeurs renvoyées :
    //  0    : signal inexistant
    //1 à 4  : signal trop faible, pas de décodage possible
  //  5    : signal parfait, décodage probable
  //  6    : signal correct, décodage probable
  //7 à 11 : signal parasité, décodage incertain selon algo

  private : //Données : 25 octets
    uint8_t  qualite;
    uint32_t ref_chrono;
    uint32_t ref_blanc;
    uint32_t ref_trou;
    uint8_t  nb_pics[PERIODE_MESURE_DCF77 + 1];
    uint8_t  nb_blancs[PERIODE_MESURE_DCF77 + 1];

    void     glisser(uint32_t jusqua);
    void     glisser(uint8_t combien);
    void     comblerTrou();
    uint8_t  calculerQualite();
};

extern tunerDCF77_c tunerDCF77;

#endif
