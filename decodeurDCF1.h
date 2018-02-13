#ifndef decodeurDCF1_h
#define decodeurDCF1_h

#include <Arduino.h>

class decodeurDCF1_c
{
  public :
    void begin(uint8_t pin); //obligatoirement une pin avec interruption

    void start(); //activer le traitement de l'interruption (non inclus dans begin)
    void stop();  //desactiver le traitement de l'interruption

    //A partir de l�, les donnees ci-dessous sont automatiquement mises � jour par l'interruption
    //Utilisation en lecture seule

    //HORLOGE
    const uint32_t millis_derniere_trame_ok(); //Suivre la valeur de cette zone
          //pour savoir si une nouvelle date/heure est disponible ci-dessous
    const uint8_t  annee();
    const uint8_t  mois();
    const uint8_t  jour();
    const uint8_t  joursem(); //1 = lundi, ... 7 = dimanche
    const uint8_t  heure();
    const uint8_t  minute();
    const bool     heure_ete();

    //SIGNAL
    const uint8_t  qualiteReception();
          //valeurs renvoy�es :
          //  0    : signal inexistant
          //1 � 4  : signal trop faible, pas de d�codage possible
          //  5    : signal parfait, d�codage probable
          //  6    : signal correct, d�codage probable
          //7 � 11 : signal parasit�, d�codage incertain
    const uint8_t  longueur_trame_en_cours(); //pour suivre la r�ception
    const uint8_t  bit_trame(uint8_t rang);   //pour suivre la r�ception

    //DIVERS
    const bool started(); //interruption activee
    const uint8_t pin();

  private :
    uint8_t _pin;
};

extern decodeurDCF1_c decodeurDCF1;

#endif
