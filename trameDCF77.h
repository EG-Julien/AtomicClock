#ifndef trameDCF77_h
#define trameDCF77_h

#include <stdint.h>

const uint8_t NB_BITS_TRAME_DCF77 = 59;
const uint8_t NB_OCTETS_TRAME_DCF77 = (NB_BITS_TRAME_DCF77 + 7) / 8;

class trameDCF77_c
{
  public :
    trameDCF77_c();

    void raz();
    void ajouterBit(uint8_t bit);
    uint8_t lireBit(uint8_t rang);

    const uint8_t longueur();
    const bool estVide();
    const bool estComplete();

    const bool decoder(uint8_t *annee, uint8_t *mois, uint8_t *jour, uint8_t *joursem,
                       uint8_t *heure, uint8_t *minute, bool *heure_ete);
    //retourne false si trame invalide, avec date/heure inchangées
    //retourne true  si trame valide, avec date/heure chargées et certifiées cohérentes.
    //nb : joursem = 1 Lundi .... 7 Dimanche

  private : //Données : 9 octets
    uint8_t _longueur;
    uint8_t _valeur[NB_OCTETS_TRAME_DCF77];

    void localiserBit(uint8_t rang, uint8_t *num_octet, uint8_t *num_bit);
    void ecrireBit(uint8_t bit, uint8_t rang);
    uint8_t calculerParite(uint8_t debut, uint8_t fin);
    uint8_t extraireValeurDCB(uint8_t debut, uint8_t fin);
};

extern trameDCF77_c trameDCF77;

#endif
